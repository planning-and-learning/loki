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
#include "builder.hpp"
#include "checks.hpp"
#include "context.hpp"
#include "diagnostics.hpp"
#include "loki/ast/ast.hpp"
#include "loki/formalism/conditions.hpp"
#include "loki/formalism/effects.hpp"
#include "loki/formalism/repository.hpp"
#include "loki/semantic/errors.hpp"
#include "mappings.hpp"

#include <cmath>
#include <urd/elements/children.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
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
    return repo().insert(*data, m_builder).first;
}

formalism::EffectView AstBuilder::parse_effect_node(const ast::EffectAnd& node)
{
    auto data = formalism::checkout<formalism::Effect<formalism::AndTag>>(m_builder);
    for (const auto& child : node.effects)
        urd::append(data->effects, parse_effect(child.get()));
    return repo().insert(*data, m_builder).first;
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
        urd::append(function_data->terms, parse_term(term));
    const auto function_term = repo().insert(*function_data, m_builder).first;
    const auto expression = parse_function_expression(node.expression.get());
    auto data = formalism::checkout<formalism::Effect<formalism::NumericTag>>(m_builder);
    data->operation = op;
    data->function = function_term.index();
    urd::store(data->expression, expression);
    return repo().insert(*data, m_builder).first;
}

formalism::EffectView AstBuilder::parse_effect_node(const ast::EffectForall& node)
{
    checks().require_requirement(formalism::RequirementKind::UniversalPreconditions, node);
    auto scope = VariableScope(m_parse_context);
    auto parameters = parse_parameters(node.parameters);
    auto child = parse_effect(node.effect.get());
    auto data = formalism::checkout<formalism::Effect<formalism::ForallTag>>(m_builder);
    urd::store(data->parameters, parameters);
    urd::store(data->effect, child);
    return repo().insert(*data, m_builder).first;
}

formalism::EffectView AstBuilder::parse_effect_node(const ast::EffectWhen& node)
{
    checks().require_requirement(formalism::RequirementKind::ConditionalEffects, node);
    const auto condition = parse_condition(node.condition.get());
    const auto effect = parse_effect(node.effect.get());
    auto data = formalism::checkout<formalism::Effect<formalism::WhenTag>>(m_builder);
    urd::store(data->condition, condition);
    urd::store(data->effect, effect);
    return repo().insert(*data, m_builder).first;
}

formalism::EffectView AstBuilder::parse_effect_node(const ast::EffectOneOf& node)
{
    checks().require_requirement(formalism::RequirementKind::NonDeterministic, node);
    auto data = formalism::checkout<formalism::Effect<formalism::OneOfTag>>(m_builder);
    for (const auto& child : node.effects)
        urd::append(data->effects, parse_effect(child.get()));
    return repo().insert(*data, m_builder).first;
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
        urd::clear_fields(*alternative_data);
        alternative_data->probability = alternative.probability;
        urd::store(alternative_data->effect, effect);
        data->alternatives.push_back(repo().insert(*alternative_data, m_builder).first.index());
    }
    constexpr auto rounding_tolerance = 1e-9;
    if (total > 1.0 + rounding_tolerance)
        m_diagnostics.throw_at(node, InvalidProbabilisticEffectError("probabilities sum to more than 1"));
    return repo().insert(*data, m_builder).first;
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
    data->name = urd::flat_string(name);
    data->original_name = data->name;
    urd::store(data->parameters, parameters);
    data->original_arity = data->parameters.size();
    if (precondition)
        urd::store(data->precondition, *precondition);
    if (effect)
        urd::store(data->effect, *effect);
    return repo().insert(*data, m_builder).first;
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
            urd::append(atom_data->terms, parse_term(term));
        }
    auto atom = repo().insert(*atom_data, m_builder).first;
    auto literal_data = formalism::checkout<formalism::Literal>(m_builder);
    literal_data->atom = atom.index();
    literal_data->polarity = true;
    auto head = repo().insert(*literal_data, m_builder).first;
    auto condition = parse_condition(node.condition);
    auto data = formalism::checkout<formalism::Axiom>(m_builder);
    urd::store(data->parameters, parameters);
    data->original_arity = data->parameters.size();
    data->head = head.index();
    urd::store(data->condition, condition);
    return repo().insert(*data, m_builder).first;
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
    urd::store(data->value, expression);
    values.push_back(repo().insert(*data, m_builder).first);
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
    urd::store(data->expression, expression);
    return repo().insert(*data, m_builder).first;
}

