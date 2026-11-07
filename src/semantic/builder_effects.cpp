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

#include "ast_mappings.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include "builder.hpp"
#include "checks.hpp"
#include "context.hpp"
#include "diagnostics.hpp"
#include "loki/ast/ast.hpp"
#include "loki/formalism/repository.hpp"
#include "loki/semantic/errors.hpp"
#include "mappings.hpp"

#include <cmath>
#include <type_traits>
#include <utility>

namespace loki::semantic
{

formalism::EffectView AstBuilder::parse_effect(const ast::Effect& effect)
{
    return boost::apply_visitor([&](const auto& node) { return parse_effect_node(node); }, effect);
}

formalism::EffectView AstBuilder::parse_effect_node(const ast::EffectLiteral& node)
{
    const auto literal = parse_literal(node.literal);
    auto data = formalism::checkout<formalism::Effect<formalism::LiteralTag>>(m_builder);
    data->literal = literal.index();
    return formalism::insert(repo(), *data).first;
}

formalism::EffectView AstBuilder::parse_effect_node(const ast::EffectAnd& node)
{
    auto data = formalism::checkout<formalism::Effect<formalism::AndTag>>(m_builder);
    for (const auto& child : node.effects)
        data->effects.push_back(urd::to_stored(parse_effect(child.get())));
    return formalism::insert(repo(), *data).first;
}

formalism::EffectView AstBuilder::parse_effect_node(const ast::EffectNumeric& node)
{
    const auto op = numeric_effect_operator(node, m_diagnostics);
    // Bare :action-costs permits only writes of the form (increase (total-cost) ...).
    const auto is_total_cost_increase = op == formalism::NumericEffectOperator::Increase && key(node.function.function.text) == "total-cost";
    if (m_parse_context.active_action_costs && is_total_cost_increase)
        checks().mark_requirement_used(formalism::RequirementKind::ActionCosts);
    else
        checks().require_requirement(formalism::RequirementKind::NumericFluents, node);
    if (m_options.strict && !m_parse_context.active_numeric_fluents && !(m_parse_context.active_action_costs && is_total_cost_increase))
        m_diagnostics.throw_at(node, MissingRequirementError("numeric-fluents"));
    auto skeleton = function(node.function.function, node.function.terms.size());
    if (m_domain_context.declared_functions.contains(key(node.function.function.text)))
        checks().check_argument_types(key(node.function.function.text), skeleton.get_parameters(), node.function.terms, node.function.function);
    auto function_data = formalism::checkout<formalism::FunctionTerm>(m_builder);
    function_data->function = skeleton.index();
    for (const auto& term : node.function.terms)
        function_data->terms.push_back(urd::to_stored(parse_term(term)));
    const auto function_term = formalism::insert(repo(), *function_data).first;
    const auto expression = parse_function_expression(node.expression.get());
    auto data = formalism::checkout<formalism::Effect<formalism::NumericTag>>(m_builder);
    data->op = op;
    data->function = function_term.index();
    data->expression = urd::to_stored(expression);
    return formalism::insert(repo(), *data).first;
}

formalism::EffectView AstBuilder::parse_effect_node(const ast::EffectForall& node)
{
    checks().require_requirement(formalism::RequirementKind::UniversalPreconditions, node);
    auto scope = VariableScope(m_parse_context);
    auto parameters = parse_parameters(node.parameters);
    auto child = parse_effect(node.effect.get());
    auto data = formalism::checkout<formalism::Effect<formalism::ForallTag>>(m_builder);
    append_indices(parameters, data->parameters);
    data->effect = urd::to_stored(child);
    return formalism::insert(repo(), *data).first;
}

formalism::EffectView AstBuilder::parse_effect_node(const ast::EffectWhen& node)
{
    checks().require_requirement(formalism::RequirementKind::ConditionalEffects, node);
    const auto condition = parse_condition(node.condition.get());
    const auto effect = parse_effect(node.effect.get());
    auto data = formalism::checkout<formalism::Effect<formalism::WhenTag>>(m_builder);
    data->condition = urd::to_stored(condition);
    data->effect = urd::to_stored(effect);
    return formalism::insert(repo(), *data).first;
}

formalism::EffectView AstBuilder::parse_effect_node(const ast::EffectOneOf& node)
{
    checks().require_requirement(formalism::RequirementKind::NonDeterministic, node);
    auto data = formalism::checkout<formalism::Effect<formalism::OneOfTag>>(m_builder);
    for (const auto& child : node.effects)
        data->effects.push_back(urd::to_stored(parse_effect(child.get())));
    return formalism::insert(repo(), *data).first;
}

formalism::EffectView AstBuilder::parse_effect_node(const ast::EffectProbabilistic& node)
{
    checks().require_requirement(formalism::RequirementKind::ProbabilisticEffects, node);
    auto total = 0.0;
    auto data = formalism::checkout<formalism::Effect<formalism::ProbabilisticTag>>(m_builder);
    auto alternative_data = formalism::checkout<formalism::EffectProbabilisticAlternative>(m_builder);
    for (const auto& alternative : node.alternatives)
    {
        if (!std::isfinite(alternative.probability) || alternative.probability < 0.0 || alternative.probability > 1.0)
            m_diagnostics.throw_at(alternative, InvalidProbabilisticEffectError("probability must be in [0, 1]"));
        total += alternative.probability;
        const auto effect = parse_effect(alternative.effect.get());
        urd::clear_members(*alternative_data);
        alternative_data->probability = alternative.probability;
        alternative_data->effect = urd::to_stored(effect);
        data->alternatives.push_back(formalism::insert(repo(), *alternative_data).first.index());
    }
    constexpr auto rounding_tolerance = 1e-9;
    if (total > 1.0 + rounding_tolerance)
        m_diagnostics.throw_at(node, InvalidProbabilisticEffectError("probabilities sum to more than 1"));
    return formalism::insert(repo(), *data).first;
}

formalism::ActionView AstBuilder::parse_action(const ast::Action& node)
{
    checks().mark_requirement_used(formalism::RequirementKind::Strips);
    const auto name = key(node.name.text);
    auto scope = VariableScope(m_parse_context);
    auto parameters = parse_parameters(node.parameters);
    auto precondition = std::optional<formalism::ConditionView> {};
    if (node.precondition)
        precondition = parse_condition(*node.precondition);
    auto effect = std::optional<formalism::EffectView> {};
    if (node.effect)
        effect = parse_effect(*node.effect);
    auto data = formalism::checkout<formalism::Action>(m_builder);
    data->name = to_cista(name);
    data->original_name = data->name;
    append_indices(parameters, data->parameters);
    data->original_arity = data->parameters.size();
    if (precondition)
        data->precondition = urd::to_stored(*precondition);
    if (effect)
        data->effect = urd::to_stored(*effect);
    return formalism::insert(repo(), *data).first;
}

formalism::AxiomView AstBuilder::parse_axiom(const ast::Axiom& node)
{
    auto scope = VariableScope(m_parse_context);
    auto parameters = parse_parameters(node.head.parameters);
    auto pred = predicate(node.head.name, parameters.size());
    auto atom_data = formalism::checkout<formalism::Atom>(m_builder);
    atom_data->predicate = pred.index();
    for (const auto& group : node.head.parameters)
        for (const auto& identifier : group.names)
        {
            auto term = ast::Term {};
            term.name = identifier;
            term.variable = true;
            atom_data->terms.push_back(urd::to_stored(parse_term(term)));
        }
    auto atom = formalism::insert(repo(), *atom_data).first;
    auto literal_data = formalism::checkout<formalism::Literal>(m_builder);
    literal_data->atom = atom.index();
    literal_data->m_polarity = true;
    auto head = formalism::insert(repo(), *literal_data).first;
    auto condition = parse_condition(node.condition);
    auto data = formalism::checkout<formalism::Axiom>(m_builder);
    append_indices(parameters, data->parameters);
    data->original_arity = data->parameters.size();
    data->head = head.index();
    data->condition = urd::to_stored(condition);
    return formalism::insert(repo(), *data).first;
}

void AstBuilder::parse_initial_element(const ast::Literal& literal,
                                       urd::Store<urd::sequence<formalism::LiteralView>>& literals,
                                       urd::Store<urd::sequence<formalism::InitialFunctionValueView>>&)
{ literals.push_back(parse_literal(literal)); }

void AstBuilder::parse_initial_element(const ast::InitialFunctionValue& value,
                                       urd::Store<urd::sequence<formalism::LiteralView>>&,
                                       urd::Store<urd::sequence<formalism::InitialFunctionValueView>>& values)
{
    const auto function = parse_function_term(value.function);
    const auto expression = parse_function_expression(value.value);
    auto data = formalism::checkout<formalism::InitialFunctionValue>(m_builder);
    data->function = function.index();
    data->value = urd::to_stored(expression);
    values.push_back(formalism::insert(repo(), *data).first);
}

formalism::MetricView AstBuilder::parse_metric(const ast::Metric& node)
{
    // A metric uses numeric fluents even if it is a constant, such as (:metric minimize 5).
    checks().require_requirement(formalism::RequirementKind::NumericFluents, node);
    const auto optimization = key(node.optimization.text);
    if (optimization != "minimize" && optimization != "maximize")
        m_diagnostics.throw_at(node.optimization, InvalidMetricError(optimization));
    const auto optimization_direction = optimization == "minimize" ? formalism::OptimizationDirection::Minimize : formalism::OptimizationDirection::Maximize;
    const auto expression = parse_function_expression(node.expression);
    auto data = formalism::checkout<formalism::Metric>(m_builder);
    data->optimization_direction = optimization_direction;
    data->expression = urd::to_stored(expression);
    return formalism::insert(repo(), *data).first;
}

formalism::FunctionSkeletonView AstBuilder::total_cost_function()
{
    if (auto it = m_domain_context.functions.find("total-cost"); it != m_domain_context.functions.end())
        return it->second;
    auto data = formalism::checkout<formalism::FunctionSkeleton>(m_builder);
    data->name = cista::offset::string("total-cost");
    data->type = m_domain_context.number_type.index();
    auto view = formalism::insert(repo(), *data).first;
    m_domain_context.functions.emplace("total-cost", view);
    return view;
}

formalism::FunctionTermView AstBuilder::total_cost_term()
{
    const auto function = total_cost_function();
    auto data = formalism::checkout<formalism::FunctionTerm>(m_builder);
    data->function = function.index();
    return formalism::insert(repo(), *data).first;
}

bool AstBuilder::writes_total_cost(formalism::EffectView effect)
{
    return std::visit(
        [&]<typename Node>(const Node& arg) -> bool
        {
            if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::NumericTag>>>)
                return arg.get_function().get_function().get_name() == "total-cost";
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::AndTag>>>
                               || std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::OneOfTag>>>)
            {
                for (auto child : arg.get_effects())
                    if (writes_total_cost(child))
                        return true;
                return false;
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::ForallTag>>>
                               || std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::WhenTag>>>)
                return writes_total_cost(arg.get_effect());
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::ProbabilisticTag>>>)
            {
                for (auto alternative : arg.get_alternatives())
                    if (writes_total_cost(alternative.get_effect()))
                        return true;
                return false;
            }
            else
                return false;
        },
        effect);
}

