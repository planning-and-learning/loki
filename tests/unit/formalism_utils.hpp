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

#ifndef LOKI_TESTS_FORMALISM_UTILS_HPP_
#define LOKI_TESTS_FORMALISM_UTILS_HPP_

#include <algorithm>
#include <gtest/gtest.h>
#include <loki/formalism/repository.hpp>
#include <loki/formalism/views.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <urd/repr/maps/unordered_flat_map.hpp>
#include <urd/repr/values/std_set.hpp>
#include <urd/repr/values/unordered_flat_set.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>

namespace loki::tests
{

template<class... Ts>
struct Overloaded : Ts...
{
    using Ts::operator()...;
};
template<class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

template<typename Target>
std::size_t count_effect_nodes(formalism::EffectView effect);

template<typename Target, typename Node>
std::size_t count_nested_effect_nodes(const Node& node)
{
    if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::AndTag>>>
                  || std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::OneOfTag>>>)
    {
        auto result = std::size_t {};
        for (auto child : node.get_effects())
            result += count_effect_nodes<Target>(child);
        return result;
    }
    else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::ForallTag>>>
                       || std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::WhenTag>>>)
    {
        return count_effect_nodes<Target>(node.get_effect());
    }
    else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::ProbabilisticTag>>>)
    {
        auto result = std::size_t {};
        for (auto alternative : node.get_alternatives())
            result += count_effect_nodes<Target>(alternative.get_effect());
        return result;
    }
    else
    {
        return 0;
    }
}

template<typename Target>
std::size_t count_effect_nodes(formalism::EffectView effect)
{
    return std::visit(
        [&](const auto& node) -> std::size_t
        {
            using Node = std::decay_t<decltype(node)>;
            auto result = count_nested_effect_nodes<Target>(node);
            if constexpr (std::is_same_v<Node, formalism::EntityView<Target>>)
                ++result;
            return result;
        },
        effect);
}

template<typename Target>
std::size_t count_condition_nodes(formalism::ConditionView condition)
{
    return std::visit(
        [&](const auto& node) -> std::size_t
        {
            using Node = std::decay_t<decltype(node)>;
            auto result = std::size_t {};
            if constexpr (std::is_same_v<Node, formalism::EntityView<Target>>)
            {
                ++result;
            }
            if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::AndTag>>>
                          || std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::OrTag>>>)
            {
                for (auto child : node.get_conditions())
                    result += count_condition_nodes<Target>(child);
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::NotTag>>>
                               || std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::ExistsTag>>>
                               || std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::ForallTag>>>)
            {
                result += count_condition_nodes<Target>(node.get_condition());
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::ImplyTag>>>)
            {
                result += count_condition_nodes<Target>(node.get_left());
                result += count_condition_nodes<Target>(node.get_right());
            }
            return result;
        },
        condition);
}

inline bool contains_not_or_imply(formalism::ConditionView condition)
{
    return count_condition_nodes<formalism::Condition<formalism::NotTag>>(condition) > 0
           || count_condition_nodes<formalism::Condition<formalism::ImplyTag>>(condition) > 0;
}

inline bool contains_forall(formalism::ConditionView condition) { return count_condition_nodes<formalism::Condition<formalism::ForallTag>>(condition) > 0; }

inline bool contains_exists(formalism::ConditionView condition) { return count_condition_nodes<formalism::Condition<formalism::ExistsTag>>(condition) > 0; }

inline std::size_t count_effect_when(formalism::EffectView effect) { return count_effect_nodes<formalism::Effect<formalism::WhenTag>>(effect); }

inline bool is_effect_and(formalism::EffectView effect) { return std::holds_alternative<formalism::EntityView<formalism::Effect<formalism::AndTag>>>(effect); }

/// The effect node of Tag: the effect itself, or the child holding it of a top-level and (the shape unit action costs
/// give, an increase of total-cost beside the effect).
template<typename Tag>
formalism::EntityView<formalism::Effect<Tag>> effect_node(formalism::EffectView effect)
{
    using Node = formalism::EntityView<formalism::Effect<Tag>>;
    if (is_effect_and(effect))
        for (auto child : std::get<formalism::EntityView<formalism::Effect<formalism::AndTag>>>(effect).get_effects())
            if (std::holds_alternative<Node>(child))
                return std::get<Node>(child);
    return std::get<Node>(effect);
}

