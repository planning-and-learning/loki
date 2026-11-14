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

#include "benchmark_utils.hpp"

#include <gtest/gtest.h>
#include <loki/loki.hpp>
#include <string>
#include <type_traits>

namespace loki::tests
{

TEST(LokiPublicFacade, ExposesParserAndTranslator)
{
    auto parser = loki::semantic::Parser(fixture_path("facade"), loki::semantic::ParserOptions { .strict = false, .add_action_costs = false });

    const auto domain = parser.get_domain();

    const auto task = parser.parse_task(fixture_path("facade", "task.pddl"));

    const auto domain_translation = loki::semantic::translate(domain);
    const auto task_translation = loki::semantic::translate(task, domain_translation);

    EXPECT_EQ(domain.get_requirements().size(), 1);
    EXPECT_EQ(domain.get_predicates().size(), 1);
    EXPECT_EQ(domain.get_actions().size(), 1);

    const auto action = domain.get_actions()[0];
    EXPECT_EQ(action.get_parameters().size(), 0);
    EXPECT_TRUE(action.get_precondition().has_value());
    EXPECT_TRUE(action.get_effect().has_value());

    EXPECT_EQ(task.get_initial_literals().size(), 1);
    EXPECT_TRUE(task.get_goal().has_value());
    EXPECT_FALSE(task.get_metric().has_value());

    EXPECT_EQ(domain_translation.get_original_domain().get_name(), domain.get_name());
    EXPECT_EQ(task_translation.get_original_task().get_name(), task.get_name());
    EXPECT_EQ(task_translation.get_translated_task().get_domain().get_name(), domain_translation.get_translated_domain().get_name());
}

TEST(LokiPublicFacade, ViewConvenienceMethodsCoverTypedSymbolsAndLiterals)
{
    auto parser = loki::semantic::Parser(fixture_path("typed_facade"));

    const auto domain = parser.get_domain();
    const auto task = parser.parse_task(fixture_path("typed_facade", "task.pddl"));

    ASSERT_GE(domain.get_types().size(), 2);
    auto saw_item_type = false;
    for (const auto type : domain.get_types())
    {
        if (type.get_name() == "item")
        {
            saw_item_type = true;
            EXPECT_EQ(type.get_bases().size(), 1);
            EXPECT_EQ(type.get_bases()[0].get_name(), "base");
        }
    }
    EXPECT_TRUE(saw_item_type);

    ASSERT_EQ(domain.get_predicates().size(), 1);
    const auto predicate = domain.get_predicates()[0];
    ASSERT_EQ(predicate.get_parameters().size(), 1);
    EXPECT_EQ(predicate.get_parameters()[0].get_types().size(), 1);

    ASSERT_EQ(domain.get_actions().size(), 1);
    const auto action = domain.get_actions()[0];
    ASSERT_EQ(action.get_parameters().size(), 1);
    EXPECT_EQ(action.get_parameters()[0].get_types().size(), 1);

    ASSERT_EQ(task.get_objects().size(), 1);
    EXPECT_EQ(task.get_objects()[0].get_types().size(), 1);

    ASSERT_EQ(task.get_initial_literals().size(), 1);
    const auto literal = task.get_initial_literals()[0];
    EXPECT_TRUE(literal.get_polarity());
    EXPECT_EQ(literal.get_atom().get_terms().size(), 1);
    static_cast<void>(action.get_precondition().value());
    static_cast<void>(action.get_effect().value());
}

TEST(LokiPublicFacade, ViewConvenienceMethodsCoverNumericEffectsAndMetrics)
{
    auto parser = loki::semantic::Parser(fixture_path("numeric_facade"));

    const auto domain = parser.get_domain();
    const auto task = parser.parse_task(fixture_path("numeric_facade", "task.pddl"));

    ASSERT_EQ(domain.get_functions().size(), 1);
    ASSERT_EQ(domain.get_actions().size(), 1);
    const auto effect = domain.get_actions()[0].get_effect();
    ASSERT_TRUE(effect.has_value());
    auto saw_numeric_effect = false;
    std::visit(
        [&](const auto effect_node)
        {
            using EffectHandle = std::decay_t<decltype(effect_node.index())>;
            if constexpr (std::is_same_v<EffectHandle, urd::index<loki::formalism::Effect<loki::formalism::AndTag>>>)
            {
                ASSERT_EQ(effect_node.get_effects().size(), 2);
                for (auto child : effect_node.get_effects())
                {
                    std::visit(
                        [&](const auto nested_effect_node)
                        {
                            using NestedEffectHandle = std::decay_t<decltype(nested_effect_node.index())>;
                            if constexpr (std::is_same_v<NestedEffectHandle, urd::index<loki::formalism::Effect<loki::formalism::NumericTag>>>)
                            {
                                saw_numeric_effect = true;
                                EXPECT_EQ(nested_effect_node.get_operator(), loki::formalism::NumericEffectOperator::Increase);
                                EXPECT_EQ(nested_effect_node.get_function().get_function().get_name(), "total-cost");
                                EXPECT_EQ(nested_effect_node.get_function().get_terms().size(), 0);

                                std::visit(
                                    [](const auto expression_node)
                                    {
                                        using ExpressionHandle = std::decay_t<decltype(expression_node.index())>;
                                        if constexpr (std::is_same_v<ExpressionHandle,
                                                                     urd::index<loki::formalism::FunctionExpression<loki::formalism::NumberTag>>>)
                                        {
                                            EXPECT_EQ(expression_node.get_value(), 1);
                                        }
                                        else
                                        {
                                            ADD_FAILURE() << "Expected numeric effect expression to be a number.";
                                        }
                                    },
                                    nested_effect_node.get_expression());
                            }
                        },
                        child);
                }
            }
            else
            {
                ADD_FAILURE() << "Expected action effect to be an effect conjunction.";
            }
        },
        effect.value());
    EXPECT_TRUE(saw_numeric_effect);

    ASSERT_TRUE(task.get_metric().has_value());
    const auto metric = task.get_metric().value();
    EXPECT_EQ(metric.get_optimization_direction(), loki::formalism::OptimizationDirection::Minimize);
    std::visit(
        [](const auto expression_node)
        {
            using ExpressionHandle = std::decay_t<decltype(expression_node.index())>;
            EXPECT_TRUE((std::is_same_v<ExpressionHandle, urd::index<loki::formalism::FunctionTerm>>) );
        },
        metric.get_expression());
}

TEST(LokiPublicFacade, FormatsSemanticDomainAndTaskAsReparseablePddl)
{
    auto parser = loki::semantic::Parser(fixture_path("facade_format"));

    const auto domain = parser.get_domain();
    const auto task = parser.parse_task(fixture_path("facade_format", "task.pddl"));

    const auto domain_text = loki::formalism::format::to_string(domain);
    const auto task_text = loki::formalism::format::to_string(task);

    auto reparsed = loki::semantic::Parser(domain_text);

    const auto reparsed_domain = reparsed.get_domain();
    const auto reparsed_task = reparsed.parse_task(task_text);

    EXPECT_EQ(reparsed_domain.get_name(), domain.get_name());
    EXPECT_EQ(reparsed_task.get_name(), task.get_name());
    EXPECT_EQ(reparsed_task.get_domain().get_name(), reparsed_domain.get_name());
}

TEST(LokiPublicFacade, FormatsAlternativeEffectsAsReparseablePddl)
{
    auto parser = loki::semantic::Parser(fixture_path("alternative_effects"), loki::semantic::ParserOptions { .strict = false, .add_action_costs = false });

    const auto domain = parser.get_domain();
    const auto domain_text = loki::formalism::format::to_string(domain);

    auto reparsed = loki::semantic::Parser(domain_text, loki::semantic::ParserOptions { .strict = false, .add_action_costs = false });
    const auto reparsed_domain = reparsed.get_domain();

    EXPECT_EQ(reparsed_domain.get_name(), domain.get_name());
    ASSERT_EQ(reparsed_domain.get_actions().size(), 2);
    ASSERT_TRUE(reparsed_domain.get_actions()[0].get_effect().has_value());
    ASSERT_TRUE(reparsed_domain.get_actions()[1].get_effect().has_value());
    std::visit(
        [](const auto effect_node)
        {
            using EffectHandle = std::decay_t<decltype(effect_node.index())>;
            EXPECT_TRUE((std::is_same_v<EffectHandle, urd::index<loki::formalism::Effect<loki::formalism::OneOfTag>>>) );
        },
        reparsed_domain.get_actions()[0].get_effect().value());
    std::visit(
        [](const auto effect_node)
        {
            using EffectHandle = std::decay_t<decltype(effect_node.index())>;
            EXPECT_TRUE((std::is_same_v<EffectHandle, urd::index<loki::formalism::Effect<loki::formalism::ProbabilisticTag>>>) );
        },
        reparsed_domain.get_actions()[1].get_effect().value());
}

TEST(LokiPublicFacade, FormatsTypedNumericTaskSectionsAsReparseablePddl)
{
    auto parser = loki::semantic::Parser(fixture_path("facade_format_numeric"), loki::semantic::ParserOptions { .strict = false, .add_action_costs = false });

    const auto domain = parser.get_domain();
    const auto task = parser.parse_task(fixture_path("facade_format_numeric", "task.pddl"));

    const auto domain_text = loki::formalism::format::to_string(domain);
    const auto task_text = loki::formalism::format::to_string(task);

    EXPECT_NE(domain_text.find("(:types item - object)"), std::string::npos);
    EXPECT_NE(domain_text.find("(:constants depot - item)"), std::string::npos);
    // number is the default function type and is left out.
    EXPECT_NE(domain_text.find("(cost ?x - item)"), std::string::npos);
    EXPECT_EQ(domain_text.find("- number"), std::string::npos);
    EXPECT_NE(domain_text.find("(increase (cost ?x) 1.5)"), std::string::npos);
    EXPECT_NE(task_text.find("(:objects package - item)"), std::string::npos);
    EXPECT_NE(task_text.find("(= (cost package) 0)"), std::string::npos);
    EXPECT_NE(task_text.find("(:metric minimize (cost package))"), std::string::npos);

    auto reparsed = loki::semantic::Parser(domain_text, loki::semantic::ParserOptions { .strict = false, .add_action_costs = false });
    const auto reparsed_domain = reparsed.get_domain();
    const auto reparsed_task = reparsed.parse_task(task_text);

    EXPECT_EQ(reparsed_domain.get_name(), domain.get_name());
    EXPECT_EQ(reparsed_domain.get_constants().size(), 1);
    ASSERT_EQ(reparsed_domain.get_functions().size(), 1);
    EXPECT_EQ(reparsed_domain.get_functions()[0].get_parameters().size(), 1);
    EXPECT_EQ(reparsed_domain.get_functions()[0].get_type().get_name(), "number");
    EXPECT_EQ(reparsed_task.get_objects().size(), 1);
    EXPECT_EQ(reparsed_task.get_initial_literals().size(), 1);
    EXPECT_EQ(reparsed_task.get_initial_function_values().size(), 1);
    ASSERT_TRUE(reparsed_task.get_metric().has_value());
    EXPECT_EQ(reparsed_task.get_metric().value().get_optimization_direction(), loki::formalism::OptimizationDirection::Minimize);
}

}  // namespace loki::tests
