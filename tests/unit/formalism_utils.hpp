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
#include <vector>
#include <yggdrasil/containers/associative_containers.hpp>

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
    if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::AndTag>>> || std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::OneOfTag>>>)
    {
        auto result = std::size_t {};
        for (auto child : node.get_effects())
            result += count_effect_nodes<Target>(child);
        return result;
    }
    else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::ForallTag>>> || std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::WhenTag>>>)
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
    return ygg::visit(
        [&](const auto& node) -> std::size_t
        {
            using Node = std::decay_t<decltype(node)>;
            auto result = count_nested_effect_nodes<Target>(node);
            if constexpr (std::is_same_v<Node, formalism::EntityView<Target>>)
                ++result;
            return result;
        },
        effect.get_variant());
}

template<typename Target>
std::size_t count_condition_nodes(formalism::ConditionView condition)
{
    return ygg::visit(
        [&](const auto& node) -> std::size_t
        {
            using Node = std::decay_t<decltype(node)>;
            auto result = std::size_t {};
            if constexpr (std::is_same_v<Node, formalism::EntityView<Target>>)
            {
                ++result;
            }
            if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::AndTag>>> || std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::OrTag>>>)
            {
                for (auto child : node.get_conditions())
                    result += count_condition_nodes<Target>(child);
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::NotTag>>>)
            {
                result += count_condition_nodes<Target>(node.get_condition());
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::ImplyTag>>>)
            {
                result += count_condition_nodes<Target>(node.get_left());
                result += count_condition_nodes<Target>(node.get_right());
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::ExistsTag>>> || std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::ForallTag>>>)
            {
                result += count_condition_nodes<Target>(node.get_condition());
            }
            return result;
        },
        condition.get_variant());
}

inline bool contains_not_or_imply(formalism::ConditionView condition)
{
    return count_condition_nodes<formalism::Condition<formalism::NotTag>>(condition) > 0 || count_condition_nodes<formalism::Condition<formalism::ImplyTag>>(condition) > 0;
}

inline bool contains_forall(formalism::ConditionView condition) { return count_condition_nodes<formalism::Condition<formalism::ForallTag>>(condition) > 0; }

inline bool contains_exists(formalism::ConditionView condition) { return count_condition_nodes<formalism::Condition<formalism::ExistsTag>>(condition) > 0; }

inline std::size_t count_effect_when(formalism::EffectView effect) { return count_effect_nodes<formalism::Effect<formalism::WhenTag>>(effect); }

inline bool is_effect_and(formalism::EffectView effect) { return effect.get_variant().is<ygg::Index<formalism::Effect<formalism::AndTag>>>(); }

inline bool condition_mentions_predicate(formalism::ConditionView condition, const std::string& name)
{
    return ygg::visit(
        Overloaded {
            [&](formalism::EntityView<formalism::Condition<formalism::LiteralTag>> node) { return std::string(node.get_literal().get_atom().get_predicate().get_name()) == name; },
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
        condition.get_variant());
}

// Conjunction of literals and numeric constraints; what grounding-based consumers require.
inline bool is_conjunctive(formalism::ConditionView condition)
{
    return ygg::visit(
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
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::LiteralTag>>> || std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::NumericConstraintTag>>>)
                return true;
            else
                return false;
        },
        condition.get_variant());
}

inline bool writes_function_named(formalism::EffectView effect, std::string_view name)
{
    return ygg::visit(
        [&](const auto& node) -> bool
        {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::NumericTag>>>)
                return std::string_view(node.get_function().get_function().get_name()) == name;
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::AndTag>>> || std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::OneOfTag>>>)
            {
                for (auto child : node.get_effects())
                    if (writes_function_named(child, name))
                        return true;
                return false;
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::ForallTag>>> || std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::WhenTag>>>)
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
        effect.get_variant());
}

inline bool has_predicate_named(formalism::DomainView domain, std::string_view name)
{
    for (auto predicate : domain.get_predicates())
        if (std::string_view(predicate.get_name()) == name)
            return true;
    return false;
}

