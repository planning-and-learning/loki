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

TEST(LokiPublicFacade, ExposesParserAndTranslatorThroughLokiNamespace)
{
    auto parser = loki::Parser(fixture_path("facade"), loki::ParserOptions { .strict = false, .add_action_costs = false });

    const auto domain = parser.get_domain();

    const auto task = parser.parse_task(fixture_path("facade", "task.pddl"));

    const auto domain_translation = loki::translate(domain);
    const auto task_translation = loki::translate(task, domain_translation);
    const auto named_domain_translation = loki::translate_domain(domain);
    const auto named_task_translation = loki::translate_task(task, named_domain_translation);

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
    EXPECT_EQ(named_domain_translation.get_original_domain().get_name(), domain.get_name());
    EXPECT_EQ(named_task_translation.get_original_task().get_name(), task.get_name());
    EXPECT_EQ(named_task_translation.get_translated_task().get_domain().get_name(), named_domain_translation.get_translated_domain().get_name());
}

TEST(LokiPublicFacade, ExposesSemanticErrorsThroughLokiNamespace)
{
    static_assert(std::is_same_v<loki::SemanticError, loki::semantic::SemanticError>);
    static_assert(std::is_same_v<loki::ParseError, loki::semantic::ParseError>);
    static_assert(std::is_same_v<loki::UnsupportedRequirementError, loki::semantic::UnsupportedRequirementError>);
    static_assert(std::is_same_v<loki::MissingRequirementError, loki::semantic::MissingRequirementError>);
    static_assert(std::is_same_v<loki::UnusedRequirementError, loki::semantic::UnusedRequirementError>);
    static_assert(std::is_same_v<loki::UndefinedTypeError, loki::semantic::UndefinedTypeError>);
    static_assert(std::is_same_v<loki::UndefinedPredicateError, loki::semantic::UndefinedPredicateError>);
    static_assert(std::is_same_v<loki::UndefinedObjectError, loki::semantic::UndefinedObjectError>);
    static_assert(std::is_same_v<loki::UndefinedVariableError, loki::semantic::UndefinedVariableError>);
    static_assert(std::is_same_v<loki::UndefinedFunctionError, loki::semantic::UndefinedFunctionError>);
    static_assert(std::is_same_v<loki::DuplicateDefinitionError, loki::semantic::DuplicateDefinitionError>);
    static_assert(std::is_same_v<loki::DuplicateTypeError, loki::semantic::DuplicateTypeError>);
    static_assert(std::is_same_v<loki::DuplicatePredicateError, loki::semantic::DuplicatePredicateError>);
    static_assert(std::is_same_v<loki::DuplicateObjectError, loki::semantic::DuplicateObjectError>);
    static_assert(std::is_same_v<loki::DuplicateVariableError, loki::semantic::DuplicateVariableError>);
    static_assert(std::is_same_v<loki::DuplicateFunctionError, loki::semantic::DuplicateFunctionError>);
    static_assert(std::is_same_v<loki::ArityMismatchError, loki::semantic::ArityMismatchError>);
    static_assert(std::is_same_v<loki::TypeMismatchError, loki::semantic::TypeMismatchError>);
    static_assert(std::is_same_v<loki::InvalidMetricError, loki::semantic::InvalidMetricError>);
    static_assert(std::is_same_v<loki::InvalidNumericConstraintError, loki::semantic::InvalidNumericConstraintError>);
    static_assert(std::is_same_v<loki::InvalidNumericEffectError, loki::semantic::InvalidNumericEffectError>);
    static_assert(std::is_same_v<loki::InvalidProbabilisticEffectError, loki::semantic::InvalidProbabilisticEffectError>);
    static_assert(std::is_same_v<loki::InvalidEqualityError, loki::semantic::InvalidEqualityError>);
    static_assert(std::is_same_v<loki::MismatchedDomainError, loki::semantic::MismatchedDomainError>);
    static_assert(std::is_same_v<loki::MissingDomainError, loki::semantic::MissingDomainError>);

    auto error = loki::SemanticError("base diagnostic");
    EXPECT_EQ(std::string(error.what()), "base diagnostic");
    error.set_display_message("rendered diagnostic");
    EXPECT_EQ(std::string(error.what()), "rendered diagnostic");
}