inline bool condition_mentions_predicate(formalism::ConditionView condition, const std::string& name)
{
    return std::visit(
        Overloaded {
            [&](formalism::EntityView<formalism::Condition<formalism::LiteralTag>> node)
            { return std::string(node.get_literal().get_atom().get_predicate().get_name()) == name; },
            [&](formalism::EntityView<formalism::Condition<formalism::NumericConstraintTag>>) { return false; },
            [&](formalism::EntityView<formalism::Condition<formalism::NotTag>> node) { return condition_mentions_predicate(node.get_condition(), name); },
            [&](formalism::EntityView<formalism::Condition<formalism::ImplyTag>> node)
            { return condition_mentions_predicate(node.get_left(), name) || condition_mentions_predicate(node.get_right(), name); },
            [&](formalism::EntityView<formalism::Condition<formalism::AndTag>> node)
            {
                for (auto child : node.get_conditions())
                    if (condition_mentions_predicate(child, name))
                        return true;
                return false;
            },
            [&](formalism::EntityView<formalism::Condition<formalism::OrTag>> node)
            {
                for (auto child : node.get_conditions())
                    if (condition_mentions_predicate(child, name))
                        return true;
                return false;
            },
            [&](formalism::EntityView<formalism::Condition<formalism::ExistsTag>> node) { return condition_mentions_predicate(node.get_condition(), name); },
            [&](formalism::EntityView<formalism::Condition<formalism::ForallTag>> node) { return condition_mentions_predicate(node.get_condition(), name); },
        },
        condition);
}

// Conjunction of literals and numeric constraints; what grounding-based consumers require.
inline bool is_conjunctive(formalism::ConditionView condition)
{
    return std::visit(
        [&](const auto& node) -> bool
        {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::AndTag>>>)
            {
                for (auto child : node.get_conditions())
                    if (!is_conjunctive(child))
                        return false;
                return true;
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::LiteralTag>>>
                               || std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::NumericConstraintTag>>>)
                return true;
            else
                return false;
        },
        condition);
}

inline bool writes_function_named(formalism::EffectView effect, std::string_view name)
{
    return std::visit(
        [&](const auto& node) -> bool
        {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::NumericTag>>>)
                return std::string_view(node.get_function().get_function().get_name()) == name;
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::AndTag>>>
                               || std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::OneOfTag>>>)
            {
                for (auto child : node.get_effects())
                    if (writes_function_named(child, name))
                        return true;
                return false;
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::ForallTag>>>
                               || std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::WhenTag>>>)
                return writes_function_named(node.get_effect(), name);
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::ProbabilisticTag>>>)
            {
                for (auto alternative : node.get_alternatives())
                    if (writes_function_named(alternative.get_effect(), name))
                        return true;
                return false;
            }
            else
                return false;
        },
        effect);
}

inline bool has_predicate_named(formalism::DomainView domain, std::string_view name)
{
    for (auto predicate : domain.get_predicates())
        if (std::string_view(predicate.get_name()) == name)
            return true;
    return false;
}

inline std::optional<urd::index<formalism::Predicate>> predicate_named(formalism::DomainView domain, const std::string& name)
{
    for (auto predicate : domain.get_predicates())
        if (std::string(predicate.get_name()) == name)
            return predicate.index();
    return std::nullopt;
}

inline bool has_function_named(formalism::DomainView domain, std::string_view name)
{
    for (auto function : domain.get_functions())
        if (std::string_view(function.get_name()) == name)
            return true;
    return false;
}

inline bool has_equality_predicate(formalism::DomainView domain) { return has_predicate_named(domain, "="); }

template<typename RequirementRange>
bool has_requirement_kind(const RequirementRange& requirements, formalism::RequirementKind kind)
{
    for (auto requirement : requirements)
        if (requirement.get_kind() == kind)
            return true;
    return false;
}

inline bool has_requirement_kind(formalism::DomainView domain, formalism::RequirementKind kind)
{ return has_requirement_kind(domain.get_requirements(), kind); }

inline std::optional<std::string> object_term_name(formalism::TermView term)
{
    return std::visit(
        Overloaded {
            [](formalism::ObjectView object) -> std::optional<std::string> { return std::string(object.get_name()); },
            [](auto) -> std::optional<std::string> { return std::nullopt; },
        },
        term);
}

inline std::optional<std::string> variable_term_name(formalism::TermView term)
{
    return std::visit(
        Overloaded {
            [&](formalism::VariableView variable) -> std::optional<std::string> { return std::string(variable.get_name()); },
            [&](auto) -> std::optional<std::string> { return std::nullopt; },
        },
        term);
}

inline std::optional<urd::index<formalism::Object>> object_term(formalism::TermView term)
{
    return std::visit(
        Overloaded {
            [](formalism::ObjectView object) -> std::optional<urd::index<formalism::Object>> { return object.index(); },
            [](auto) -> std::optional<urd::index<formalism::Object>> { return std::nullopt; },
        },
        term);
}

