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

// PPDDL's rewards and fractions, PDDL 2.1's bare function heads, free section order, and PDDL 3's constraints and preferences.

namespace loki::tests
{
namespace
{

constexpr auto permissive = semantic::ParserOptions { .add_action_costs = false };
constexpr auto strict = semantic::ParserOptions { .strict = true, .add_action_costs = false };

std::string domain_with(const std::string& requirements, const std::string& body)
{ return "(define (domain d) (:requirements :strips " + requirements + ") (:predicates (p) (q ?x)) " + body + ")"; }

}  // namespace

TEST(LokiSemanticLanguage, ParsesPpddlRewardsAndFractions)
{
    semantic::Parser parser(fixture_path("ppddl_rewards"), strict);
    const auto domain = parser.get_domain();
    ASSERT_EQ(domain.actions().size(), 1);
    // :parameters is optional; 1/3 is a probability; (reward) is declared by :rewards.
    EXPECT_EQ(fmt::format("{}", domain.actions()[0].effect().value()),
              "(and (decrease (reward) 1) (probabilistic 0.3333333333333333 (p) 0.6666666666666666 (q)))");
    const auto task = parser.parse_task(fixture_path("ppddl_rewards", "problem.pddl"));
    ASSERT_TRUE(task.goal_reward().has_value());
    EXPECT_EQ(task.goal_reward().value(), 500.0);
    const auto translation = semantic::translate(parser.get_domain());
    const auto task_translation = semantic::translate(task, translation);
    // The strict parser reads the translation back.
    semantic::Parser reparsed(fmt::format("{}", translation.get_translated_domain()), strict);
    EXPECT_EQ(reparsed.parse_task(fmt::format("{}", task_translation.get_translated_task())).goal_reward().value(), 500.0);
}

TEST(LokiSemanticLanguage, RejectsAZeroDenominator)
{
    EXPECT_THROW(semantic::Parser(domain_with(":probabilistic-effects", "(:action a :effect (probabilistic 1/0 (p)))"), permissive),
                 semantic::InvalidProbabilisticEffectError);
}

TEST(LokiSemanticLanguage, WritesABareZeroAryFunction)
{
    semantic::Parser parser(domain_with(":numeric-fluents", "(:functions (fuel)) (:action a :parameters () :effect (increase fuel 2))"), strict);
    EXPECT_EQ(fmt::format("{}", parser.get_domain().actions()[0].effect().value()), "(increase (fuel) 2)");
}

TEST(LokiSemanticLanguage, ReadsSectionsInAnyOrder)
{
    semantic::Parser parser(std::string(R"((define (domain d) (:requirements :strips :typing :numeric-fluents)
  (:functions (f ?x - t)) (:types t) (:predicates (p ?x - t)) (:constants c - t)
  (:action a :parameters () :precondition (p c) :effect (increase (f c) 1))))"),
                            strict);
    EXPECT_EQ(parser.get_domain().constants().size(), 1);
    EXPECT_EQ(parser.get_domain().functions().size(), 1);
}

TEST(LokiSemanticLanguage, ParsesAndTranslatesConstraintsAndPreferences)
{
    semantic::Parser parser(fixture_path("pddl3_basic"), strict);
    const auto domain = parser.get_domain();
    ASSERT_TRUE(domain.constraints().has_value());
    const auto task = parser.parse_task(fixture_path("pddl3_basic", "problem.pddl"));
    ASSERT_TRUE(task.constraints().has_value());
    EXPECT_EQ(fmt::format("{}", task.constraints().value()), "(and (always-within 2 (held a) (clear table)) (hold-during 1 3 (clear b)) (within 5 (held a)))");

    const auto translation = semantic::translate(domain, semantic::TranslatorOptions { .compile_typing = false });
    const auto translated = translation.get_translated_domain();
    // The disjunction of the preference is a derived predicate, not a split: one preference, one count.
    ASSERT_EQ(translated.actions().size(), 1);
    EXPECT_NE(fmt::format("{}", translated.actions()[0].precondition().value()).find("(preference careful (loki-preference-"), std::string::npos);
    // Keeping typing, the universal constraint's typed parameter restricts it: no guard (CompileTyping guards it).
    EXPECT_NE(fmt::format("{}", translated.constraints().value()).find("(forall (?x - block) (always (loki-constraint-"), std::string::npos);

    const auto task_translation = semantic::translate(task, translation, semantic::TranslatorOptions { .compile_typing = false });
    const auto goal = fmt::format("{}", task_translation.get_translated_task().goal().value());
    EXPECT_NE(goal.find("(forall (?x - block) (preference all-held "), std::string::npos);
    EXPECT_EQ(goal.find("loki-typed-"), std::string::npos);
    EXPECT_NE(goal.find("(preference hold-b (held b))"), std::string::npos);

    // The strict parser reads the translation back.
    semantic::Parser reparsed(fmt::format("{}", translated), strict);
    const auto reparsed_task = reparsed.parse_task(fmt::format("{}", task_translation.get_translated_task()));
    EXPECT_TRUE(reparsed_task.constraints().has_value());
}

