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

#include "loki/semantic/translator/infer_requirements.hpp"

#include "mappings.hpp"

#include <set>
#include <string_view>
#include <type_traits>
#include <yggdrasil/containers/unordered_set.hpp>

namespace loki::semantic::detail
{
namespace
{

using enum formalism::RequirementKind;

/// Mirrors the requirement checks of the strict parser (AstBuilder and SemanticChecks): each construct adds what parsing it requires
/// or marks used, so that the result declares every requirement the reparse demands and none that it would reject as unused.
struct RequirementCollector
{
    std::set<formalism::RequirementKind> requirements;

    template<typename Node, typename Tag>
    static constexpr bool is = std::is_same_v<std::decay_t<Node>, formalism::EntityView<Tag>>;

    static bool is_total_cost(formalism::FunctionSkeletonView function) { return std::string_view(function.get_name()) == "total-cost"; }

    void parameters(formalism::EntityListView<formalism::Parameter> parameters)
    {
        for (const auto parameter : parameters)
            if (!parameter.get_types().empty())
                requirements.insert(Typing);
    }

    void objects(formalism::EntityListView<formalism::Object> objects)
    {
        for (const auto object : objects)
            if (!object.get_types().empty())
                requirements.insert(Typing);
    }

    void types(formalism::EntityListView<formalism::Type> types)
    {
        for (const auto type : types)
        {
            const auto name = std::string_view(type.get_name());
            if (name != "object" && name != "number")
                requirements.insert(Typing);
        }
    }

    void function(formalism::FunctionSkeletonView function)
    {
        requirements.insert(is_total_cost(function) ? ActionCosts : NumericFluents);
        parameters(function.get_parameters());
    }

    void atom(formalism::AtomView atom)
    {
        if (std::string_view(atom.get_predicate().get_name()) == "=")
            requirements.insert(Equality);
    }

    void function_term(formalism::FunctionTermView term) { requirements.insert(is_total_cost(term.get_function()) ? ActionCosts : NumericFluents); }

    void expression(formalism::FunctionExpressionView expression)
    {
        ygg::visit(
            [&](const auto& node)
            {
                using Node = decltype(node);
                if constexpr (std::is_same_v<std::decay_t<Node>, formalism::FunctionTermView>)
                    function_term(node);
                else if constexpr (is<Node, formalism::FunctionExpression<formalism::UnaryTag>>)
                    this->expression(node.get_expression());
                else if constexpr (is<Node, formalism::FunctionExpression<formalism::BinaryTag>>)
                {
                    this->expression(node.get_left());
                    this->expression(node.get_right());
                }
                else if constexpr (is<Node, formalism::FunctionExpression<formalism::MultiTag>>)
                    for (const auto arg : node.get_args())
                        this->expression(arg);
            },
            expression.get_variant());
    }

    void condition(formalism::ConditionView condition)
    {
        ygg::visit(
            [&](const auto& node)
            {
                using Node = decltype(node);
                if constexpr (is<Node, formalism::Condition<formalism::LiteralTag>>)
                {
                    atom(node.get_literal().get_atom());
                    if (!node.get_literal().get_polarity())
                        requirements.insert(NegativePreconditions);
                }
                else if constexpr (is<Node, formalism::Condition<formalism::AndTag>>)
                    for (const auto child : node.get_conditions())
                        this->condition(child);
                else if constexpr (is<Node, formalism::Condition<formalism::OrTag>>)
                {
                    requirements.insert(DisjunctivePreconditions);
                    for (const auto child : node.get_conditions())
                        this->condition(child);
                }
                else if constexpr (is<Node, formalism::Condition<formalism::NotTag>>)
                {
                    requirements.insert(NegativePreconditions);
                    this->condition(node.get_condition());
                }
                else if constexpr (is<Node, formalism::Condition<formalism::ImplyTag>>)
                {
                    requirements.insert(DisjunctivePreconditions);
                    this->condition(node.get_left());
                    this->condition(node.get_right());
                }
                else if constexpr (is<Node, formalism::Condition<formalism::ExistsTag>> || is<Node, formalism::Condition<formalism::ForallTag>>)
                {
                    requirements.insert(is<Node, formalism::Condition<formalism::ExistsTag>> ? ExistentialPreconditions : UniversalPreconditions);
                    parameters(node.get_parameters());
                    this->condition(node.get_condition());
                }
                else if constexpr (is<Node, formalism::Condition<formalism::NumericConstraintTag>>)
                {
                    requirements.insert(NumericFluents);
                    expression(node.get_left());
                    expression(node.get_right());
                }
            },
            condition.get_variant());
    }

