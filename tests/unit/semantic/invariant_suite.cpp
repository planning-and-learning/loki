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

#include "../benchmark_utils.hpp"
#include "../formalism_utils.hpp"

#include <cstdint>
#include <gtest/gtest.h>
#include <loki/formalism/formatter.hpp>
#include <loki/semantic/options.hpp>
#include <loki/semantic/parser.hpp>
#include <loki/semantic/translator.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <yggdrasil/containers/associative_containers.hpp>

namespace loki::tests
{
namespace
{

void collect_effect_binder_names(formalism::EffectView effect, ygg::UnorderedSet<std::string>& names)
{
    ygg::visit(
        [&](const auto& node)
        {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::ForallTag>>>)
            {
                for (auto parameter : node.get_parameters())
                    names.insert(std::string(parameter.get_variable().get_name()));
                collect_effect_binder_names(node.get_effect(), names);
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::AndTag>>> || std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::OneOfTag>>>)
            {
                for (auto child : node.get_effects())
                    collect_effect_binder_names(child, names);
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::WhenTag>>>)
            {
                collect_effect_binder_names(node.get_effect(), names);
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::ProbabilisticTag>>>)
            {
                for (auto alternative : node.get_alternatives())
                    collect_effect_binder_names(alternative.get_effect(), names);
            }
        },
        effect.get_variant());
}

// NNF, no quantifiers, no disjunction: the shape every translated precondition,
// axiom condition, and goal must have.
void expect_flat_condition(formalism::ConditionView condition, const std::string& what)
{
    EXPECT_FALSE(contains_not_or_imply(condition)) << what;
    EXPECT_FALSE(contains_forall(condition)) << what;
    EXPECT_FALSE(contains_exists(condition)) << what;
    EXPECT_EQ(count_condition_nodes<formalism::Condition<formalism::OrTag>>(condition), 0) << what;
}

template<typename Children>
void expect_distinct_children(const Children& children, const std::string& what)
{
    auto seen = ygg::UnorderedSet<std::uint64_t> {};
    for (auto child : children)
        EXPECT_TRUE(seen.insert(child.get_index().get_value()).second) << what << ": duplicate child in canonical list";
}

// Effect normal form: no and-in-and, merged whens with disjunction-free conditions,
// distributed foralls, aggregated numeric effects per target.
void expect_effect_normal_form(formalism::EffectView effect, bool top_level, const std::string& what)
{
    ygg::visit(
        [&](const auto& node)
        {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::AndTag>>>)
            {
                EXPECT_TRUE(top_level) << what << ": nested effect conjunction";
                expect_distinct_children(node.get_effects(), what);
                auto numeric_targets = ygg::UnorderedSet<std::string> {};
                for (auto child : node.get_effects())
                {
                    ygg::visit(
                        [&](const auto& grandchild)
                        {
                            using Grandchild = std::decay_t<decltype(grandchild)>;
                            if constexpr (std::is_same_v<Grandchild, formalism::EntityView<formalism::Effect<formalism::NumericTag>>>)
                            {
                                auto key = std::string(grandchild.get_function().get_function().get_name());
                                for (auto term : grandchild.get_function().get_terms())
                                    key += "," + std::to_string(term.get_index().get_value());
                                EXPECT_TRUE(numeric_targets.insert(key).second) << what << ": unaggregated numeric effects on target " << key;
                            }
                        },
                        child.get_variant());
                    expect_effect_normal_form(child, false, what);
                }
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::WhenTag>>>)
            {
                EXPECT_EQ(count_condition_nodes<formalism::Condition<formalism::OrTag>>(node.get_condition()), 0) << what << ": disjunctive when-condition";
                EXPECT_EQ(count_effect_nodes<formalism::Effect<formalism::WhenTag>>(node.get_effect()), 0) << what << ": nested when";
                EXPECT_FALSE(node.get_effect().get_variant().template is<ygg::Index<formalism::Effect<formalism::AndTag>>>()) << what << ": conjunctive when-body";
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::ForallTag>>>)
            {
                EXPECT_FALSE(node.get_effect().get_variant().template is<ygg::Index<formalism::Effect<formalism::AndTag>>>()) << what << ": undistributed universal effect";
                expect_effect_normal_form(node.get_effect(), false, what);
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::OneOfTag>>>)
            {
                for (auto child : node.get_effects())
                    expect_effect_normal_form(child, false, what);
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::ProbabilisticTag>>>)
            {
                for (auto alternative : node.get_alternatives())
                    expect_effect_normal_form(alternative.get_effect(), false, what);
            }
        },
        effect.get_variant());
}

template<typename AxiomRange>
void expect_axiom_invariants(const AxiomRange& axioms, const std::string& what)
{
    for (auto axiom : axioms)
    {
        const auto atom = axiom.get_head().get_atom();
        EXPECT_EQ(atom.get_predicate().get_parameters().size(), atom.get_terms().size()) << what;
        EXPECT_GE(axiom.get_parameters().size(), atom.get_terms().size()) << what;
        expect_flat_condition(axiom.get_condition(), what + ": axiom condition");
    }
}