formalism::ActionView AstBuilder::add_unit_cost(formalism::ActionView view)
{
    if (const auto effect = view.get_effect(); effect && writes_total_cost(effect.value()))
        return view;

    auto number_data = formalism::checkout<formalism::FunctionExpression<formalism::NumberTag>>(m_builder);
    number_data->value = 1.0;
    const auto one = formalism::insert(repo(), *number_data).first;
    const auto total_cost = total_cost_term();
    auto numeric_data = formalism::checkout<formalism::Effect<formalism::NumericTag>>(m_builder);
    numeric_data->op = formalism::NumericEffectOperator::Increase;
    numeric_data->function = total_cost.index();
    numeric_data->expression = one.index();
    const auto increase = formalism::insert(repo(), *numeric_data).first;
    auto combined = formalism::EffectView(increase);
    if (const auto effect = view.get_effect())
    {
        auto and_data = formalism::checkout<formalism::Effect<formalism::AndTag>>(m_builder);
        and_data->effects.push_back(urd::to_stored(effect.value()));
        and_data->effects.push_back(increase.index());
        combined = formalism::insert(repo(), *and_data).first;
    }

    auto data = formalism::checkout<formalism::Action>(m_builder);
    data->name = view.get_name();
    data->original_name = data->name;
    data->parameters.reserve(view.get_parameters().size());
    for (auto parameter : view.get_parameters())
        data->parameters.push_back(parameter.index());
    data->original_arity = data->parameters.size();
    data->precondition = view.data().precondition;
    data->effect = urd::to_stored(combined);
    return formalism::insert(repo(), *data).first;
}