    void effect(formalism::EffectView effect)
    {
        ygg::visit(
            [&](const auto& node)
            {
                using Node = decltype(node);
                if constexpr (is<Node, formalism::Effect<formalism::LiteralTag>>)
                    atom(node.get_literal().get_atom());
                else if constexpr (is<Node, formalism::Effect<formalism::AndTag>> || is<Node, formalism::Effect<formalism::OneOfTag>>)
                {
                    if constexpr (is<Node, formalism::Effect<formalism::OneOfTag>>)
                        requirements.insert(NonDeterministic);
                    for (const auto child : node.get_effects())
                        this->effect(child);
                }
                else if constexpr (is<Node, formalism::Effect<formalism::NumericTag>>)
                {
                    // Bare :action-costs permits only writes of the form (increase (total-cost) ...).
                    const auto increases_total_cost =
                        node.get_operator() == formalism::NumericEffectOperator::Increase && is_total_cost(node.get_function().get_function());
                    requirements.insert(increases_total_cost ? ActionCosts : NumericFluents);
                    function_term(node.get_function());
                    expression(node.get_expression());
                }
                else if constexpr (is<Node, formalism::Effect<formalism::ForallTag>>)
                {
                    requirements.insert(UniversalPreconditions);
                    parameters(node.get_parameters());
                    this->effect(node.get_effect());
                }
                else if constexpr (is<Node, formalism::Effect<formalism::WhenTag>>)
                {
                    requirements.insert(ConditionalEffects);
                    condition(node.get_condition());
                    this->effect(node.get_effect());
                }
                else if constexpr (is<Node, formalism::Effect<formalism::ProbabilisticTag>>)
                {
                    requirements.insert(ProbabilisticEffects);
                    for (const auto alternative : node.get_alternatives())
                        this->effect(alternative.get_effect());
                }
            },
            effect.get_variant());
    }

    void axioms(formalism::EntityListView<formalism::Axiom> axioms)
    {
        for (const auto axiom : axioms)
        {
            requirements.insert(DerivedPredicates);
            parameters(axiom.get_parameters());
            condition(axiom.get_condition());
            // The printer writes the parameters that are not in the head as an exists around the condition.
            auto head_variables = ygg::UnorderedSet<formalism::VariableView> {};
            for (const auto term : axiom.get_head().get_atom().get_terms())
                ygg::visit(
                    [&](const auto& node)
                    {
                        if constexpr (std::is_same_v<std::decay_t<decltype(node)>, formalism::VariableView>)
                            head_variables.insert(node);
                    },
                    term.get_variant());
            for (const auto parameter : axiom.get_parameters())
                if (!head_variables.contains(parameter.get_variable()))
                    requirements.insert(ExistentialPreconditions);
        }
    }

    void predicates(formalism::EntityListView<formalism::Predicate> predicates)
    {
        for (const auto predicate : predicates)
            parameters(predicate.get_parameters());
    }
};

}  // namespace

std::vector<formalism::RequirementKind> infer_requirements(formalism::DomainView domain)
{
    auto collector = RequirementCollector {};
    // The strict parser demands :strips of every domain.
    collector.requirements.insert(Strips);
    collector.types(domain.get_types());
    collector.objects(domain.get_constants());
    collector.predicates(domain.get_predicates());
    for (const auto function : domain.get_functions())
        collector.function(function);
    for (const auto action : domain.get_actions())
    {
        collector.parameters(action.get_parameters());
        if (const auto precondition = action.get_precondition())
            collector.condition(precondition.value());
        if (const auto effect = action.get_effect())
            collector.effect(effect.value());
    }
    collector.axioms(domain.get_axioms());
    return { collector.requirements.begin(), collector.requirements.end() };
}

std::vector<formalism::RequirementKind> infer_requirements(formalism::TaskView task, formalism::DomainView domain)
{
    auto collector = RequirementCollector {};
    collector.types(task.get_types());
    collector.predicates(task.get_predicates());
    collector.objects(task.get_objects());
    for (const auto literal : task.get_initial_literals())
        collector.atom(literal.get_atom());
    for (const auto value : task.get_initial_function_values())
        collector.function_term(value.get_function());
    if (const auto goal = task.get_goal())
        collector.condition(goal.value());
    if (const auto metric = task.get_metric())
    {
        collector.requirements.insert(NumericFluents);
        collector.expression(metric.value().get_expression());
    }
    collector.axioms(task.get_axioms());

    // What a domain requirement provides needs no declaration in the task, and declaring it again is redundant.
    collector.requirements.erase(Strips);
    for (const auto requirement : domain.get_requirements())
        for (const auto capability : requirement_capabilities(requirement.get_kind()))
            collector.requirements.erase(capability);
    return { collector.requirements.begin(), collector.requirements.end() };
}

}  // namespace loki::semantic::detail
