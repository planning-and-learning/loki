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

#include <gtest/gtest.h>
#include <loki/formalism/formatter.hpp>
#include <loki/semantic/options.hpp>
#include <loki/semantic/parser.hpp>
#include <string>
#include <type_traits>

namespace loki::tests
{

TEST(LokiSemanticActionCosts, AddsActionCostsDefaultsInPermissiveMode)
{
    auto parser = semantic::Parser(fixture_path("action-costs"));
    const auto parsed_task = parser.parse_task(fixture_path("action-costs", "task.pddl"));

    ASSERT_TRUE(parsed_task.get_metric().has_value());
    EXPECT_EQ(parsed_task.get_metric().value().get_optimization_direction(), formalism::OptimizationDirection::Minimize);
    ygg::visit(
        [](const auto expression)
        {
            using Expression = std::decay_t<decltype(expression)>;
            EXPECT_TRUE((std::is_same_v<Expression, formalism::FunctionTermView>) );
            if constexpr (std::is_same_v<Expression, formalism::FunctionTermView>)
            {
                EXPECT_EQ(expression.get_function().get_name(), "total-cost");
                EXPECT_EQ(expression.get_terms().size(), 0);
            }
        },
        parsed_task.get_metric().value().get_expression().get_variant());

    ASSERT_EQ(parsed_task.get_initial_function_values().size(), 1);
    const auto initial_value = parsed_task.get_initial_function_values()[0];
    EXPECT_EQ(initial_value.get_function().get_function().get_name(), "total-cost");
    EXPECT_EQ(initial_value.get_function().get_terms().size(), 0);
    ygg::visit(
        [](const auto expression)
        {
            using Expression = std::decay_t<decltype(expression)>;
            EXPECT_TRUE((std::is_same_v<Expression, formalism::EntityView<formalism::FunctionExpression<formalism::NumberTag>>>) );
            if constexpr (std::is_same_v<Expression, formalism::EntityView<formalism::FunctionExpression<formalism::NumberTag>>>)
            {
                EXPECT_EQ(expression.get_value(), 0.0);
            }
        },
        initial_value.get_value().get_variant());
}

TEST(LokiSemanticActionCosts, AddActionCostsOptionInjectsUnitCosts)
{
    auto options = semantic::ParserOptions {};
    options.add_action_costs = true;
    auto parser = semantic::Parser(fixture_path("unit-cost-injection"), options);

    const auto domain_text = formalism::format::to_string(parser.get_domain());
    EXPECT_NE(domain_text.find(":action-costs"), std::string::npos);
    EXPECT_NE(domain_text.find("(total-cost)"), std::string::npos);
    EXPECT_NE(domain_text.find("(increase (total-cost) 1)"), std::string::npos);

    const auto parsed_task = parser.parse_task(fixture_path("unit-cost-injection", "task.pddl"));
    ASSERT_TRUE(parsed_task.get_metric().has_value());
    EXPECT_EQ(parsed_task.get_metric().value().get_optimization_direction(), formalism::OptimizationDirection::Minimize);
    ASSERT_EQ(parsed_task.get_initial_function_values().size(), 1);
    EXPECT_EQ(parsed_task.get_initial_function_values()[0].get_function().get_function().get_name(), "total-cost");
}

TEST(LokiSemanticActionCosts, AddActionCostsOptionSkipsActionsAlreadyWritingTotalCost)
{
    auto options = semantic::ParserOptions {};
    options.add_action_costs = true;
    auto parser = semantic::Parser(fixture_path("mixed-costs"), options);

    const auto domain_text = formalism::format::to_string(parser.get_domain());
    const auto count = [&](const std::string& needle)
    {
        auto occurrences = 0;
        for (auto pos = domain_text.find(needle); pos != std::string::npos; pos = domain_text.find(needle, pos + 1))
            ++occurrences;
        return occurrences;
    };
    EXPECT_EQ(count("(increase (total-cost) 5)"), 1);
    // Only the cheap action gets a unit cost; the expensive one already writes total-cost.
    EXPECT_EQ(count("(increase (total-cost) 1)"), 1);
}

TEST(LokiSemanticActionCosts, AddActionCostsOptionCompletesInsteadOfStrictErrors)
{
    auto options = semantic::ParserOptions {};
    options.strict = true;
    options.add_action_costs = true;
    auto parser = semantic::Parser(fixture_path("action-costs"), options);
    const auto parsed_task = parser.parse_task(fixture_path("action-costs", "task.pddl"));

    ASSERT_TRUE(parsed_task.get_metric().has_value());
    ASSERT_EQ(parsed_task.get_initial_function_values().size(), 1);
}

TEST(LokiSemanticActionCosts, NumericFluentsTaskWithoutMetricKeepsMetricAbsent)
{
    // add_action_costs must not graft total-cost onto genuine numeric domains either.
    for (const auto strict : { false, true })
    {
        for (const auto add_action_costs : { false, true })
        {
            auto options = semantic::ParserOptions {};
            options.strict = strict;
            options.add_action_costs = add_action_costs;
            auto parser = semantic::Parser(fixture_path("numeric-fluents"), options);
            EXPECT_FALSE(has_function_named(parser.get_domain(), "total-cost"));
            EXPECT_FALSE(has_requirement_kind(parser.get_domain(), formalism::RequirementKind::ActionCosts));
            const auto parsed_task = parser.parse_task(fixture_path("numeric-fluents", "task.pddl"));
            EXPECT_FALSE(parsed_task.get_metric().has_value());
            ASSERT_EQ(parsed_task.get_initial_function_values().size(), 1);
            EXPECT_EQ(parsed_task.get_initial_function_values()[0].get_function().get_function().get_name(), "fuel");
        }
    }
}

}  // namespace loki::tests
