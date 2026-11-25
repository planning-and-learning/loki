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
#include "checks.hpp"
#include "context.hpp"
#include "diagnostics.hpp"
#include "loki/ast/ast.hpp"
#include "loki/formalism/conditions.hpp"
#include "loki/formalism/constraints.hpp"
#include "loki/formalism/repository.hpp"
#include "loki/semantic/errors.hpp"
#include "mappings.hpp"

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>

// Constraints and preferences (PDDL 3).

namespace loki::semantic
{

namespace
{

struct ModalSignature
{
    formalism::ConstraintOperator operation;
    std::size_t numbers;
    std::size_t conditions;
};

/// The operator of a modal constraint (ast::ConstraintModal) with its numbers of time bounds and conditions.
ModalSignature modal_signature(char operation)
{
    using enum formalism::ConstraintOperator;
    switch (operation)
    {
        case 'E':
            return { AtEnd, 0, 1 };
        case 'A':
            return { Always, 0, 1 };
        case 'S':
            return { Sometime, 0, 1 };
        case 'W':
            return { Within, 1, 1 };
        case 'O':
            return { AtMostOnce, 0, 1 };
        case 'a':
            return { SometimeAfter, 0, 2 };
        case 'b':
            return { SometimeBefore, 0, 2 };
        case 'w':
            return { AlwaysWithin, 1, 2 };
        case 'h':
            return { HoldDuring, 2, 1 };
        case 'H':
            return { HoldAfter, 1, 1 };
        case 'n':
            return { Next, 0, 1 };
        case 'N':
            return { WeakNext, 0, 1 };
        case 'u':
            return { Until, 2, 2 };
        case 'r':
            return { Release, 2, 2 };
        case 'y':
            return { Yesterday, 0, 1 };
        case 'Y':
            return { WeakYesterday, 0, 1 };
        case 's':
            return { Since, 2, 2 };
        case 't':
            return { Triggers, 2, 2 };
        case 'o':
            return { Once, 2, 1 };
        default:
            return { Historically, 2, 1 };
    }
}

/// Whether a formula holds a modal operator (with preferences: whether it holds one of those too).
bool contains_modal(const ast::Constraint& constraint, bool or_preference)
{
    return boost::apply_visitor(
        [&]<typename Node>(const Node& node) -> bool
        {
            if constexpr (std::same_as<Node, ast::ConstraintModal>)
                return true;
            else if constexpr (std::same_as<Node, ast::ConstraintPreference>)
                return or_preference || contains_modal(node.constraint.get(), or_preference);
            else if constexpr (std::same_as<Node, ast::ConstraintAnd> || std::same_as<Node, ast::ConstraintOr>)
                return std::ranges::any_of(node.constraints, [&](const auto& child) { return contains_modal(child.get(), or_preference); });
            else if constexpr (std::same_as<Node, ast::ConstraintImply>)
                return contains_modal(node.left.get(), or_preference) || contains_modal(node.right.get(), or_preference);
            else if constexpr (std::same_as<Node, ast::ConstraintState>)
                return false;
            else
                return contains_modal(node.constraint.get(), or_preference);
        },
        constraint);
}

template<typename Target, typename Source>
Target tagged(const Source& source)
{
    auto result = Target {};
    static_cast<boost::spirit::x3::position_tagged&>(result) = static_cast<const boost::spirit::x3::position_tagged&>(source);
    return result;
}

template<typename Node>
ast::Condition as_condition(Node node)
{
    // Keeps the node's position: diagnostics point into the source.
    const auto position = static_cast<const boost::spirit::x3::position_tagged&>(node);
    auto result = ast::Condition {};
    result = std::move(node);
    static_cast<boost::spirit::x3::position_tagged&>(result) = position;
    return result;
}

/// A formula without modal operators as the goal description it is (preferences included).
ast::Condition to_condition(const ast::Constraint& constraint)
{
    return boost::apply_visitor(
        [&]<typename Node>(const Node& node) -> ast::Condition
        {
            if constexpr (std::same_as<Node, ast::ConstraintState>)
                return node.condition;
            else if constexpr (std::same_as<Node, ast::ConstraintAnd> || std::same_as<Node, ast::ConstraintOr>)
            {
                using Target = std::conditional_t<std::same_as<Node, ast::ConstraintAnd>, ast::ConditionAnd, ast::ConditionOr>;
                auto result = tagged<Target>(node);
                for (const auto& child : node.constraints)
                    result.conditions.push_back(to_condition(child.get()));
                return as_condition(result);
            }
            else if constexpr (std::same_as<Node, ast::ConstraintNot>)
            {
                auto result = tagged<ast::ConditionNot>(node);
                result.condition = to_condition(node.constraint.get());
                return as_condition(result);
            }
            else if constexpr (std::same_as<Node, ast::ConstraintImply>)
            {
                auto result = tagged<ast::ConditionImply>(node);
                result.left = to_condition(node.left.get());
                result.right = to_condition(node.right.get());
                return as_condition(result);
            }
            else if constexpr (std::same_as<Node, ast::ConstraintExists> || std::same_as<Node, ast::ConstraintForall>)
            {
                using Target = std::conditional_t<std::same_as<Node, ast::ConstraintExists>, ast::ConditionExists, ast::ConditionForall>;
                auto result = tagged<Target>(node);
                result.parameters = node.parameters;
                result.condition = to_condition(node.constraint.get());
                return as_condition(result);
            }
            else if constexpr (std::same_as<Node, ast::ConstraintPreference>)
            {
                auto result = tagged<ast::ConditionPreference>(node);
                result.name = node.name;
                result.condition = to_condition(node.constraint.get());
                return as_condition(result);
            }
            else
                throw std::logic_error("a modal formula is no goal description");
        },
        constraint);
}

}  // namespace

template<typename Node>
std::string AstBuilder::preference_name(const Node& node)
{
    if (m_parse_context.in_preference)
        m_diagnostics.throw_at(node, InvalidConstraintError("a preference occurs inside another preference"));
    if (!node.name)
        return {};
    auto result = key(node.name->text);
    m_domain_context.preference_names.insert(result);
    return result;
}

formalism::ConditionView AstBuilder::parse_condition_node(const ast::ConditionPreference& node)
{
    checks().require_requirement(formalism::RequirementKind::Preferences, node);
    if (!m_parse_context.preferences_allowed)
        m_diagnostics.throw_at(node, InvalidConstraintError("a preference occurs only in a precondition, a goal or a constraint"));
    const auto name = preference_name(node);
    const auto in_preference = FlagScope(m_parse_context.in_preference, true);
    const auto condition = parse_condition(node.condition.get());
    auto data = formalism::checkout<formalism::Condition<formalism::PreferenceTag>>(m_builder);
    data->name = urd::flat_string(name);
    urd::store(data->condition, condition);
    return repo().insert(*data, m_builder).first;
}

formalism::FunctionExpressionView AstBuilder::parse_function_expression_node(const ast::FunctionExpressionIsViolated& node)
{
    checks().require_requirement(formalism::RequirementKind::Preferences, node);
    const auto name = key(node.name.text);
    if (!m_domain_context.preference_names.contains(name))
        m_diagnostics.throw_at(node.name, InvalidConstraintError("no preference is named " + name));
    auto data = formalism::checkout<formalism::FunctionExpression<formalism::IsViolatedTag>>(m_builder);
    data->name = urd::flat_string(name);
    return repo().insert(*data, m_builder).first;
}

formalism::ConstraintView AstBuilder::parse_constraint(const ast::Constraint& constraint)
{
    // A formula without modal operators and preferences is a goal description, read as one (its checks and diagnostics);
    // the empty conjunction stays PDDL 3's empty constraint.
    const auto* conjunction = boost::get<ast::ConstraintAnd>(&constraint);
    const auto empty = conjunction && conjunction->constraints.empty();
    if (!empty && !contains_modal(constraint, true))
    {
        // PDDL 3 has goal descriptions only as a modal operator's operands.
        if (!m_parse_context.in_modal_operand)
            checks().require_requirement(formalism::RequirementKind::TrajectoryFormulas, constraint);
        const auto operand = FlagScope(m_parse_context.in_modal_operand, false);
        const auto preferences = FlagScope(m_parse_context.preferences_allowed, false);
        auto data = formalism::checkout<formalism::Constraint<formalism::StateTag>>(m_builder);
        urd::store(data->condition, parse_condition(to_condition(constraint)));
        return repo().insert(*data, m_builder).first;
    }
    // A formula nested in an operand of a PDDL 3 operator, e.g. (always (sometime p)), is beyond PDDL 3.
    if (m_parse_context.in_modal_operand)
        checks().require_requirement(formalism::RequirementKind::TrajectoryFormulas, constraint);
    const auto operand = FlagScope(m_parse_context.in_modal_operand, false);
    return boost::apply_visitor([&](const auto& node) { return parse_constraint_node(node); }, constraint);
}

formalism::ConstraintView AstBuilder::parse_constraint_node(const ast::ConstraintState&)
{
    // Holds no modal operator: parse_constraint reads it.
    throw std::logic_error("a goal description is read by parse_constraint");
}

formalism::ConstraintView AstBuilder::parse_constraint_node(const ast::ConstraintAnd& node)
{
    auto data = formalism::checkout<formalism::Constraint<formalism::AndTag>>(m_builder);
    for (const auto& child : node.constraints)
        urd::append(data->constraints, parse_constraint(child.get()));
    return repo().insert(*data, m_builder).first;
}

formalism::ConstraintView AstBuilder::parse_constraint_node(const ast::ConstraintOr& node)
{
    checks().require_requirement(formalism::RequirementKind::TrajectoryFormulas, node);
    checks().require_requirement(formalism::RequirementKind::DisjunctivePreconditions, node);
    const auto preferences = FlagScope(m_parse_context.preferences_allowed, false);
    auto data = formalism::checkout<formalism::Constraint<formalism::OrTag>>(m_builder);
    for (const auto& child : node.constraints)
        urd::append(data->constraints, parse_constraint(child.get()));
    return repo().insert(*data, m_builder).first;
}

formalism::ConstraintView AstBuilder::parse_constraint_node(const ast::ConstraintNot& node)
{
    checks().require_requirement(formalism::RequirementKind::TrajectoryFormulas, node);
    const auto preferences = FlagScope(m_parse_context.preferences_allowed, false);
    return negated(parse_constraint(node.constraint.get()));
}

formalism::ConstraintView AstBuilder::parse_constraint_node(const ast::ConstraintImply& node)
{
    // (imply a b) is (or (not a) b).
    checks().require_requirement(formalism::RequirementKind::TrajectoryFormulas, node);
    checks().require_requirement(formalism::RequirementKind::DisjunctivePreconditions, node);
    const auto preferences = FlagScope(m_parse_context.preferences_allowed, false);
    auto data = formalism::checkout<formalism::Constraint<formalism::OrTag>>(m_builder);
    urd::append(data->constraints, negated(parse_constraint(node.left.get())));
    urd::append(data->constraints, parse_constraint(node.right.get()));
    return repo().insert(*data, m_builder).first;
}

formalism::ConstraintView AstBuilder::negated(formalism::ConstraintView constraint)
{
    auto data = formalism::checkout<formalism::Constraint<formalism::NotTag>>(m_builder);
    urd::store(data->constraint, constraint);
    return repo().insert(*data, m_builder).first;
}

formalism::ConstraintView AstBuilder::parse_constraint_node(const ast::ConstraintForall& node)
{
    checks().require_requirement(formalism::RequirementKind::UniversalPreconditions, node);
    auto scope = VariableScope(m_parse_context);
    auto parameters = parse_parameters(node.parameters);
    const auto child = parse_constraint(node.constraint.get());
    auto data = formalism::checkout<formalism::Constraint<formalism::ForallTag>>(m_builder);
    urd::store(data->parameters, parameters);
    urd::store(data->constraint, child);
    return repo().insert(*data, m_builder).first;
}

formalism::ConstraintView AstBuilder::parse_constraint_node(const ast::ConstraintExists& node)
{
    checks().require_requirement(formalism::RequirementKind::TrajectoryFormulas, node);
    checks().require_requirement(formalism::RequirementKind::ExistentialPreconditions, node);
    const auto preferences = FlagScope(m_parse_context.preferences_allowed, false);
    auto scope = VariableScope(m_parse_context);
    auto parameters = parse_parameters(node.parameters);
    const auto child = parse_constraint(node.constraint.get());
    auto data = formalism::checkout<formalism::Constraint<formalism::ExistsTag>>(m_builder);
    urd::store(data->parameters, parameters);
    urd::store(data->constraint, child);
    return repo().insert(*data, m_builder).first;
}

formalism::ConstraintView AstBuilder::parse_constraint_node(const ast::ConstraintPreference& node)
{
    checks().require_requirement(formalism::RequirementKind::Preferences, node);
    // As in a goal: a preference stands only under conjunctions and universal quantifiers.
    if (!m_parse_context.preferences_allowed)
        m_diagnostics.throw_at(node, InvalidConstraintError("a preference occurs only under and and forall"));
    const auto name = preference_name(node);
    const auto in_preference = FlagScope(m_parse_context.in_preference, true);
    const auto child = parse_constraint(node.constraint.get());
    auto data = formalism::checkout<formalism::Constraint<formalism::PreferenceTag>>(m_builder);
    data->name = urd::flat_string(name);
    urd::store(data->constraint, child);
    return repo().insert(*data, m_builder).first;
}

formalism::ConstraintView AstBuilder::parse_constraint_node(const ast::ConstraintModal& node)
{
    const auto signature = modal_signature(node.operation);
    const auto operation = std::string(formalism::to_string(signature.operation));
    if (formalism::is_trajectory_operator(signature.operation))
        checks().require_requirement(formalism::RequirementKind::TrajectoryFormulas, node);
    else
        checks().require_requirement(formalism::RequirementKind::Constraints, node);
    if (node.constraints.size() != signature.conditions)
        m_diagnostics.throw_at(node, InvalidConstraintError(operation + " takes " + std::to_string(signature.conditions) + " operands"));
    if (formalism::is_trajectory_operator(signature.operation))
    {
        // An interval (interval lo hi) with 0 <= lo <= hi, hi possibly inf; next and yesterday take none.
        if (!node.numbers.empty())
        {
            if (!formalism::takes_interval(signature.operation))
                m_diagnostics.throw_at(node, InvalidConstraintError(operation + " takes no interval"));
            const auto lower = node.numbers[0];
            const auto upper = node.numbers[1];
            if (!std::isfinite(lower) || lower < 0.0 || std::isnan(upper) || upper < lower)
                m_diagnostics.throw_at(node, InvalidConstraintError("the interval of " + operation + " is (interval lo hi) with 0 <= lo <= hi"));
        }
    }
    else
    {
        if (node.numbers.size() != signature.numbers)
            m_diagnostics.throw_at(node,
                                   InvalidConstraintError(operation + " takes " + std::to_string(signature.numbers) + " numbers and "
                                                          + std::to_string(signature.conditions) + " conditions"));
        for (const auto number : node.numbers)
            if (!std::isfinite(number) || number < 0.0)
                m_diagnostics.throw_at(node, InvalidConstraintError("the time bounds of " + operation + " are non-negative numbers"));
    }
    // The operands of a modal operator hold no preference; PDDL 3's own are goal descriptions.
    const auto preferences = FlagScope(m_parse_context.preferences_allowed, false);
    auto data = formalism::checkout<formalism::Constraint<formalism::ModalTag>>(m_builder);
    data->operation = signature.operation;
    for (const auto number : node.numbers)
        data->numbers.push_back(number);
    for (const auto& operand : node.constraints)
    {
        const auto in_operand = FlagScope(m_parse_context.in_modal_operand, !formalism::is_trajectory_operator(signature.operation));
        urd::append(data->constraints, parse_constraint(operand.get()));
    }
    return repo().insert(*data, m_builder).first;
}

std::optional<formalism::ConditionView> AstBuilder::parse_goal(const ast::Constraint& goal, std::optional<formalism::ConstraintView>& constraints)
{
    // A goal without modal operators is a goal description on the final state, as always.
    if (!contains_modal(goal, false))
    {
        const auto preferences = FlagScope(m_parse_context.preferences_allowed, true);
        return parse_condition(to_condition(goal));
    }
    // A formula over the state trajectory: the plan's trace satisfies it, read from the initial state, so it is a hard
    // constraint of the task (its preferences soft ones), conjoined with the task's constraints.
    checks().require_requirement(formalism::RequirementKind::TrajectoryFormulas, goal);
    const auto preferences = FlagScope(m_parse_context.preferences_allowed, true);
    const auto formula = parse_constraint(goal);
    if (!constraints)
    {
        constraints = formula;
        return std::nullopt;
    }
    auto data = formalism::checkout<formalism::Constraint<formalism::AndTag>>(m_builder);
    for (const auto part : { *constraints, formula })
    {
        if (const auto* conjunction = std::get_if<formalism::EntityView<formalism::Constraint<formalism::AndTag>>>(&part))
            for (const auto child : conjunction->get_constraints())
                urd::append(data->constraints, child);
        else
            urd::append(data->constraints, part);
    }
    constraints = formalism::ConstraintView(repo().insert(*data, m_builder).first);
    return std::nullopt;
}

std::optional<formalism::ConstraintView> AstBuilder::parse_constraints(const urd::Store<urd::sequence<ast::Constraint>>& constraints)
{
    const auto preferences = FlagScope(m_parse_context.preferences_allowed, true);
    if (constraints.empty())
        return std::nullopt;
    if (constraints.size() == 1)
        return parse_constraint(constraints.front());
    auto data = formalism::checkout<formalism::Constraint<formalism::AndTag>>(m_builder);
    for (const auto& constraint : constraints)
        urd::append(data->constraints, parse_constraint(constraint));
    return formalism::ConstraintView(repo().insert(*data, m_builder).first);
}

}  // namespace loki::semantic
