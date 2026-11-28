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

#ifndef LOKI_SEMANTIC_TRANSLATOR_REMOVE_UNIVERSAL_QUANTIFIERS_TRANSLATOR_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_REMOVE_UNIVERSAL_QUANTIFIERS_TRANSLATOR_HPP_

#include "copy_translator_component.hpp"
#include "loki/formalism/preferences.hpp"

#include <concepts>
#include <string_view>
#include <urd/elements/clone.hpp>
#include <urd/elements/flat.hpp>

namespace loki::semantic::detail
{

template<typename Derived>
class RemoveUniversalQuantifiersTranslator : public CopyTranslatorComponent<Derived, RemoveUniversalQuantifiersTranslator<Derived>>
{
public:
    explicit RemoveUniversalQuantifiersTranslator(CopyContext& context) :
        CopyTranslatorComponent<Derived, RemoveUniversalQuantifiersTranslator<Derived>>(context)
    {
    }

    /// The negated head of a generated axiom loki-universal-*(free parameters) <- condition.
    formalism::ConditionView make_generated_axiom_condition(formalism::ConditionView condition);
    /// The condition of an effect that holds a choice, if a conjunction of literals and numeric constraints; otherwise
    /// the head of a generated axiom loki-choice-*(free parameters) <- condition. Splitting a disjunction or lifting an
    /// existential of the condition into a forall effect would apply the choice once per disjunct or witness, where it
    /// is drawn once (choices.hpp); a derived atom is evaluated once.
    formalism::ConditionView make_choice_condition(formalism::ConditionView condition);
    /// The condition of a timed condition, each conjunct that is no literal or numeric constraint the head of a
    /// generated axiom loki-timed-*(free parameters) <- conjunct: a durative action's condition is never split or
    /// lifted into parameters (TopLevelTranslator).
    formalism::ConditionView make_timed_condition(formalism::ConditionView condition);
    /// The condition with each conjunct that is no literal or numeric constraint replaced by the head of a generated
    /// axiom prefix*(free parameters) <- conjunct.
    formalism::ConditionView make_conjunctive_condition(formalism::ConditionView condition,
                                                        std::string_view prefix,
                                                        urd::Store<urd::map<GeneratedUniversalConditionKey, formalism::ConditionView>>& cache);
    /// The head of a generated axiom prefix*(free parameters) <- condition, as a literal of the given polarity; one axiom
    /// per condition and free parameters in cache.
    formalism::ConditionView make_generated_axiom_literal(formalism::ConditionView condition,
                                                          std::string_view prefix,
                                                          bool polarity,
                                                          urd::Store<urd::map<GeneratedUniversalConditionKey, formalism::ConditionView>>& cache);
    formalism::ConditionView remove_universal_quantifiers(formalism::ConditionView condition);
    formalism::ConditionView remove_universal_quantifiers_node(formalism::EntityView<formalism::Condition<formalism::AndTag>> source);
    formalism::ConditionView remove_universal_quantifiers_node(formalism::EntityView<formalism::Condition<formalism::OrTag>> source);
    /// A not or an imply: its conditions translated in member order (both sides of an imply can generate
    /// loki-universal-* names), the record rebuilt.
    template<typename Tag>
        requires std::same_as<Tag, formalism::NotTag> || std::same_as<Tag, formalism::ImplyTag>
    formalism::ConditionView remove_universal_quantifiers_node(formalism::EntityView<formalism::Condition<Tag>> source);
    formalism::ConditionView remove_universal_quantifiers_node(formalism::EntityView<formalism::Condition<formalism::ExistsTag>> source);
    formalism::ConditionView remove_universal_quantifiers_node(formalism::EntityView<formalism::Condition<formalism::ForallTag>> source);
    formalism::ConditionView remove_universal_quantifiers_node(formalism::EntityView<formalism::Condition<formalism::TimedTag>> source);
    /// A preference counts its violations: its condition becomes a conjunction (make_conjunctive_condition), never a derived
    /// predicate as a whole.
    formalism::ConditionView remove_universal_quantifiers_node(formalism::EntityView<formalism::Condition<formalism::PreferenceTag>> source);
    template<typename T>
    formalism::ConditionView remove_universal_quantifiers_node(formalism::EntityView<T> source);
};

template<typename Derived>
formalism::ConditionView RemoveUniversalQuantifiersTranslator<Derived>::make_generated_axiom_condition(formalism::ConditionView condition)
{ return make_generated_axiom_literal(condition, "loki-universal-", false, this->copy_context().generated_universal_conditions); }

template<typename Derived>
formalism::ConditionView RemoveUniversalQuantifiersTranslator<Derived>::make_choice_condition(formalism::ConditionView condition)
{ return make_conjunctive_condition(condition, "loki-choice-", this->copy_context().generated_choice_conditions); }

template<typename Derived>
formalism::ConditionView RemoveUniversalQuantifiersTranslator<Derived>::make_timed_condition(formalism::ConditionView condition)
{ return make_conjunctive_condition(condition, "loki-timed-", this->copy_context().generated_timed_conditions); }

template<typename Derived>
formalism::ConditionView
RemoveUniversalQuantifiersTranslator<Derived>::make_conjunctive_condition(formalism::ConditionView condition,
                                                                          std::string_view prefix,
                                                                          urd::Store<urd::map<GeneratedUniversalConditionKey, formalism::ConditionView>>& cache)
{
    const auto is_basic = [](formalism::ConditionView node)
    {
        return std::holds_alternative<formalism::EntityView<formalism::Condition<formalism::LiteralTag>>>(node)
               || std::holds_alternative<formalism::EntityView<formalism::Condition<formalism::NumericConstraintTag>>>(node);
    };
    if (is_basic(condition))
        return condition;
    const auto* conjunction = std::get_if<formalism::EntityView<formalism::Condition<formalism::AndTag>>>(&condition);
    if (!conjunction)
        return make_generated_axiom_literal(condition, prefix, true, cache);
    auto data = formalism::checkout<formalism::Condition<formalism::AndTag>>(this->copy_context().builder);
    for (auto conjunct : conjunction->conditions())
        this->self().append_conjunct(*data, is_basic(conjunct) ? conjunct : make_generated_axiom_literal(conjunct, prefix, true, cache));
    return this->self().make_conjunction(*data);
}

template<typename Derived>
formalism::ConditionView RemoveUniversalQuantifiersTranslator<Derived>::make_generated_axiom_literal(
    formalism::ConditionView condition,
    std::string_view prefix,
    bool polarity,
    urd::Store<urd::map<GeneratedUniversalConditionKey, formalism::ConditionView>>& cache)
{
    const auto free_parameters = this->self().free_parameters_in_scope(condition);

    const auto key = GeneratedUniversalConditionKey { free_parameters, condition };
    if (auto it = cache.find(key); it != cache.end())
        return it->second;

    auto predicate_data = formalism::checkout<formalism::Predicate>(this->copy_context().builder);
    auto atom_data = formalism::checkout<formalism::Atom>(this->copy_context().builder);
    for (auto [parameter, variable] : free_parameters)
    {
        predicate_data->parameters.push_back(parameter.index());
        atom_data->terms.push_back(variable.index());
    }

    predicate_data->name = urd::flat_string(this->self().next_generated_predicate_name(prefix));
    const auto predicate = this->copy_context().storage->repository.insert(*predicate_data, this->copy_context().storage->builder).first;
    atom_data->predicate = predicate.index();
    const auto atom = this->copy_context().storage->repository.insert(*atom_data, this->copy_context().storage->builder).first.index();
    auto literal_data = formalism::checkout<formalism::Literal>(this->copy_context().builder);
    literal_data->atom = atom;
    literal_data->polarity = true;
    const auto positive_head = this->copy_context().storage->repository.insert(*literal_data, this->copy_context().storage->builder).first.index();
    urd::clear_fields(*literal_data);
    literal_data->atom = atom;
    literal_data->polarity = polarity;
    const auto result_literal = this->copy_context().storage->repository.insert(*literal_data, this->copy_context().storage->builder).first.index();
    auto axiom_data = formalism::checkout<formalism::Axiom>(this->copy_context().builder);
    for (const auto& parameter : predicate_data->parameters)
        axiom_data->parameters.push_back(urd::clone(parameter));
    axiom_data->original_arity = axiom_data->parameters.size();
    axiom_data->head = positive_head;
    urd::store(axiom_data->condition, condition);
    const auto axiom = this->copy_context().storage->repository.insert(*axiom_data, this->copy_context().storage->builder).first;

    urd::append(this->copy_context().generated_predicates, predicate);
    urd::append(this->copy_context().generated_axioms, axiom);
    auto condition_data = formalism::checkout<formalism::Condition<formalism::LiteralTag>>(this->copy_context().builder);
    condition_data->literal = result_literal;
    auto result = formalism::ConditionView(this->copy_context().storage->repository.insert(*condition_data, this->copy_context().storage->builder).first);
    cache.emplace(key, result);
    return result;
}

template<typename Derived>
formalism::ConditionView RemoveUniversalQuantifiersTranslator<Derived>::remove_universal_quantifiers(formalism::ConditionView condition)
{
    return std::visit([&](const auto& node) { return this->self().remove_universal_quantifiers_node(node); }, condition);
}

template<typename Derived>
formalism::ConditionView
RemoveUniversalQuantifiersTranslator<Derived>::remove_universal_quantifiers_node(formalism::EntityView<formalism::Condition<formalism::AndTag>> source)
{
    auto data = formalism::checkout<formalism::Condition<formalism::AndTag>>(this->copy_context().builder);
    for (auto condition : source.conditions())
        this->self().append_conjunct(*data, this->self().remove_universal_quantifiers(condition));
    return this->self().make_conjunction(*data);
}

template<typename Derived>
formalism::ConditionView
RemoveUniversalQuantifiersTranslator<Derived>::remove_universal_quantifiers_node(formalism::EntityView<formalism::Condition<formalism::OrTag>> source)
{
    auto data = formalism::checkout<formalism::Condition<formalism::OrTag>>(this->copy_context().builder);
    for (auto condition : source.conditions())
        this->self().append_disjunct(*data, this->self().remove_universal_quantifiers(condition));
    return this->self().make_disjunction(*data);
}

template<typename Derived>
template<typename Tag>
    requires std::same_as<Tag, formalism::NotTag> || std::same_as<Tag, formalism::ImplyTag>
formalism::ConditionView
RemoveUniversalQuantifiersTranslator<Derived>::remove_universal_quantifiers_node(formalism::EntityView<formalism::Condition<Tag>> source)
{
    auto scratch = this->copy_context().builder.template scratch<formalism::Condition<Tag>>();
    auto& data = urd::map_variant_children<formalism::SymbolRepositoryTypes, formalism::Condition<Tag>>(
        source,
        *scratch,
        [&](formalism::ConditionView condition) { return this->self().remove_universal_quantifiers(condition); });
    return formalism::ConditionView(this->copy_context().storage->repository.insert(data).first);
}

template<typename Derived>
formalism::ConditionView
RemoveUniversalQuantifiersTranslator<Derived>::remove_universal_quantifiers_node(formalism::EntityView<formalism::Condition<formalism::ExistsTag>> source)
{
    this->self().increment_quantifications(source.parameters());
    auto parameter_views = this->self().copy_parameter_views(source.parameters());
    this->self().enter_scope(parameter_views);
    auto condition = this->self().remove_universal_quantifiers(source.condition());
    this->self().leave_scope();
    auto data = formalism::checkout<formalism::Condition<formalism::ExistsTag>>(this->copy_context().builder);
    for (auto parameter : parameter_views)
        data->parameters.push_back(parameter.index());
    urd::store(data->condition, condition);
    return this->self().flatten_condition(
        formalism::ConditionView(this->copy_context().storage->repository.insert(*data, this->copy_context().storage->builder).first));
}

template<typename Derived>
formalism::ConditionView
RemoveUniversalQuantifiersTranslator<Derived>::remove_universal_quantifiers_node(formalism::EntityView<formalism::Condition<formalism::PreferenceTag>> source)
{
    const auto condition = this->self().make_conjunctive_condition(this->self().remove_universal_quantifiers(source.condition()),
                                                                   "loki-preference-",
                                                                   this->copy_context().generated_preference_conditions);
    auto data = formalism::checkout<formalism::Condition<formalism::PreferenceTag>>(this->copy_context().builder);
    data->name = source.name();
    urd::store(data->condition, condition);
    return formalism::ConditionView(this->copy_context().storage->repository.insert(*data, this->copy_context().storage->builder).first);
}

template<typename Derived>
formalism::ConditionView
RemoveUniversalQuantifiersTranslator<Derived>::remove_universal_quantifiers_node(formalism::EntityView<formalism::Condition<formalism::ForallTag>> source)
{
    // A universal preference is a preference per binding: it stays universal (the parser separates it from hard conditions).
    if (formalism::contains_preference(source.condition()))
    {
        this->self().increment_quantifications(source.parameters());
        auto parameter_views = this->self().copy_parameter_views(source.parameters());
        this->self().enter_scope(parameter_views);
        const auto condition = this->self().remove_universal_quantifiers(source.condition());
        this->self().leave_scope();
        auto data = formalism::checkout<formalism::Condition<formalism::ForallTag>>(this->copy_context().builder);
        for (auto parameter : parameter_views)
            data->parameters.push_back(parameter.index());
        urd::store(data->condition, condition);
        return formalism::ConditionView(this->copy_context().storage->repository.insert(*data, this->copy_context().storage->builder).first);
    }

    this->self().increment_quantifications(source.parameters());
    auto parameter_views = this->self().copy_parameter_views(source.parameters());
    this->self().enter_scope(parameter_views);
    auto negated = this->self().negate_condition(source.condition());
    this->self().leave_scope();
    auto data = formalism::checkout<formalism::Condition<formalism::ExistsTag>>(this->copy_context().builder);
    for (auto parameter : parameter_views)
        data->parameters.push_back(parameter.index());
    urd::store(data->condition, negated);
    const auto exists_not = this->self().flatten_condition(
        formalism::ConditionView(this->copy_context().storage->repository.insert(*data, this->copy_context().storage->builder).first));
    const auto translated_exists_not = this->self().remove_universal_quantifiers(exists_not);
    return this->self().make_generated_axiom_condition(translated_exists_not);
}

template<typename Derived>
formalism::ConditionView
RemoveUniversalQuantifiersTranslator<Derived>::remove_universal_quantifiers_node(formalism::EntityView<formalism::Condition<formalism::TimedTag>> source)
{
    const auto condition = this->self().make_timed_condition(this->self().remove_universal_quantifiers(source.condition()));
    auto data = formalism::checkout<formalism::Condition<formalism::TimedTag>>(this->copy_context().builder);
    data->time = source.time();
    urd::store(data->condition, condition);
    return formalism::ConditionView(this->copy_context().storage->repository.insert(*data, this->copy_context().storage->builder).first);
}

template<typename Derived>
template<typename T>
formalism::ConditionView RemoveUniversalQuantifiersTranslator<Derived>::remove_universal_quantifiers_node(formalism::EntityView<T> source)
{ return formalism::ConditionView(this->self().copy(source)); }

}  // namespace loki::semantic::detail

#endif
