/*
 * Copyright (C) 2024-2026 Dominik Drexler
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef LOKI_SEMANTIC_TRANSLATOR_CONSTRAINT_TRANSLATOR_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_CONSTRAINT_TRANSLATOR_HPP_

#include "copy_translator_component.hpp"
#include "loki/formalism/constraints.hpp"

#include <algorithm>
#include <concepts>
#include <optional>
#include <type_traits>
#include <variant>
#include <yggdrasil/diagnostics/errors.hpp>

namespace loki::semantic::detail
{

/// Formulas over the state trajectory (PDDL 3 constraints and :trajectory-formulas). A formula is evaluated over the state
/// trajectory, so each goal description in it stays one formula: removing universal quantifiers makes each conjunct of one
/// that is no literal or numeric constraint a generated derived predicate loki-constraint-*, and nothing splits or lifts it.
/// The negation normal form pushes negation through the duals of the operators. Compiling typing guards a quantified
/// formula by the type conditions of its parameters.
template<typename Derived>
class ConstraintTranslator : public CopyTranslatorComponent<Derived, ConstraintTranslator<Derived>>
{
public:
    explicit ConstraintTranslator(CopyContext& context) : CopyTranslatorComponent<Derived, ConstraintTranslator<Derived>>(context) {}

    formalism::ConstraintView copy(formalism::ConstraintView source);
    formalism::ConstraintView copy(formalism::EntityView<formalism::Constraint<formalism::StateTag>> source);
    formalism::ConstraintView copy(formalism::EntityView<formalism::Constraint<formalism::NotTag>> source);
    formalism::ConstraintView copy(formalism::EntityView<formalism::Constraint<formalism::ForallTag>> source);
    formalism::ConstraintView copy(formalism::EntityView<formalism::Constraint<formalism::ExistsTag>> source);

    /// The negation of a formula in negation normal form, in negation normal form: next and weak-next, until and release,
    /// yesterday and weak-yesterday, since and triggers, once and historically, sometime and always are duals, at end
    /// negates its operand; the other PDDL 3 operators have no dual among the operators and stay under a not.
    formalism::ConstraintView negate_constraint(formalism::ConstraintView constraint);

    /// Whether the formula is in PDDL 3's form: conjunctions, universals and preferences over modal constraints of PDDL 3.
    [[nodiscard]] bool is_pddl3_formula(formalism::ConstraintView constraint) const;

    /// A PDDL 3 formula holding vacuously for the bindings of the parameters outside their types: the condition that
    /// triggers a modal constraint (at-most-once, sometime-after, sometime-before, always-within) gets the type conditions as
    /// conjuncts, the condition that one demands (at end, always, sometime, within, hold-during, hold-after) the alternative
    /// of their negation (CompileTypingTranslator::guard_with_types).
    formalism::ConstraintView guard_constraint(formalism::ConstraintView constraint, formalism::EntityListView<formalism::Parameter> parameters);

private:
    template<typename Tag>
    formalism::ConstraintView insert(urd::builder<formalism::Constraint<Tag>, formalism::Repository>& data)
    { return formalism::ConstraintView(this->copy_context().storage->repository.insert(data, this->copy_context().storage->builder).first); }

    formalism::ConstraintView state(formalism::ConditionView condition)
    {
        auto data = formalism::checkout<formalism::Constraint<formalism::StateTag>>(this->copy_context().builder);
        urd::store(data->condition, condition);
        return insert(*data);
    }

    /// Whether typing goes away for typed parameters below the root of a hierarchy: guarding needs them.
    [[nodiscard]] bool guards(formalism::EntityListView<formalism::Parameter> parameters) const
    {
        return this->self().compiles_typing_now()
               && std::ranges::any_of(parameters,
                                      [](auto parameter) { return std::ranges::any_of(parameter.types(), [](auto type) { return !type.bases().empty(); }); });
    }

    template<typename Tag>
    formalism::ConstraintView copy_quantified(formalism::EntityView<formalism::Constraint<Tag>> source);
};

template<typename Derived>
formalism::ConstraintView ConstraintTranslator<Derived>::copy(formalism::ConstraintView source)
{
    // A formula's variables keep their names: each quantifier is a scope of its own, and renaming them by a count of
    // quantifications would rename them again on every retranslation.
    // ponytail: a goal description that requantifies an enclosing quantifier's variable is not renamed apart.
    const auto renaming_disabled = RenamingDisabled(this->copy_context());
    return std::visit([&](const auto& node) -> formalism::ConstraintView { return this->self().copy(node); }, source);
}

template<typename Derived>
formalism::ConstraintView ConstraintTranslator<Derived>::copy(formalism::EntityView<formalism::Constraint<formalism::StateTag>> source)
{
    auto condition = this->self().copy(source.condition());
    if (this->copy_context().phase == TranslationPhase::RemoveUniversalQuantifiers)
        condition = this->self().make_conjunctive_condition(condition, "loki-constraint-", this->copy_context().generated_constraint_conditions);
    return state(condition);
}

template<typename Derived>
formalism::ConstraintView ConstraintTranslator<Derived>::copy(formalism::EntityView<formalism::Constraint<formalism::NotTag>> source)
{
    const auto operand = this->self().copy(source.constraint());
    if (this->copy_context().phase == TranslationPhase::ToNegationNormalForm)
        return this->self().negate_constraint(operand);
    auto data = formalism::checkout<formalism::Constraint<formalism::NotTag>>(this->copy_context().builder);
    urd::store(data->constraint, operand);
    return insert(*data);
}

template<typename Derived>
formalism::ConstraintView ConstraintTranslator<Derived>::copy(formalism::EntityView<formalism::Constraint<formalism::ForallTag>> source)
{ return copy_quantified(source); }

template<typename Derived>
formalism::ConstraintView ConstraintTranslator<Derived>::copy(formalism::EntityView<formalism::Constraint<formalism::ExistsTag>> source)
{ return copy_quantified(source); }

template<typename Derived>
template<typename Tag>
formalism::ConstraintView ConstraintTranslator<Derived>::copy_quantified(formalism::EntityView<formalism::Constraint<Tag>> source)
{
    this->self().increment_quantifications(source.parameters());
    auto parameter_views = this->self().copy_parameter_views(source.parameters());
    this->self().enter_scope(parameter_views);
    auto constraint = this->self().copy(source.constraint());
    if (guards(source.parameters()))
    {
        if constexpr (std::same_as<Tag, formalism::ExistsTag>)
        {
            // For some binding within the types: (and types φ).
            auto types = formalism::checkout<formalism::Condition<formalism::AndTag>>(this->copy_context().builder);
            this->self().type_conditions_for_parameters(source.parameters(), *types);
            auto data = formalism::checkout<formalism::Constraint<formalism::AndTag>>(this->copy_context().builder);
            urd::append(data->constraints, state(this->self().make_conjunction(*types)));
            urd::append(data->constraints, constraint);
            constraint = insert(*data);
        }
        else if (is_pddl3_formula(constraint))
        {
            constraint = this->self().guard_constraint(constraint, source.parameters());
        }
        else
        {
            // For every binding outside the types vacuously: (or outside φ), outside a derived predicate loki-typed-*.
            auto data = formalism::checkout<formalism::Constraint<formalism::OrTag>>(this->copy_context().builder);
            urd::append(data->constraints, state(this->self().guard_with_types(std::nullopt, source.parameters())));
            urd::append(data->constraints, constraint);
            constraint = insert(*data);
        }
    }
    auto data = formalism::checkout<formalism::Constraint<Tag>>(this->copy_context().builder);
    if (this->self().compiles_typing_now())
        this->self().copy_parameters_without_types(source.parameters(), data->parameters);
    else
        for (auto parameter : parameter_views)
            data->parameters.push_back(parameter.index());
    urd::store(data->constraint, constraint);
    const auto out = insert(*data);
    this->self().leave_scope();
    return out;
}

template<typename Derived>
formalism::ConstraintView ConstraintTranslator<Derived>::negate_constraint(formalism::ConstraintView constraint)
{
    auto& context = this->copy_context();
    return std::visit(
        [&]<typename Node>(const Node& node) -> formalism::ConstraintView
        {
            using formalism::Constraint;
            if constexpr (std::same_as<Node, formalism::EntityView<Constraint<formalism::StateTag>>>)
                return state(this->self().negate_condition(node.condition()));
            else if constexpr (std::same_as<Node, formalism::EntityView<Constraint<formalism::NotTag>>>)
                return node.constraint();
            else if constexpr (std::same_as<Node, formalism::EntityView<Constraint<formalism::AndTag>>>
                               || std::same_as<Node, formalism::EntityView<Constraint<formalism::OrTag>>>)
            {
                using Dual = std::conditional_t<std::same_as<Node, formalism::EntityView<Constraint<formalism::AndTag>>>, formalism::OrTag, formalism::AndTag>;
                auto data = formalism::checkout<Constraint<Dual>>(context.builder);
                for (auto child : node.constraints())
                    urd::append(data->constraints, this->self().negate_constraint(child));
                return insert(*data);
            }
            else if constexpr (std::same_as<Node, formalism::EntityView<Constraint<formalism::ForallTag>>>
                               || std::same_as<Node, formalism::EntityView<Constraint<formalism::ExistsTag>>>)
            {
                using Dual =
                    std::conditional_t<std::same_as<Node, formalism::EntityView<Constraint<formalism::ForallTag>>>, formalism::ExistsTag, formalism::ForallTag>;
                auto data = formalism::checkout<Constraint<Dual>>(context.builder);
                for (auto parameter : node.parameters())
                    data->parameters.push_back(parameter.index());
                urd::store(data->constraint, this->self().negate_constraint(node.constraint()));
                return insert(*data);
            }
            else if constexpr (std::same_as<Node, formalism::EntityView<Constraint<formalism::PreferenceTag>>>)
            {
                throw ygg::diagnostics::SemanticError("a preference occurs under a negation");
            }
            else
            {
                using enum formalism::ConstraintOperator;
                const auto operation = node.operation();
                auto dual = std::optional<formalism::ConstraintOperator> {};
                switch (operation)
                {
                    case Next:
                        dual = WeakNext;
                        break;
                    case WeakNext:
                        dual = Next;
                        break;
                    case Until:
                        dual = Release;
                        break;
                    case Release:
                        dual = Until;
                        break;
                    case Yesterday:
                        dual = WeakYesterday;
                        break;
                    case WeakYesterday:
                        dual = Yesterday;
                        break;
                    case Since:
                        dual = Triggers;
                        break;
                    case Triggers:
                        dual = Since;
                        break;
                    case Once:
                        dual = Historically;
                        break;
                    case Historically:
                        dual = Once;
                        break;
                    case Sometime:
                        dual = Always;
                        break;
                    case Always:
                        dual = Sometime;
                        break;
                    case AtEnd:
                        dual = AtEnd;
                        break;
                    default:
                        break;
                }
                if (!dual)
                {
                    auto data = formalism::checkout<Constraint<formalism::NotTag>>(context.builder);
                    urd::store(data->constraint, formalism::ConstraintView(node));
                    return insert(*data);
                }
                auto data = formalism::checkout<Constraint<formalism::ModalTag>>(context.builder);
                data->operation = *dual;
                for (const auto number : node.numbers())
                    data->numbers.push_back(number);
                for (auto operand : node.constraints())
                    urd::append(data->constraints, this->self().negate_constraint(operand));
                return insert(*data);
            }
        },
        constraint);
}

template<typename Derived>
bool ConstraintTranslator<Derived>::is_pddl3_formula(formalism::ConstraintView constraint) const
{
    return std::visit(
        [&]<typename Node>(const Node& node) -> bool
        {
            using formalism::Constraint;
            if constexpr (std::same_as<Node, formalism::EntityView<Constraint<formalism::AndTag>>>)
                return std::ranges::all_of(node.constraints(), [&](auto child) { return is_pddl3_formula(child); });
            else if constexpr (std::same_as<Node, formalism::EntityView<Constraint<formalism::ForallTag>>>
                               || std::same_as<Node, formalism::EntityView<Constraint<formalism::PreferenceTag>>>)
                return is_pddl3_formula(node.constraint());
            else if constexpr (std::same_as<Node, formalism::EntityView<Constraint<formalism::ModalTag>>>)
                return !formalism::is_trajectory_operator(node.operation())
                       && std::ranges::all_of(node.constraints(),
                                              [](auto operand)
                                              { return std::holds_alternative<formalism::EntityView<Constraint<formalism::StateTag>>>(operand); });
            else
                return false;
        },
        constraint);
}

template<typename Derived>
formalism::ConstraintView ConstraintTranslator<Derived>::guard_constraint(formalism::ConstraintView constraint,
                                                                          formalism::EntityListView<formalism::Parameter> parameters)
{
    auto& context = this->copy_context();
    return std::visit(
        [&]<typename Node>(const Node& node) -> formalism::ConstraintView
        {
            using formalism::Constraint;
            if constexpr (std::same_as<Node, formalism::EntityView<Constraint<formalism::AndTag>>>)
            {
                auto data = formalism::checkout<Constraint<formalism::AndTag>>(context.builder);
                for (auto child : node.constraints())
                    urd::append(data->constraints, this->self().guard_constraint(child, parameters));
                return insert(*data);
            }
            else if constexpr (std::same_as<Node, formalism::EntityView<Constraint<formalism::ForallTag>>>)
            {
                auto data = formalism::checkout<Constraint<formalism::ForallTag>>(context.builder);
                for (auto parameter : node.parameters())
                    data->parameters.push_back(parameter.index());
                urd::store(data->constraint, this->self().guard_constraint(node.constraint(), parameters));
                return insert(*data);
            }
            else if constexpr (std::same_as<Node, formalism::EntityView<Constraint<formalism::PreferenceTag>>>)
            {
                auto data = formalism::checkout<Constraint<formalism::PreferenceTag>>(context.builder);
                data->name = node.name();
                urd::store(data->constraint, this->self().guard_constraint(node.constraint(), parameters));
                return insert(*data);
            }
            else if constexpr (std::same_as<Node, formalism::EntityView<Constraint<formalism::ModalTag>>>)
            {
                using enum formalism::ConstraintOperator;
                const auto operation = node.operation();
                const auto triggered = operation == AtMostOnce || operation == SometimeAfter || operation == SometimeBefore || operation == AlwaysWithin;
                auto data = formalism::checkout<Constraint<formalism::ModalTag>>(context.builder);
                data->operation = operation;
                for (const auto number : node.numbers())
                    data->numbers.push_back(number);
                auto first = true;
                for (auto operand : node.constraints())
                {
                    // is_pddl3_formula: each operand is a goal description.
                    auto condition = std::get<formalism::EntityView<Constraint<formalism::StateTag>>>(operand).condition();
                    if (first && triggered)
                        this->self().prepend_type_conditions(condition, parameters);
                    else if (first)
                        condition = this->self().guard_with_types(condition, parameters);
                    first = false;
                    urd::append(data->constraints, state(condition));
                }
                return insert(*data);
            }
            else
                return node;
        },
        constraint);
}

}  // namespace loki::semantic::detail

#endif