void expect_domain_invariants(formalism::DomainView domain)
{
    auto predicate_names = ygg::UnorderedSet<std::string> {};
    for (auto predicate : domain.get_predicates())
        EXPECT_TRUE(predicate_names.insert(std::string(predicate.get_name())).second) << "duplicate domain predicate name " << predicate.get_name();

    for (auto action : domain.get_actions())
    {
        const auto what = "action " + std::string(action.get_name());

        auto parameter_names = ygg::UnorderedSet<std::string> {};
        for (auto parameter : action.get_parameters())
            EXPECT_TRUE(parameter_names.insert(std::string(parameter.get_variable().get_name())).second) << what << ": duplicate parameter";

        if (const auto precondition = action.get_precondition())
            expect_flat_condition(precondition.value(), what + ": precondition");

        if (const auto effect = action.get_effect())
        {
            expect_effect_normal_form(effect.value(), true, what + ": effect");
            auto binder_names = ygg::UnorderedSet<std::string> {};
            collect_effect_binder_names(effect.value(), binder_names);
            for (const auto& name : binder_names)
                EXPECT_FALSE(parameter_names.contains(name)) << what << ": effect binder " << name << " shadows an action parameter";
        }
    }

    expect_axiom_invariants(domain.get_axioms(), "domain " + std::string(domain.get_name()));
    if (!domain.get_axioms().empty())
    {
        EXPECT_TRUE(has_requirement_kind(domain, formalism::RequirementKind::DerivedPredicates)) << domain.get_name();
    }
}

void expect_task_invariants(formalism::DomainView translated_domain, formalism::TaskView task)
{
    EXPECT_EQ(task.get_domain().get_name(), translated_domain.get_name());

    auto object_names = ygg::UnorderedSet<std::string> {};
    for (auto object : task.get_objects())
        EXPECT_TRUE(object_names.insert(std::string(object.get_name())).second) << "duplicate task object name " << object.get_name();
    auto domain_predicate_names = ygg::UnorderedSet<std::string> {};
    for (auto predicate : translated_domain.get_predicates())
        domain_predicate_names.insert(std::string(predicate.get_name()));

    auto task_predicate_names = ygg::UnorderedSet<std::string> {};
    for (auto predicate : task.get_predicates())
    {
        const auto name = std::string(predicate.get_name());
        EXPECT_TRUE(task_predicate_names.insert(name).second) << "duplicate task predicate name " << name;
        if (name.starts_with("loki-"))
        {
            EXPECT_FALSE(domain_predicate_names.contains(name)) << "task-generated predicate reuses domain name " << name;
        }
    }

    if (const auto goal = task.get_goal())
    {
        EXPECT_TRUE(is_conjunctive(goal.value())) << "task goal is not a conjunction of literals";
    }

    expect_axiom_invariants(task.get_axioms(), "task " + std::string(task.get_name()));
    if (!task.get_axioms().empty())
    {
        EXPECT_TRUE(has_requirement_kind(task.get_requirements(), formalism::RequirementKind::DerivedPredicates)) << task.get_name();
    }
}

}  // namespace

TEST(LokiSemanticInvariantSuite, TranslatorOptionCombinationsKeepInvariants)
{
    const auto cases = all_positive_cases();
    ASSERT_FALSE(cases.empty());

    auto checked = std::size_t { 0 };
    for (const auto& item : cases)
    {
        SCOPED_TRACE(item.name);
        ++checked;

        auto parser = semantic::Parser(item.domain_file);
        auto task = std::optional<formalism::TaskView> {};
        if (item.task_file)
            task = parser.parse_task(*item.task_file);

        for (const auto compile_typing : { false, true })
        {
            for (const auto compile_conditional_effects : { false, true })
            {
                for (const auto materialize_equality : { false, true })
                {
                    SCOPED_TRACE("compile_typing=" + std::to_string(compile_typing) + " compile_conditional_effects="
                                 + std::to_string(compile_conditional_effects) + " materialize_equality=" + std::to_string(materialize_equality));

                    auto options = semantic::TranslatorOptions {};
                    options.compile_typing = compile_typing;
                    options.compile_conditional_effects = compile_conditional_effects;
                    options.materialize_equality = materialize_equality;

                    const auto domain_translation = semantic::translate(parser.get_domain(), options);
                    const auto translated_domain = domain_translation.get_translated_domain();

                    expect_domain_invariants(translated_domain);

                    if (compile_typing)
                    {
                        EXPECT_TRUE(translated_domain.get_types().empty());
                        for (auto action : translated_domain.get_actions())
                            for (auto parameter : action.get_parameters())
                                EXPECT_TRUE(parameter.get_types().empty());
                    }
                    else
                    {
                        EXPECT_FALSE(translated_domain.get_types().empty());
                    }

                    if (compile_conditional_effects)
                    {
                        // Multiplied-out actions must expose plain conjunctive preconditions.
                        for (auto action : translated_domain.get_actions())
                            if (const auto precondition = action.get_precondition())
                            {
                                EXPECT_TRUE(is_conjunctive(precondition.value()));
                            }
                    }

                    if (!task)
                    {
                        const auto domain_text = formalism::format::to_string(translated_domain);
                        auto reparsed = semantic::Parser(domain_text);
                        EXPECT_EQ(reparsed.get_domain().get_name(), translated_domain.get_name());
                        continue;
                    }

                    const auto task_translation = semantic::translate(*task, domain_translation, options);
                    const auto translated_task = task_translation.get_translated_task();

                    EXPECT_EQ(&translated_task.get_domain().get_context().get_root(), &translated_domain.get_context().get_root());
                    expect_task_invariants(translated_domain, translated_task);

                    if (materialize_equality)
                    {
                        if (has_equality_predicate(translated_domain))
                        {
                            EXPECT_EQ(count_equality_literals(translated_task.get_initial_literals()),
                                      count_unique_object_names(translated_domain, translated_task));
                        }
                    }
                    else
                    {
                        EXPECT_EQ(count_equality_literals(translated_task.get_initial_literals()), 0);
                    }

                    // Formatted output must reparse for every option combination.
                    const auto domain_text = formalism::format::to_string(translated_domain);
                    const auto task_text = formalism::format::to_string(translated_task);
                    auto reparsed = semantic::Parser(domain_text);
                    const auto reparsed_task = reparsed.parse_task(task_text);
                    EXPECT_EQ(reparsed.get_domain().get_name(), translated_domain.get_name());
                    EXPECT_EQ(reparsed_task.get_name(), translated_task.get_name());
                }
            }
        }
    }
    EXPECT_GT(checked, 0) << "no positive case was available";
}

