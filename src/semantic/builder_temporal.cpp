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
#include "loki/formalism/effects.hpp"
#include "loki/formalism/repository.hpp"
#include "loki/semantic/errors.hpp"
#include "mappings.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <variant>

// Durative actions (PDDL 2.1) and timed initial literals (PDDL 2.2).

namespace loki::semantic
{

namespace
{

formalism::TimeSpecifier time_specifier(char time)
{
    if (time == 's')
        return formalism::TimeSpecifier::Start;
    if (time == 'e')
        return formalism::TimeSpecifier::End;
    return formalism::TimeSpecifier::OverAll;
}

constexpr auto condition_times = std::array { formalism::TimeSpecifier::Start, formalism::TimeSpecifier::OverAll, formalism::TimeSpecifier::End };
constexpr auto effect_times = std::array { formalism::TimeSpecifier::Start, formalism::TimeSpecifier::End };

template<typename View>
using Buckets = std::array<urd::Store<urd::sequence<View>>, 3>;

}  // namespace

formalism::ConditionView AstBuilder::parse_condition_node(const ast::ConditionTimed& node)
{
    if (!m_parse_context.in_durative_action)
        m_diagnostics.throw_at(node, InvalidTemporalExpressionError("a timed condition occurs only in a durative action"));
    if (m_parse_context.in_timed)
        m_diagnostics.throw_at(node, InvalidTemporalExpressionError("a timed condition occurs inside another timed condition or effect"));
    const auto timed = FlagScope(m_parse_context.in_timed, true);
    const auto condition = parse_condition(node.condition.get());
    auto data = formalism::checkout<formalism::Condition<formalism::TimedTag>>(m_builder);
    data->time = time_specifier(node.time);
    urd::store(data->condition, condition);
    return repo().insert(*data, m_builder).first;
}

formalism::EffectView AstBuilder::parse_effect_node(const ast::EffectTimed& node)
{
    if (!m_parse_context.in_durative_action)
        m_diagnostics.throw_at(node, InvalidTemporalExpressionError("a timed effect occurs only in a durative action"));
    if (m_parse_context.in_timed)
        m_diagnostics.throw_at(node, InvalidTemporalExpressionError("a timed effect occurs inside another timed effect"));
    const auto timed = FlagScope(m_parse_context.in_timed, true);
    const auto effect = parse_effect(node.effect.get());
    auto data = formalism::checkout<formalism::Effect<formalism::TimedTag>>(m_builder);
    data->time = time_specifier(node.time);
    urd::store(data->effect, effect);
    return repo().insert(*data, m_builder).first;
}

formalism::FunctionExpressionView AstBuilder::parse_function_expression_node(const ast::FunctionExpressionDuration& node)
{
    if (!m_parse_context.in_durative_action)
        m_diagnostics.throw_at(node, InvalidTemporalExpressionError("?duration occurs only in a durative action"));
    auto data = formalism::checkout<formalism::FunctionExpression<formalism::DurationTag>>(m_builder);
    return repo().insert(*data, m_builder).first;
}

void AstBuilder::parse_initial_element(const ast::TimedInitialLiteral& value, InitialElements& elements)
{
    checks().require_requirement(formalism::RequirementKind::TimedInitialLiterals, value);
    if (!std::isfinite(value.time) || value.time < 0.0)
        m_diagnostics.throw_at(value, InvalidTemporalExpressionError("the time of a timed initial literal is a non-negative number"));
    const auto literal = parse_literal(value.literal);
    auto data = formalism::checkout<formalism::TimedInitialLiteral>(m_builder);
    data->time = value.time;
    data->literal = literal.index();
    elements.timed_literals.push_back(repo().insert(*data, m_builder).first);
}

formalism::DurativeActionView AstBuilder::parse_durative_action(const ast::DurativeAction& node)
{
    checks().mark_requirement_used(formalism::RequirementKind::Strips);
    checks().require_requirement(formalism::RequirementKind::DurativeActions, node);
    const auto name = key(node.name.text);
    auto scope = VariableScope(m_parse_context);
    auto parameters = parse_parameters(node.parameters);
    const auto durative = FlagScope(m_parse_context.in_durative_action, true);

    auto duration = std::optional<formalism::ConditionView> {};
    {
        const auto in_duration = FlagScope(m_parse_context.in_duration, true);
        const auto constraint = parse_condition(node.duration);
        // A conjunction of comparisons of ?duration, each possibly at start or at end.
        const auto is_comparison = [](formalism::ConditionView condition)
        {
            if (std::holds_alternative<formalism::EntityView<formalism::Condition<formalism::NumericConstraintTag>>>(condition))
                return true;
            const auto* timed = std::get_if<formalism::EntityView<formalism::Condition<formalism::TimedTag>>>(&condition);
            return timed && timed->get_time() != formalism::TimeSpecifier::OverAll
                   && std::holds_alternative<formalism::EntityView<formalism::Condition<formalism::NumericConstraintTag>>>(timed->get_condition());
        };
        auto valid = is_comparison(constraint);
        if (const auto* conjunction = std::get_if<formalism::EntityView<formalism::Condition<formalism::AndTag>>>(&constraint))
        {
            valid = true;
            for (auto conjunct : conjunction->get_conditions())
                valid = valid && is_comparison(conjunct);
            if (!conjunction->get_conditions().empty())
                duration = constraint;
        }
        else if (valid)
        {
            duration = constraint;
        }
        if (!valid)
            m_diagnostics.throw_at(node.duration, InvalidTemporalExpressionError("a duration constraint is a conjunction of comparisons of ?duration"));
    }

    auto condition = std::optional<formalism::ConditionView> {};
    if (node.condition)
    {
        const auto preferences = FlagScope(m_parse_context.preferences_allowed, true);
        condition = group_timed_condition(parse_condition(*node.condition), node);
    }
    auto effect = std::optional<formalism::EffectView> {};
    if (node.effect)
        effect = group_timed_effect(parse_effect(*node.effect), node);

    auto data = formalism::checkout<formalism::DurativeAction>(m_builder);
    data->name = urd::flat_string(name);
    data->original_name = data->name;
    urd::store(data->parameters, parameters);
    data->original_arity = data->parameters.size();
    if (duration)
        urd::store(data->duration, *duration);
    if (condition)
        urd::store(data->condition, *condition);
    if (effect)
        urd::store(data->effect, *effect);
    return repo().insert(*data, m_builder).first;
}

std::optional<formalism::ConditionView> AstBuilder::group_timed_condition(formalism::ConditionView condition, const ast::DurativeAction& node)
{
    const auto conjunction = [&](const urd::Store<urd::sequence<formalism::ConditionView>>& parts) -> formalism::ConditionView
    {
        if (parts.size() == 1)
            return parts.front();
        auto data = formalism::checkout<formalism::Condition<formalism::AndTag>>(m_builder);
        for (auto part : parts)
            urd::append(data->conditions, part);
        return repo().insert(*data, m_builder).first;
    };

    const auto split = [&](const auto& self, formalism::ConditionView node_view, Buckets<formalism::ConditionView>& buckets) -> void
    {
        std::visit(
            [&]<typename Node>(const Node& part)
            {
                if constexpr (std::same_as<Node, formalism::EntityView<formalism::Condition<formalism::AndTag>>>)
                {
                    for (auto child : part.get_conditions())
                        self(self, child, buckets);
                }
                else if constexpr (std::same_as<Node, formalism::EntityView<formalism::Condition<formalism::TimedTag>>>)
                {
                    buckets.at(static_cast<std::size_t>(part.get_time())).push_back(part.get_condition());
                }
                else if constexpr (std::same_as<Node, formalism::EntityView<formalism::Condition<formalism::PreferenceTag>>>)
                {
                    // (preference p (at t c)) is (at t (preference p c)): a preference reads one time point or the interval.
                    auto inner = Buckets<formalism::ConditionView> {};
                    self(self, part.get_condition(), inner);
                    const auto times = std::ranges::count_if(inner, [](const auto& bucket) { return !bucket.empty(); });
                    if (times != 1)
                        m_diagnostics.throw_at(node, InvalidTemporalExpressionError("a preference of a durative action reads one time point"));
                    for (const auto time : condition_times)
                    {
                        auto& bucket = inner.at(static_cast<std::size_t>(time));
                        if (bucket.empty())
                            continue;
                        auto data = formalism::checkout<formalism::Condition<formalism::PreferenceTag>>(m_builder);
                        data->name = part.get_name();
                        urd::store(data->condition, conjunction(bucket));
                        buckets.at(static_cast<std::size_t>(time)).push_back(repo().insert(*data, m_builder).first);
                    }
                }
                else if constexpr (std::same_as<Node, formalism::EntityView<formalism::Condition<formalism::ForallTag>>>)
                {
                    auto inner = Buckets<formalism::ConditionView> {};
                    self(self, part.get_condition(), inner);
                    for (const auto time : condition_times)
                    {
                        auto& bucket = inner.at(static_cast<std::size_t>(time));
                        if (bucket.empty())
                            continue;
                        auto data = formalism::checkout<formalism::Condition<formalism::ForallTag>>(m_builder);
                        for (auto parameter : part.get_parameters())
                            data->parameters.push_back(parameter.index());
                        urd::store(data->condition, conjunction(bucket));
                        buckets.at(static_cast<std::size_t>(time)).push_back(repo().insert(*data, m_builder).first);
                    }
                }
                else
                {
                    m_diagnostics.throw_at(node,
                                           InvalidTemporalExpressionError("a condition of a durative action is a conjunction of timed conditions "
                                                                          "(at start, over all, at end)"));
                }
            },
            node_view);
    };

    auto buckets = Buckets<formalism::ConditionView> {};
    split(split, condition, buckets);
    auto parts = urd::Store<urd::sequence<formalism::ConditionView>> {};
    for (const auto time : condition_times)
    {
        const auto& bucket = buckets.at(static_cast<std::size_t>(time));
        if (bucket.empty())
            continue;
        auto data = formalism::checkout<formalism::Condition<formalism::TimedTag>>(m_builder);
        data->time = time;
        urd::store(data->condition, conjunction(bucket));
        parts.push_back(repo().insert(*data, m_builder).first);
    }
    if (parts.empty())
        return std::nullopt;
    return conjunction(parts);
}

std::optional<formalism::EffectView> AstBuilder::group_timed_effect(formalism::EffectView effect, const ast::DurativeAction& node)
{
    const auto conjunction = [&](const urd::Store<urd::sequence<formalism::EffectView>>& parts) -> formalism::EffectView
    {
        if (parts.size() == 1)
            return parts.front();
        auto data = formalism::checkout<formalism::Effect<formalism::AndTag>>(m_builder);
        for (auto part : parts)
            urd::append(data->effects, part);
        return repo().insert(*data, m_builder).first;
    };

    const auto split = [&](const auto& self, formalism::EffectView node_view, Buckets<formalism::EffectView>& buckets) -> void
    {
        std::visit(
            [&]<typename Node>(const Node& part)
            {
                if constexpr (std::same_as<Node, formalism::EntityView<formalism::Effect<formalism::AndTag>>>)
                {
                    for (auto child : part.get_effects())
                        self(self, child, buckets);
                }
                else if constexpr (std::same_as<Node, formalism::EntityView<formalism::Effect<formalism::TimedTag>>>)
                {
                    buckets.at(static_cast<std::size_t>(part.get_time())).push_back(part.get_effect());
                }
                else if constexpr (std::same_as<Node, formalism::EntityView<formalism::Effect<formalism::ForallTag>>>)
                {
                    auto inner = Buckets<formalism::EffectView> {};
                    self(self, part.get_effect(), inner);
                    for (const auto time : effect_times)
                    {
                        auto& bucket = inner.at(static_cast<std::size_t>(time));
                        if (bucket.empty())
                            continue;
                        auto data = formalism::checkout<formalism::Effect<formalism::ForallTag>>(m_builder);
                        for (auto parameter : part.get_parameters())
                            data->parameters.push_back(parameter.index());
                        urd::store(data->effect, conjunction(bucket));
                        buckets.at(static_cast<std::size_t>(time)).push_back(repo().insert(*data, m_builder).first);
                    }
                }
                else if constexpr (std::same_as<Node, formalism::EntityView<formalism::Effect<formalism::WhenTag>>>)
                {
                    auto inner = Buckets<formalism::EffectView> {};
                    self(self, part.get_effect(), inner);
                    const auto condition = group_timed_condition(part.get_condition(), node);
                    for (const auto time : effect_times)
                    {
                        auto& bucket = inner.at(static_cast<std::size_t>(time));
                        if (bucket.empty())
                            continue;
                        auto data = formalism::checkout<formalism::Effect<formalism::WhenTag>>(m_builder);
                        // Timed only at the effect's own time point, the condition reads the state the effect is applied in.
                        const auto* timed = condition ? std::get_if<formalism::EntityView<formalism::Condition<formalism::TimedTag>>>(&*condition) : nullptr;
                        if (!condition)
                        {
                            auto empty = formalism::checkout<formalism::Condition<formalism::AndTag>>(m_builder);
                            urd::store(data->condition, repo().insert(*empty, m_builder).first);
                        }
                        else if (timed && timed->get_time() == time)
                            urd::store(data->condition, timed->get_condition());
                        else
                            urd::store(data->condition, *condition);
                        urd::store(data->effect, conjunction(bucket));
                        buckets.at(static_cast<std::size_t>(time)).push_back(repo().insert(*data, m_builder).first);
                    }
                }
                else
                {
                    m_diagnostics.throw_at(
                        node,
                        InvalidTemporalExpressionError("an effect of a durative action is a conjunction of timed effects (at start, at end)"));
                }
            },
            node_view);
    };

    auto buckets = Buckets<formalism::EffectView> {};
    split(split, effect, buckets);
    auto parts = urd::Store<urd::sequence<formalism::EffectView>> {};
    for (const auto time : effect_times)
    {
        const auto& bucket = buckets.at(static_cast<std::size_t>(time));
        if (bucket.empty())
            continue;
        auto data = formalism::checkout<formalism::Effect<formalism::TimedTag>>(m_builder);
        data->time = time;
        urd::store(data->effect, conjunction(bucket));
        parts.push_back(repo().insert(*data, m_builder).first);
    }
    if (parts.empty())
        return std::nullopt;
    return conjunction(parts);
}

}  // namespace loki::semantic