TEST(LokiSemanticLanguage, RejectsMalformedConstraintsAndPreferences)
{
    const auto pddl3 = std::string(":preferences :constraints :disjunctive-preconditions :negative-preconditions");
    // A modal operator's arity, a nested preference, a preference below a disjunction or in an effect's condition.
    EXPECT_THROW(semantic::Parser(domain_with(pddl3, "(:constraints (within (p)))"), permissive), semantic::InvalidConstraintError);
    EXPECT_THROW(semantic::Parser(domain_with(pddl3, "(:constraints (hold-during 1 (p)))"), permissive), semantic::InvalidConstraintError);
    EXPECT_THROW(semantic::Parser(domain_with(pddl3, "(:action a :parameters () :precondition (preference x (preference y (p))))"), permissive),
                 semantic::InvalidConstraintError);
    EXPECT_THROW(semantic::Parser(domain_with(pddl3, "(:action a :parameters () :precondition (or (p) (preference x (p))))"), permissive),
                 semantic::InvalidConstraintError);
    EXPECT_THROW(semantic::Parser(domain_with(pddl3 + " :conditional-effects", "(:action a :parameters () :effect (when (preference x (p)) (p)))"), permissive),
                 semantic::InvalidConstraintError);
    // is-violated reads a declared preference.
    semantic::Parser parser(domain_with(pddl3 + " :numeric-fluents", "(:action a :parameters () :precondition (preference x (p)))"), permissive);
    EXPECT_THROW(parser.parse_task(std::string("(define (problem t) (:domain d) (:init) (:goal (p)) (:metric minimize (is-violated y)))")),
                 semantic::InvalidConstraintError);
    EXPECT_NO_THROW(parser.parse_task(std::string("(define (problem t) (:domain d) (:init) (:goal (p)) (:metric minimize (is-violated x)))")));
}

TEST(LokiSemanticLanguage, ParsesAndTranslatesTrajectoryFormulas)
{
    semantic::Parser parser(fixture_path("trajectory_formulas"), strict);
    const auto domain = parser.get_domain();
    ASSERT_TRUE(domain.constraints().has_value());
    const auto constraints = fmt::format("{}", domain.constraints().value());
    // Nesting, metric intervals (an upper bound inf), past operators; imply as (or (not a) b); a goal description's not
    // stays a goal description.
    EXPECT_NE(constraints.find("(always (sometime (door-open)))"), std::string::npos);
    EXPECT_NE(constraints.find("(until (interval 0 inf) (not (visited r2)) (visited r1))"), std::string::npos);
    EXPECT_NE(constraints.find("(historically (interval 0 10) (or (door-open) (weak-yesterday (not (door-open)))))"), std::string::npos);
    EXPECT_NE(constraints.find("(or (not (visited ?r)) (once (at ?r)))"), std::string::npos);
    EXPECT_NE(constraints.find("(not (sometime (and (door-open) (next (not (door-open))))))"), std::string::npos);

    // A goal over the trajectory is a hard constraint of the task, conjoined with its own; (next r1 r2) is an atom.
    const auto task = parser.parse_task(fixture_path("trajectory_formulas", "problem.pddl"));
    EXPECT_FALSE(task.goal().has_value());
    ASSERT_TRUE(task.constraints().has_value());
    EXPECT_EQ(fmt::format("{}", task.constraints().value()),
              "(and (at end (at r3)) (next r1 r2) (once (interval 1 3) (door-open)) (preference fast (within 5 (visited r3))) (sometime (visited r2)))");

    for (const auto compile_typing : { false, true })
    {
        SCOPED_TRACE(compile_typing);
        const auto options = semantic::TranslatorOptions { .compile_typing = compile_typing };
        const auto translation = semantic::translate(domain, options);
        const auto translated = fmt::format("{}", translation.get_translated_domain().constraints().value());
        // Negation normal form: through the duals, into the goal descriptions.
        EXPECT_NE(translated.find("(always (or (not (door-open)) (weak-next (door-open))))"), std::string::npos);
        // Compiling typing guards the universal formula by a derived predicate, the existential one by the type conditions.
        EXPECT_EQ(translated.find("(or (loki-typed-") != std::string::npos, compile_typing);
        EXPECT_EQ(translated.find("(exists (?r) (and (and (object ?r) (room ?r))") != std::string::npos, compile_typing);

        // The strict parser reads the translation back, and translating it again changes nothing.
        const auto task_translation = semantic::translate(task, translation, options);
        const auto domain_text = fmt::format("{}", translation.get_translated_domain());
        const auto task_text = fmt::format("{}", task_translation.get_translated_task());
        semantic::Parser reparsed(domain_text, strict);
        const auto reparsed_task = reparsed.parse_task(task_text);
        const auto retranslation = semantic::translate(reparsed.get_domain(), options);
        EXPECT_EQ(fmt::format("{}", retranslation.get_translated_domain()), domain_text);
        EXPECT_EQ(fmt::format("{}", semantic::translate(reparsed_task, retranslation, options).get_translated_task()), task_text);
    }
}