formalism::FunctionSkeletonView AstBuilder::total_cost_function()
{
    if (auto it = m_domain_context.functions.find("total-cost"); it != m_domain_context.functions.end())
        return it->second;
    auto data = formalism::checkout<formalism::FunctionSkeleton>(m_builder);
    data->name = urd::flat_string("total-cost");
    data->type = m_domain_context.number_type.index();
    auto view = repo().insert(*data, m_builder).first;
    m_domain_context.functions.emplace("total-cost", view);
    return view;
}

formalism::FunctionTermView AstBuilder::total_cost_term()
{
    const auto function = total_cost_function();
    auto data = formalism::checkout<formalism::FunctionTerm>(m_builder);
    data->function = function.index();
    return repo().insert(*data, m_builder).first;
}

namespace
{
bool writes_total_cost(formalism::EntityView<formalism::Effect<formalism::NumericTag>> effect)
{ return effect.get_function().get_function().get_name() == "total-cost"; }

/// Whether a numeric effect on total-cost occurs in node.
template<typename T>
bool writes_total_cost(formalism::EntityView<T> node)
{
    auto found = false;
    urd::for_each_child<formalism::SymbolRepositoryTypes, T>(node, [&](auto child) { found = found || writes_total_cost(child); });
    return found;
}
}  // namespace

formalism::ActionView AstBuilder::add_unit_cost(formalism::ActionView view)
{
    if (writes_total_cost(view))
        return view;

    auto number_data = formalism::checkout<formalism::FunctionExpression<formalism::NumberTag>>(m_builder);
    number_data->value = 1.0;
    const auto one = repo().insert(*number_data, m_builder).first;
    const auto total_cost = total_cost_term();
    auto numeric_data = formalism::checkout<formalism::Effect<formalism::NumericTag>>(m_builder);
    numeric_data->operation = formalism::NumericEffectOperator::Increase;
    numeric_data->function = total_cost.index();
    numeric_data->expression = one.index();
    const auto increase = repo().insert(*numeric_data, m_builder).first;
    auto combined = formalism::EffectView(increase);
    if (const auto effect = view.get_effect())
    {
        auto and_data = formalism::checkout<formalism::Effect<formalism::AndTag>>(m_builder);
        urd::append(and_data->effects, effect.value());
        and_data->effects.push_back(increase.index());
        combined = repo().insert(*and_data, m_builder).first;
    }

    auto data = formalism::checkout<formalism::Action>(m_builder);
    data->name = view.get_name();
    data->original_name = data->name;
    data->parameters.reserve(view.get_parameters().size());
    for (auto parameter : view.get_parameters())
        data->parameters.push_back(parameter.index());
    data->original_arity = data->parameters.size();
    urd::store(data->precondition, view.get_precondition());
    urd::store(data->effect, combined);
    return repo().insert(*data, m_builder).first;
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
            m_diagnostics.throw_at(task, ygg::diagnostics::SemanticError("Missing total-cost metric for :action-costs"));
        if (missing_initial_value)
            m_diagnostics.throw_at(task, ygg::diagnostics::SemanticError("Missing initial value for total-cost for :action-costs"));
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
        metric = repo().insert(*data, m_builder).first;
    }
    if (missing_initial_value)
    {
        auto number_data = formalism::checkout<formalism::FunctionExpression<formalism::NumberTag>>(m_builder);
        number_data->value = 0.0;
        const auto zero = repo().insert(*number_data, m_builder).first.index();
        const auto function = total_cost_term();
        auto data = formalism::checkout<formalism::InitialFunctionValue>(m_builder);
        data->function = function.index();
        data->value = zero;
        initial_function_values.push_back(repo().insert(*data, m_builder).first);
    }
}

}  // namespace loki::semantic