inline std::optional<ygg::Index<formalism::Predicate>> predicate_named(formalism::DomainView domain, const std::string& name)
{
    for (auto predicate : domain.get_predicates())
        if (std::string(predicate.get_name()) == name)
            return predicate.get_index();
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
{
    return has_requirement_kind(domain.get_requirements(), kind);
}

inline std::optional<std::string> object_term_name(formalism::TermView term)
{
    return ygg::visit(
        Overloaded {
            [](formalism::ObjectView object) -> std::optional<std::string> { return std::string(object.get_name()); },
            [](auto) -> std::optional<std::string> { return std::nullopt; },
        },
        term.get_variant());
}

inline std::optional<std::string> variable_term_name(formalism::TermView term)
{
    return ygg::visit(
        Overloaded {
            [&](formalism::VariableView variable) -> std::optional<std::string> { return std::string(variable.get_name()); },
            [&](auto) -> std::optional<std::string> { return std::nullopt; },
        },
        term.get_variant());
}

inline std::optional<ygg::Index<formalism::Object>> object_term(formalism::TermView term)
{
    return ygg::visit(
        Overloaded {
            [](formalism::ObjectView object) -> std::optional<ygg::Index<formalism::Object>> { return object.get_index(); },
            [](auto) -> std::optional<ygg::Index<formalism::Object>> { return std::nullopt; },
        },
        term.get_variant());
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
        if (left && right && left->get_value() == right->get_value())
            ++result;
    }
    return result;
}

template<typename LiteralRange>
bool initial_literals_use_predicate(const LiteralRange& literals, const std::string& name, ygg::Index<formalism::Predicate> expected)
{
    auto found = false;
    for (auto literal : literals)
    {
        const auto predicate = literal.get_atom().get_predicate();
        if (std::string(predicate.get_name()) != name)
            continue;
        found = true;
        if (predicate.get_index() != expected)
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
    auto names = ygg::UnorderedSet<std::string> {};
    for (auto object : domain.get_constants())
        names.insert(std::string(object.get_name()));
    for (auto object : task.get_objects())
        names.insert(std::string(object.get_name()));
    return names.size();
}

template<typename Views>
void expect_contiguous_indices(Views views, const std::string& label, bool require_order = true)
{
    SCOPED_TRACE(label);

    using View = std::remove_cvref_t<decltype(*views.begin())>;
    auto local = std::vector<View> {};
    local.reserve(views.size());
    for (auto view : views)
        if (&views.get_context().get_canonical_context(view.get_index()) == &views.get_context())
            local.push_back(view);
    if (local.empty())
        return;

    auto follows = [](auto previous, auto current)
    {
        using Index = decltype(previous.get_index());
        return current.get_index() == Index(previous.get_index().get_value() + 1);
    };

    auto sorted = local;
    std::sort(sorted.begin(), sorted.end(), [](auto lhs, auto rhs) { return lhs.get_index() < rhs.get_index(); });
    for (size_t i = 1; i < sorted.size(); ++i)
    {
        if (!follows(sorted[i - 1], sorted[i]))
        {
            ADD_FAILURE() << label << " has a local index gap between " << sorted[i - 1].get_index().get_value() << " and "
                          << sorted[i].get_index().get_value();
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
                          << ": previous=" << local[i - 1].get_index().get_value() << ", current=" << local[i].get_index().get_value();
            break;
        }
    }
}

inline void expect_contiguous_domain_indices(formalism::DomainView domain)
{
    expect_contiguous_indices(domain.get_requirements(), "domain.requirements");
    expect_contiguous_indices(domain.get_types(), "domain.types", false);
    expect_contiguous_indices(domain.get_constants(), "domain.constants");
    expect_contiguous_indices(domain.get_predicates(), "domain.predicates");
    expect_contiguous_indices(domain.get_functions(), "domain.functions");
    expect_contiguous_indices(domain.get_actions(), "domain.actions");
    expect_contiguous_indices(domain.get_axioms(), "domain.axioms");
}

inline void expect_contiguous_task_indices(formalism::TaskView task)
{
    expect_contiguous_indices(task.get_requirements(), "task.requirements");
    expect_contiguous_indices(task.get_objects(), "task.objects");
    expect_contiguous_indices(task.get_initial_literals(), "task.initial_literals");
    expect_contiguous_indices(task.get_initial_function_values(), "task.initial_function_values");
    expect_contiguous_indices(task.get_axioms(), "task.axioms");
}

}  // namespace loki::tests

#endif