TEST(LokiSemanticInvariantSuite, ParsesAllCasesWithContiguousTopLevelIndices)
{
    const auto cases = all_positive_cases();
    ASSERT_FALSE(cases.empty());

    for (const auto& item : cases)
    {
        SCOPED_TRACE(item.name);

        semantic::Parser parser(item.domain_file);
        const auto domain = parser.get_domain();
        expect_contiguous_domain_indices(domain);
        if (item.task_file)
        {
            const auto task = parser.parse_task(*item.task_file);
            EXPECT_EQ(task.get_domain(), domain);
            expect_contiguous_task_indices(task);
        }
    }
}

TEST(LokiSemanticInvariantSuite, AddActionCostsOptionHoldsAcrossFixtures)
{
    const auto cases = all_positive_cases();
    ASSERT_FALSE(cases.empty());

    for (const auto& item : cases)
    {
        SCOPED_TRACE(item.name);

        auto baseline_options = semantic::ParserOptions {};
        baseline_options.add_action_costs = false;
        auto baseline_parser = semantic::Parser(item.domain_file, baseline_options);
        const auto baseline_domain = baseline_parser.get_domain();
        const auto declares_action_costs = has_requirement_kind(baseline_domain, formalism::RequirementKind::ActionCosts);
        const auto declares_numeric_fluents = has_requirement_kind(baseline_domain, formalism::RequirementKind::Fluents)
                                              || has_requirement_kind(baseline_domain, formalism::RequirementKind::NumericFluents);

        auto options = semantic::ParserOptions {};
        options.add_action_costs = true;
        auto parser = semantic::Parser(item.domain_file, options);
        const auto domain = parser.get_domain();

        // Three regimes: action-cost domains get missing artifacts completed; genuine numeric
        // domains are left untouched (absent metric means unit costs); plain domains get
        // action-costs, total-cost, and unit-cost effects injected.

        if (!declares_action_costs && declares_numeric_fluents)
        {
            EXPECT_FALSE(has_requirement_kind(domain, formalism::RequirementKind::ActionCosts));
            continue;
        }

        EXPECT_TRUE(has_function_named(domain, "total-cost"));
        if (!declares_action_costs)
        {
            for (auto action : domain.get_actions())
            {
                SCOPED_TRACE(std::string(action.get_name()));
                ASSERT_TRUE(action.get_effect().has_value());
                EXPECT_TRUE(writes_function_named(action.get_effect().value(), "total-cost"));
            }
        }

        if (!item.task_file)
            continue;
        const auto task = parser.parse_task(*item.task_file);

        ASSERT_TRUE(task.get_metric().has_value());
        EXPECT_EQ(task.get_metric().value().get_optimization_direction(), formalism::OptimizationDirection::Minimize);
        auto has_total_cost_init = false;
        for (auto value : task.get_initial_function_values())
            has_total_cost_init = has_total_cost_init || std::string_view(value.get_function().get_function().get_name()) == "total-cost";
        EXPECT_TRUE(has_total_cost_init);

        // The completed output must still reparse.
        auto reparsed = semantic::Parser(formalism::format::to_string(domain));
        const auto reparsed_task = reparsed.parse_task(formalism::format::to_string(task));
        EXPECT_EQ(reparsed_task.get_name(), task.get_name());
    }
}

}  // namespace loki::tests
