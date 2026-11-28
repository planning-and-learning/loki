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

#include "builder.hpp"

#include "ast_mappings.hpp"
#include "checks.hpp"
#include "context.hpp"
#include "diagnostics.hpp"
#include "loki/ast/ast.hpp"
#include "loki/formalism/conditions.hpp"
#include "loki/formalism/repository.hpp"
#include "loki/semantic/errors.hpp"
#include "mappings.hpp"

#include <algorithm>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>

namespace loki::semantic
{

AstBuilder::VariableScope::VariableScope(ParseContext& context) : m_context(context) { m_context.variable_scopes.push_scope(); }

AstBuilder::VariableScope::~VariableScope() { m_context.variable_scopes.pop_scope(); }

AstBuilder::AstBuilder(const ParserOptions& options,
                       const DiagnosticContext& diagnostics,
                       formalism::Repository& repository,
                       DomainContext& domain_context,
                       ParseContext& parse_context) :
    m_options(options),
    m_diagnostics(diagnostics),
    m_builder(),
    m_repository(repository),
    m_domain_context(domain_context),
    m_parse_context(parse_context)
{
}

formalism::Repository& AstBuilder::repo() noexcept { return m_repository; }

SemanticChecks AstBuilder::checks() const { return SemanticChecks { &m_options, &m_diagnostics, &m_domain_context, &m_parse_context }; }

formalism::DomainView AstBuilder::build_domain(const ast::Domain& domain)
{
    // Each kind of section concatenated in source order: a domain's sections can come in any order.
    const auto parts = ast::parts(domain);
    auto requirements = parse_requirements(parts.requirements);
    if (m_options.strict && std::none_of(parts.requirements.begin(), parts.requirements.end(), [](const auto& node) { return node.name.text == "strips"; }))
        m_diagnostics.throw_at(domain.name, MissingRequirementError("strips"));
    // Genuine numeric domains keep their own cost structure; a planner interprets an absent
    // metric as unit costs, so grafting total-cost onto :numeric-fluents would be wrong.
    // A PPDDL domain with :rewards has its own objective, (reward), just as a numeric domain has.
    const auto inject_unit_costs =
        m_options.add_action_costs && !m_parse_context.active_action_costs && !m_parse_context.active_numeric_fluents && !m_parse_context.active_rewards;
    if (inject_unit_costs)
    {
        m_parse_context.active_action_costs = true;
        remember_requirement(m_parse_context, formalism::RequirementKind::ActionCosts);
        auto data = formalism::checkout<formalism::Requirement>(m_builder);
        data->kind = formalism::RequirementKind::ActionCosts;
        const auto requirement = repo().insert(*data, m_builder).first;
        if (std::none_of(requirements.begin(), requirements.end(), [&](const auto value) { return value.index() == requirement.index(); }))
            requirements.push_back(requirement);
    }
    m_domain_context.requirement_kinds = m_parse_context.active_requirements;
    m_domain_context.action_costs = m_parse_context.active_action_costs;
    m_domain_context.numeric_fluents = m_parse_context.active_numeric_fluents;
    m_domain_context.rewards = m_parse_context.active_rewards;
    auto types = parse_type_hierarchy(parts.types);
    auto constants = parse_objects(parts.constants, m_domain_context.objects, m_domain_context.declared_objects);
    auto predicates = parse_predicates(parts.predicates);
    auto functions = parse_functions(parts.functions);
    if (m_parse_context.active_action_costs && !m_domain_context.declared_functions.contains("total-cost"))
    {
        if (m_options.strict && !m_options.add_action_costs)
            m_diagnostics.throw_at(domain.name, ygg::diagnostics::SemanticError("Missing total-cost function for :action-costs"));
        m_domain_context.declared_functions.insert("total-cost");
        functions.push_back(total_cost_function());
    }
    // PPDDL declares (reward) by :rewards alone.
    if (m_parse_context.active_rewards && !m_domain_context.declared_functions.contains("reward"))
    {
        m_domain_context.declared_functions.insert("reward");
        functions.push_back(number_function("reward"));
    }
    auto axioms = urd::Store<urd::sequence<formalism::AxiomView>> {};
    for (const auto& declaration : parts.declarations)
        if (const auto* axiom = boost::get<ast::Axiom>(&declaration))
        {
            checks().require_requirement(formalism::RequirementKind::DerivedPredicates, *axiom);
            axioms.push_back(parse_axiom(*axiom));
        }
    auto actions = urd::Store<urd::sequence<formalism::ActionView>> {};
    for (const auto& declaration : parts.declarations)
        if (const auto* action = boost::get<ast::Action>(&declaration))
            actions.push_back(parse_action(*action));
    if (inject_unit_costs)
        for (auto& action : actions)
            action = add_unit_cost(action);
    auto durative_actions = urd::Store<urd::sequence<formalism::DurativeActionView>> {};
    for (const auto& declaration : parts.declarations)
        if (const auto* action = boost::get<ast::DurativeAction>(&declaration))
            durative_actions.push_back(parse_durative_action(*action));
    // After the actions: is-violated reads the names of their preferences.
    const auto constraints = parse_constraints(parts.constraints);
    checks().reject_unused_requirements(parts.requirements);

    types.clear();
    for (const auto& [_, type] : m_domain_context.types)
        types.push_back(type);
    std::sort(types.begin(), types.end(), [](auto lhs, auto rhs) { return lhs.index() < rhs.index(); });

    auto data = formalism::checkout<formalism::Domain>(m_builder);
    data->name = urd::flat_string(domain.name.text);
    urd::store(data->requirements, requirements);
    urd::store(data->types, types);
    urd::store(data->constants, constants);
    urd::store(data->predicates, predicates);
    urd::store(data->functions, functions);
    urd::store(data->actions, actions);
    urd::store(data->durative_actions, durative_actions);
    urd::store(data->axioms, axioms);
    if (constraints)
        urd::store(data->constraints, *constraints);
    return repo().insert(*data, m_builder).first;
}

formalism::TaskView AstBuilder::build_task(const ast::Task& task)
{
    if (task.domain_name.text != m_domain_context.domain_name)
        m_diagnostics.throw_at(task.domain_name, MismatchedDomainError(m_domain_context.domain_name, task.domain_name.text));

    auto requirements = parse_requirements(task.requirements);
    if (m_options.strict)
    {
        // Tasks inherit the domain's requirements; only additional ones belong here.
        for (const auto& node : task.requirements)
        {
            const auto kind = requirement_kind(node, m_diagnostics);
            for (const auto domain_requirement : m_domain_context.get_domain().requirements())
                if (domain_requirement.kind() == kind)
                    m_diagnostics.throw_at(node.name, RedundantRequirementError(node.name.text));
        }
    }
    auto types = parse_type_hierarchy(task.types);
    auto predicates = parse_predicates(task.predicates);
    auto objects = parse_objects(task.objects, m_parse_context.task_objects, m_parse_context.declared_objects);
    auto initial = InitialElements {};
    for (const auto& element : task.initial)
        boost::apply_visitor([&](const auto& node) { parse_initial_element(node, initial); }, element);
    auto& initial_literals = initial.literals;
    auto& initial_function_values = initial.values;

    auto constraints = std::optional<formalism::ConstraintView> {};
    if (task.constraints)
    {
        const auto preferences = FlagScope(m_parse_context.preferences_allowed, true);
        constraints = parse_constraint(*task.constraints);
    }

    auto goal = std::optional<formalism::ConditionView> {};
    if (task.goal)
        goal = parse_goal(*task.goal, constraints);

    auto metric = std::optional<formalism::MetricView> {};
    if (task.metric)
        metric = parse_metric(*task.metric);
    complete_action_costs(task, initial_function_values, metric);

    auto axioms = urd::Store<urd::sequence<formalism::AxiomView>> {};
    for (const auto& axiom : task.axioms)
    {
        checks().require_requirement(formalism::RequirementKind::DerivedPredicates, axiom);
        axioms.push_back(parse_axiom(axiom));
    }
    checks().reject_unused_requirements(task.requirements);

    auto data = formalism::checkout<formalism::Task>(m_builder);
    data->name = urd::flat_string(task.name.text);
    data->domain = m_domain_context.get_domain().index();
    urd::store(data->requirements, requirements);
    urd::store(data->types, types);
    urd::store(data->predicates, predicates);
    urd::store(data->objects, objects);
    urd::store(data->initial_literals, initial_literals);
    urd::store(data->initial_function_values, initial_function_values);
    urd::store(data->timed_initial_literals, initial.timed_literals);
    urd::store(data->initial_effects, initial.effects);
    if (goal)
        urd::store(data->goal, *goal);
    if (task.goal_reward)
    {
        checks().require_requirement(formalism::RequirementKind::Rewards, task);
        if (!m_parse_context.active_rewards && m_options.strict)
            m_diagnostics.throw_at(task, MissingRequirementError("rewards"));
        data->goal_reward = *task.goal_reward;
    }
    urd::store(data->metric, metric);
    urd::store(data->axioms, axioms);
    if (constraints)
        urd::store(data->constraints, *constraints);
    return repo().insert(*data, m_builder).first;
}

}  // namespace loki::semantic