TEST(LokiPublicFacade, ViewConvenienceMethodsCoverTypedSymbolsAndLiterals)
{
    auto parser = loki::Parser(fixture_path("typed-facade"));

    const auto domain = parser.get_domain();
    const auto task = parser.parse_task(fixture_path("typed-facade", "task.pddl"));

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
    static_cast<void>(action.get_precondition().value().get_variant());
    static_cast<void>(action.get_effect().value().get_variant());
    static_cast<void>(literal.get_atom().get_terms()[0].get_variant());
}

TEST(LokiPublicFacade, ViewConvenienceMethodsCoverNumericEffectsAndMetrics)
{
    auto parser = loki::Parser(fixture_path("numeric-facade"));

    const auto domain = parser.get_domain();
    const auto task = parser.parse_task(fixture_path("numeric-facade", "task.pddl"));

    ASSERT_EQ(domain.get_functions().size(), 1);
    ASSERT_EQ(domain.get_actions().size(), 1);
    const auto effect = domain.get_actions()[0].get_effect();
    ASSERT_TRUE(effect.has_value());
    auto saw_numeric_effect = false;
    ygg::visit(
        [&](const auto effect_node)
        {
            using EffectHandle = std::decay_t<decltype(effect_node.get_index())>;
            if constexpr (std::is_same_v<EffectHandle, ygg::Index<loki::formalism::Effect<loki::formalism::AndTag>>>)
            {
                ASSERT_EQ(effect_node.get_effects().size(), 2);
                for (auto child : effect_node.get_effects())
                {
                    ygg::visit(
                        [&](const auto nested_effect_node)
                        {
                            using NestedEffectHandle = std::decay_t<decltype(nested_effect_node.get_index())>;
                            if constexpr (std::is_same_v<NestedEffectHandle, ygg::Index<loki::formalism::Effect<loki::formalism::NumericTag>>>)
                            {
                                saw_numeric_effect = true;
                                EXPECT_EQ(nested_effect_node.get_operator(), loki::formalism::NumericEffectOperator::Increase);
                                EXPECT_EQ(nested_effect_node.get_function().get_function().get_name(), "total-cost");
                                EXPECT_EQ(nested_effect_node.get_function().get_terms().size(), 0);

                                ygg::visit(
                                    [](const auto expression_node)
                                    {
                                        using ExpressionHandle = std::decay_t<decltype(expression_node.get_index())>;
                                        if constexpr (std::is_same_v<ExpressionHandle, ygg::Index<loki::formalism::FunctionExpression<loki::formalism::NumberTag>>>)
                                        {
                                            EXPECT_EQ(expression_node.get_value(), 1);
                                        }
                                        else
                                        {
                                            ADD_FAILURE() << "Expected numeric effect expression to be a number.";
                                        }
                                    },
                                    nested_effect_node.get_expression().get_variant());
                            }
                        },
                        child.get_variant());
                }
            }
            else
            {
                ADD_FAILURE() << "Expected action effect to be an effect conjunction.";
            }
        },
        effect.value().get_variant());
    EXPECT_TRUE(saw_numeric_effect);

    ASSERT_TRUE(task.get_metric().has_value());
    const auto metric = task.get_metric().value();
    EXPECT_EQ(metric.get_optimization_direction(), loki::formalism::OptimizationDirection::Minimize);
    ygg::visit(
        [](const auto expression_node)
        {
            using ExpressionHandle = std::decay_t<decltype(expression_node.get_index())>;
            EXPECT_TRUE((std::is_same_v<ExpressionHandle, ygg::Index<loki::formalism::FunctionTerm>>) );
        },
        metric.get_expression().get_variant());
}