bool AstBuilder::has_total_cost_initial_value(const urd::Store<urd::sequence<formalism::InitialFunctionValueView>>& values)
{
    for (auto value : values)
    {
        const auto function = value.get_function();
        if (function.get_function().get_name() == "total-cost" && function.get_terms().empty())
            return true;
    }
    return false;
}

void AstBuilder::complete_action_costs(const ast::Task& task,
                                       urd::Store<urd::sequence<formalism::InitialFunctionValueView>>& initial_function_values,
                                       std::optional<formalism::MetricView>& metric)
{
    if (!m_parse_context.active_action_costs)
        return;

    const auto missing_metric = !metric;
    const auto missing_initial_value = !has_total_cost_initial_value(initial_function_values);
    if (m_options.strict && !m_options.add_action_costs)
    {
        if (missing_metric)
            m_diagnostics.throw_at(task, SemanticError("Missing total-cost metric for :action-costs"));
        if (missing_initial_value)
            m_diagnostics.throw_at(task, SemanticError("Missing initial value for total-cost for :action-costs"));
        return;
    }

    if (missing_metric || missing_initial_value)
        checks().mark_requirement_used(formalism::RequirementKind::ActionCosts);
    if (missing_metric)
    {
        const auto total_cost = total_cost_term();
        auto data = formalism::checkout<formalism::Metric>(m_builder);
        data->optimization_direction = formalism::OptimizationDirection::Minimize;
        data->expression = total_cost.index();
        metric = formalism::insert(repo(), *data).first;
    }
    if (missing_initial_value)
    {
        auto number_data = formalism::checkout<formalism::FunctionExpression<formalism::NumberTag>>(m_builder);
        number_data->value = 0.0;
        const auto zero = formalism::insert(repo(), *number_data).first.index();
        const auto function = total_cost_term();
        auto data = formalism::checkout<formalism::InitialFunctionValue>(m_builder);
        data->function = function.index();
        data->value = zero;
        initial_function_values.push_back(formalism::insert(repo(), *data).first);
    }
}

}  // namespace loki::semantic