TEST(LokiSemanticLanguage, NegatesTrajectoryFormulasThroughTheirDuals)
{
    const auto negated = [](const std::string& formula)
    {
        semantic::Parser parser(domain_with(":negative-preconditions :trajectory-formulas", "(:constraints (not " + formula + "))"), permissive);
        const auto translation = semantic::translate(parser.get_domain());
        return fmt::format("{}", translation.get_translated_domain().constraints().value());
    };
    EXPECT_EQ(negated("(next (p))"), "(weak-next (not (p)))");
    EXPECT_EQ(negated("(weak-yesterday (p))"), "(yesterday (not (p)))");
    EXPECT_EQ(negated("(until (interval 1 2) (p) (next (p)))"), "(release (interval 1 2) (not (p)) (weak-next (not (p))))");
    EXPECT_EQ(negated("(since (p) (yesterday (p)))"), "(triggers (not (p)) (weak-yesterday (not (p))))");
    EXPECT_EQ(negated("(once (p))"), "(historically (not (p)))");
    EXPECT_EQ(negated("(always (p))"), "(sometime (not (p)))");
    EXPECT_EQ(negated("(at end (p))"), "(at end (not (p)))");
    // PDDL 3's other operators have no dual among the operators: the not stays.
    EXPECT_EQ(negated("(at-most-once (p))"), "(not (at-most-once (p)))");
}

TEST(LokiSemanticLanguage, RejectsMalformedTrajectoryFormulas)
{
    const auto extension = std::string(":trajectory-formulas :preferences");
    const auto rejects = [&](const std::string& constraints)
    { return semantic::Parser(domain_with(extension, "(:constraints " + constraints + ")"), permissive); };
    // Operands, intervals, and preferences only under and and forall.
    EXPECT_THROW(rejects("(until (p))"), semantic::InvalidConstraintError);
    EXPECT_THROW(rejects("(next (interval 0 1) (p))"), semantic::InvalidConstraintError);
    EXPECT_THROW(rejects("(once (interval 3 1) (p))"), semantic::InvalidConstraintError);
    EXPECT_THROW(rejects("(once (interval -1 1) (p))"), semantic::InvalidConstraintError);
    EXPECT_THROW(rejects("(always (preference x (sometime (p))))"), semantic::InvalidConstraintError);
    EXPECT_THROW(rejects("(not (preference x (sometime (p))))"), semantic::InvalidConstraintError);

    // Beyond PDDL 3 only with :trajectory-formulas, in strict mode: the new operators, nesting, a bare goal description.
    const auto pddl3 = std::string(":constraints");
    for (const auto* constraints : { "(next (p))", "(always (sometime (p)))", "(p)", "(not (always (p)))" })
    {
        SCOPED_TRACE(constraints);
        EXPECT_THROW(semantic::Parser(domain_with(pddl3, std::string("(:constraints ") + constraints + ")"), strict), semantic::MissingRequirementError);
    }
    EXPECT_NO_THROW(semantic::Parser(domain_with(pddl3, "(:constraints (always (p)))"), strict));
}

}  // namespace loki::tests