TEST(LokiPublicFacade, FormatsSemanticDomainAndTaskAsReparseablePddl)
{
    auto parser = loki::Parser(fixture_path("facade-format"));

    const auto domain = parser.get_domain();
    const auto task = parser.parse_task(fixture_path("facade-format", "task.pddl"));

    const auto domain_text = loki::format_domain(domain);
    const auto task_text = loki::format_task(task);

    auto reparsed = loki::Parser(domain_text);

    const auto reparsed_domain = reparsed.get_domain();
    const auto reparsed_task = reparsed.parse_task(task_text);

    EXPECT_EQ(reparsed_domain.get_name(), domain.get_name());
    EXPECT_EQ(reparsed_task.get_name(), task.get_name());
    EXPECT_EQ(reparsed_task.get_domain().get_name(), reparsed_domain.get_name());
}

TEST(LokiPublicFacade, FormatsAlternativeEffectsAsReparseablePddl)
{
    auto parser = loki::Parser(fixture_path("alternative-effects"), loki::ParserOptions { .strict = false, .add_action_costs = false });

    const auto domain = parser.get_domain();
    const auto domain_text = loki::format_domain(domain);

    auto reparsed = loki::Parser(domain_text, loki::ParserOptions { .strict = false, .add_action_costs = false });
    const auto reparsed_domain = reparsed.get_domain();

    EXPECT_EQ(reparsed_domain.get_name(), domain.get_name());
    ASSERT_EQ(reparsed_domain.get_actions().size(), 2);
    ASSERT_TRUE(reparsed_domain.get_actions()[0].get_effect().has_value());
    ASSERT_TRUE(reparsed_domain.get_actions()[1].get_effect().has_value());
    ygg::visit(
        [](const auto effect_node)
        {
            using EffectHandle = std::decay_t<decltype(effect_node.get_index())>;
            EXPECT_TRUE((std::is_same_v<EffectHandle, ygg::Index<loki::formalism::Effect<loki::formalism::OneOfTag>>>) );
        },
        reparsed_domain.get_actions()[0].get_effect().value().get_variant());
    ygg::visit(
        [](const auto effect_node)
        {
            using EffectHandle = std::decay_t<decltype(effect_node.get_index())>;
            EXPECT_TRUE((std::is_same_v<EffectHandle, ygg::Index<loki::formalism::Effect<loki::formalism::ProbabilisticTag>>>) );
        },
        reparsed_domain.get_actions()[1].get_effect().value().get_variant());
}

TEST(LokiPublicFacade, FormatsTypedNumericTaskSectionsAsReparseablePddl)
{
    auto parser = loki::Parser(fixture_path("facade-format-numeric"), loki::ParserOptions { .strict = false, .add_action_costs = false });

    const auto domain = parser.get_domain();
    const auto task = parser.parse_task(fixture_path("facade-format-numeric", "task.pddl"));

    const auto domain_text = loki::format_domain(domain);
    const auto task_text = loki::format_task(task);

    EXPECT_NE(domain_text.find("(:types item - object)"), std::string::npos);
    EXPECT_NE(domain_text.find("(:constants depot - item)"), std::string::npos);
    EXPECT_NE(domain_text.find("(cost ?x - item) - number"), std::string::npos);
    EXPECT_NE(domain_text.find("(increase (cost ?x) 1.5)"), std::string::npos);
    EXPECT_NE(task_text.find("(:objects package - item)"), std::string::npos);
    EXPECT_NE(task_text.find("(= (cost package) 0)"), std::string::npos);
    EXPECT_NE(task_text.find("(:metric minimize (cost package))"), std::string::npos);

    auto reparsed = loki::Parser(domain_text, loki::ParserOptions { .strict = false, .add_action_costs = false });
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
