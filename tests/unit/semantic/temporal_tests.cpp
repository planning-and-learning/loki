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

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <loki/semantic/errors.hpp>
#include <loki/semantic/options.hpp>
#include <loki/semantic/parser.hpp>
#include <loki/semantic/translator.hpp>
#include <string>
#include <yggdrasil/formatting/cista_formatters.hpp>

namespace loki::tests
{
namespace
{

constexpr auto parser_options = semantic::ParserOptions { .add_action_costs = false };

/// A domain with one durative action of the given parts.
std::string durative_domain(const std::string& duration, const std::string& condition, const std::string& effect)
{
    return R"((define (domain d)
  (:requirements :strips :durative-actions :duration-inequalities :numeric-fluents :negative-preconditions :conditional-effects
                 :universal-preconditions :disjunctive-preconditions)
  (:predicates (p) (q) (r ?x) (at ?x ?y))
  (:functions (f))
  (:durative-action a :parameters (?x) :duration )"
           + duration + " :condition " + condition + " :effect " + effect + "))";
}

}  // namespace

TEST(LokiSemanticTemporal, GroupsConditionsAndEffectsPerTimePoint)
{
    semantic::Parser parser(durative_domain("(= ?duration 2)",
                                            "(and (at start (p)) (over all (q)) (at start (r ?x)) (forall (?y) (at end (r ?y))))",
                                            "(and (at start (not (p))) (at end (p)) (at end (increase (f) ?duration)))"),
                            parser_options);
    const auto action = parser.get_domain().durative_actions()[0];
    EXPECT_EQ(fmt::format("{}", action.duration().value()), "(= ?duration 2)");
    EXPECT_EQ(fmt::format("{}", action.condition().value()),
              "(and (at end (forall (?y - object) (r ?y))) (at start (and (p) (r ?x))) (over all (q)))");
    EXPECT_EQ(fmt::format("{}", action.effect().value()), "(and (and (at end (increase (f) ?duration)) (at end (p))) (at start (not (p))))");
}

TEST(LokiSemanticTemporal, ReadsAConditionAtTheEffectsTimeUntimed)
{
    semantic::Parser parser(durative_domain("(= ?duration 1)", "()", "(and (when (at end (p)) (at end (q))) (when (at start (p)) (at end (q))))"),
                            parser_options);
    const auto action = parser.get_domain().durative_actions()[0];
    EXPECT_FALSE(action.condition().has_value());
    // Printed in PDDL 2.1's form: the condition timed again.
    EXPECT_EQ(fmt::format("{}", action.effect().value()), "(and (when (at start (p)) (at end (q))) (when (at end (p)) (at end (q))))");
}

TEST(LokiSemanticTemporal, DistinguishesThePredicateAtFromTimeSpecifiers)
{
    semantic::Parser parser(durative_domain("(= ?duration 1)", "(at start (at ?x ?x))", "(at end (at ?x ?x))"), parser_options);
    const auto action = parser.get_domain().durative_actions()[0];
    EXPECT_EQ(fmt::format("{}", action.condition().value()), "(at start (at ?x ?x))");
    EXPECT_EQ(fmt::format("{}", action.effect().value()), "(at end (at ?x ?x))");
}

TEST(LokiSemanticTemporal, TranslatesNonConjunctiveTimedConditionsToDerivedPredicates)
{
    semantic::Parser parser(durative_domain("(= ?duration 1)", "(and (over all (or (p) (q))) (at start (p)))", "(at end (p))"), parser_options);
    const auto translation = semantic::translate(parser.get_domain(), semantic::TranslatorOptions { .compile_typing = false });
    const auto domain = translation.get_translated_domain();
    const auto action = domain.durative_actions()[0];
    EXPECT_EQ(fmt::format("{}", action.condition().value()), "(and (at start (and (object ?x) (p))) (over all (loki-timed-0)))");
    EXPECT_EQ(domain.axioms().size(), 2);
}

TEST(LokiSemanticTemporal, ParsesTimedInitialLiterals)
{
    semantic::Parser parser(durative_domain("(= ?duration 1)", "()", "(at end (p))"), parser_options);
    const auto task = parser.parse_task(std::string(R"((define (problem t) (:domain d) (:requirements :timed-initial-literals) (:objects o)
  (:init (at o o) (at 1.5 (p)) (at 3 (not (q)))) (:goal (p))))"));
    EXPECT_EQ(task.initial_literals().size(), 1);
    ASSERT_EQ(task.timed_initial_literals().size(), 2);
    const auto text = fmt::format("{}", task);
    EXPECT_NE(text.find("(at 1.5 (p))"), std::string::npos);
    EXPECT_NE(text.find("(at 3 (not (q)))"), std::string::npos);
}

TEST(LokiSemanticTemporal, RejectsTemporalExpressionsOutsideDurativeActions)
{
    const auto domain = [](const std::string& action)
    { return "(define (domain d) (:requirements :strips :durative-actions :numeric-fluents) (:predicates (p)) (:functions (f)) " + action + ")"; };
    EXPECT_THROW(semantic::Parser(domain("(:action a :parameters () :precondition (at start (p)))"), parser_options), semantic::InvalidTemporalExpressionError);
    EXPECT_THROW(semantic::Parser(domain("(:action a :parameters () :effect (at end (p)))"), parser_options), semantic::InvalidTemporalExpressionError);
    EXPECT_THROW(semantic::Parser(domain("(:action a :parameters () :effect (increase (f) ?duration))"), parser_options),
                 semantic::InvalidTemporalExpressionError);
}

TEST(LokiSemanticTemporal, RejectsMalformedDurativeActions)
{
    // An untimed condition, a timed condition inside another, an untimed effect, and an over all duration constraint.
    EXPECT_THROW(semantic::Parser(durative_domain("(= ?duration 1)", "(p)", "(at end (p))"), parser_options), semantic::InvalidTemporalExpressionError);
    EXPECT_THROW(semantic::Parser(durative_domain("(= ?duration 1)", "(at start (at end (p)))", "(at end (p))"), parser_options),
                 semantic::InvalidTemporalExpressionError);
    EXPECT_THROW(semantic::Parser(durative_domain("(= ?duration 1)", "()", "(p)"), parser_options), semantic::InvalidTemporalExpressionError);
    EXPECT_THROW(semantic::Parser(durative_domain("(over all (= ?duration 1))", "()", "(at end (p))"), parser_options),
                 semantic::InvalidTemporalExpressionError);
}

TEST(LokiSemanticTemporal, RequiresTemporalRequirementsInStrictMode)
{
    const auto strict = semantic::ParserOptions { .strict = true, .add_action_costs = false };
    EXPECT_THROW(semantic::Parser(std::string(R"((define (domain d) (:requirements :strips) (:predicates (p))
  (:durative-action a :parameters () :duration (= ?duration 1) :effect (at end (p)))))"),
                                  strict),
                 semantic::MissingRequirementError);
    EXPECT_THROW(semantic::Parser(std::string(R"((define (domain d) (:requirements :strips :durative-actions) (:predicates (p))
  (:durative-action a :parameters () :duration (<= ?duration 1) :effect (at end (p)))))"),
                                  strict),
                 semantic::MissingRequirementError);
    EXPECT_NO_THROW(semantic::Parser(std::string(R"((define (domain d) (:requirements :strips :duration-inequalities) (:predicates (p))
  (:durative-action a :parameters () :duration (<= ?duration 1) :effect (at end (p)))))"),
                                     strict));
}

TEST(LokiSemanticTemporal, ReparsesTheTranslation)
{
    semantic::Parser parser(fixture_path("temporal_basic"), semantic::ParserOptions { .strict = true, .add_action_costs = false });
    const auto task = parser.parse_task(fixture_path("temporal_basic", "problem.pddl"));
    const auto domain_translation = semantic::translate(parser.get_domain());
    const auto task_translation = semantic::translate(task, domain_translation);
    const auto domain_text = fmt::format("{}", domain_translation.get_translated_domain());
    const auto task_text = fmt::format("{}", task_translation.get_translated_task());
    semantic::Parser reparsed(domain_text, semantic::ParserOptions { .strict = true, .add_action_costs = false });
    const auto reparsed_task = reparsed.parse_task(task_text);
    EXPECT_EQ(reparsed.get_domain().durative_actions().size(), 2);
    EXPECT_EQ(reparsed_task.timed_initial_literals().size(), 2);
}

}  // namespace loki::tests