// Reflexive (= o o) initialization literals produced by materialize_equality.
template<typename LiteralRange>
std::size_t count_equality_literals(const LiteralRange& literals)
{
    auto result = std::size_t { 0 };
    for (auto literal : literals)
    {
        if (!literal.get_polarity())
            continue;
        const auto atom = literal.get_atom();
        if (std::string(atom.get_predicate().get_name()) != "=" || atom.get_terms().size() != 2)
            continue;
        const auto left = object_term(atom.get_terms()[0]);
        const auto right = object_term(atom.get_terms()[1]);
        if (left && right && left->value == right->value)
            ++result;
    }
    return result;
}

template<typename LiteralRange>
bool initial_literals_use_predicate(const LiteralRange& literals, const std::string& name, urd::index<formalism::Predicate> expected)
{
    auto found = false;
    for (auto literal : literals)
    {
        const auto predicate = literal.get_atom().get_predicate();
        if (std::string(predicate.get_name()) != name)
            continue;
        found = true;
        if (predicate.index() != expected)
            return false;
    }
    return found;
}

template<typename LiteralRange>
bool has_initial_unary_literal(const LiteralRange& literals, const std::string& predicate_name, const std::string& object_name)
{
    for (auto literal : literals)
    {
        if (!literal.get_polarity())
            continue;
        const auto atom = literal.get_atom();
        if (atom.get_terms().size() != 1 || std::string(atom.get_predicate().get_name()) != predicate_name)
            continue;
        const auto object = object_term_name(atom.get_terms()[0]);
        if (object && *object == object_name)
            return true;
    }
    return false;
}

template<typename LiteralRange>
std::size_t count_initial_literals_for_predicate(const LiteralRange& literals, const std::string& name)
{
    auto result = std::size_t { 0 };
    for (auto literal : literals)
    {
        if (std::string(literal.get_atom().get_predicate().get_name()) == name)
            ++result;
    }
    return result;
}

inline std::size_t count_unique_object_names(formalism::DomainView domain, formalism::TaskView task)
{
    auto names = urd::Store<urd::set<std::string>> {};
    for (auto object : domain.get_constants())
        names.insert(std::string(object.get_name()));
    for (auto object : task.get_objects())
        names.insert(std::string(object.get_name()));
    return names.size();
}

template<typename Views, typename Context>
void expect_contiguous_indices(Views views, const Context& context, const std::string& label, bool require_order = true)
{
    SCOPED_TRACE(label);

    using View = std::remove_cvref_t<decltype(*views.begin())>;
    auto local = urd::Store<urd::sequence<View>> {};
    local.reserve(views.size());
    for (auto view : views)
        if (&context.get_canonical_context(view.index()) == &context)
            local.push_back(view);
    if (local.empty())
        return;

    auto follows = [](auto previous, auto current)
    {
        return current.index().value == previous.index().value + 1;
    };

    auto sorted = local;
    std::sort(sorted.begin(), sorted.end(), [](auto lhs, auto rhs) { return lhs.index() < rhs.index(); });
    for (size_t i = 1; i < sorted.size(); ++i)
    {
        if (!follows(sorted[i - 1], sorted[i]))
        {
            ADD_FAILURE() << label << " has a local index gap between " << sorted[i - 1].index().value << " and "
                          << sorted[i].index().value;
            break;
        }
    }

    if (!require_order)
        return;

    for (size_t i = 1; i < local.size(); ++i)
    {
        if (!follows(local[i - 1], local[i]))
        {
            ADD_FAILURE() << label << " is not stored in contiguous local canonical index order at offset " << i
                          << ": previous=" << local[i - 1].index().value << ", current=" << local[i].index().value;
            break;
        }
    }
}

inline void expect_contiguous_domain_indices(formalism::DomainView domain)
{
    expect_contiguous_indices(domain.get_requirements(), domain.context(), "domain.requirements");
    expect_contiguous_indices(domain.get_types(), domain.context(), "domain.types", false);
    expect_contiguous_indices(domain.get_constants(), domain.context(), "domain.constants");
    expect_contiguous_indices(domain.get_predicates(), domain.context(), "domain.predicates");
    expect_contiguous_indices(domain.get_functions(), domain.context(), "domain.functions");
    expect_contiguous_indices(domain.get_actions(), domain.context(), "domain.actions");
    expect_contiguous_indices(domain.get_axioms(), domain.context(), "domain.axioms");
}

inline void expect_contiguous_task_indices(formalism::TaskView task)
{
    expect_contiguous_indices(task.get_requirements(), task.context(), "task.requirements");
    expect_contiguous_indices(task.get_objects(), task.context(), "task.objects");
    expect_contiguous_indices(task.get_initial_literals(), task.context(), "task.initial_literals");
    expect_contiguous_indices(task.get_initial_function_values(), task.context(), "task.initial_function_values");
    expect_contiguous_indices(task.get_axioms(), task.context(), "task.axioms");
}

}  // namespace loki::tests

#endif
