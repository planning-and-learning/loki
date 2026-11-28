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

#include <cstddef>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <loki/formalism/repository.hpp>
#include <loki/formalism/views.hpp>
#include <loki/semantic/errors.hpp>
#include <loki/semantic/options.hpp>
#include <loki/semantic/parser.hpp>
#include <loki/semantic/translator.hpp>
#include <map>
#include <set>
#include <string>
#include <type_traits>
#include <urd/repr/maps/unordered_flat_map.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>
#include <yggdrasil/formatting/cista_formatters.hpp>

namespace loki::tests
{

TEST(LokiSemanticParser, AllowsDuplicateActionNamesAndInternsIdenticalBodies)
{
    auto parser = semantic::Parser(fixture_path("duplicate_action_name"));

    EXPECT_EQ(parser.get_domain().actions().size(), 2);
}

TEST(LokiSemanticParser, LowercasesSourceBeforeParsing)
{
    auto options = semantic::ParserOptions {};
    options.add_action_costs = false;
    auto parser = semantic::Parser(ygg::common::read_file(fixture_path("mixed_case")), options);

    EXPECT_EQ(parser.get_domain().name(), "mixed-domain");
    ASSERT_EQ(parser.get_domain().predicates().size(), 1);
    EXPECT_EQ(parser.get_domain().predicates()[0].name(), "ready");

    const auto task = parser.parse_task(ygg::common::read_file(fixture_path("mixed_case", "task.pddl")));
    EXPECT_EQ(task.name(), "mixed-task");
    ASSERT_EQ(task.objects().size(), 1);
    EXPECT_EQ(task.objects()[0].name(), "item");
}

TEST(LokiSemanticParser, ReturnedRequirementsAreDeclaredVerbatim)
{
    auto options = semantic::ParserOptions {};
    options.add_action_costs = false;
    auto parser = semantic::Parser(fixture_path("adl_requirements"), options);

    // Permissive mode neither validates nor rewrites requirements: the domain
    // declares :adl, so :adl is what comes back.
    auto kinds = std::set<formalism::RequirementKind> {};
    for (const auto requirement : parser.get_domain().requirements())
        kinds.insert(requirement.kind());

    const auto expected = std::set<formalism::RequirementKind> { formalism::RequirementKind::Adl };
    EXPECT_EQ(kinds, expected);
}

TEST(LokiSemanticParser, StrictModeAllowsForwardTypeReferences)
{
    auto options = semantic::ParserOptions {};
    options.strict = true;
    options.add_action_costs = false;
    auto parser = semantic::Parser(fixture_path("forward_types"), options);

    auto saw_child = false;
    auto saw_parent = false;
    for (const auto type : parser.get_domain().types())
    {
        if (type.name() == "child")
        {
            ASSERT_EQ(type.bases().size(), 1);
            EXPECT_EQ(type.bases()[0].name(), "parent");
            saw_child = true;
        }
        else if (type.name() == "parent")
        {
            ASSERT_EQ(type.bases().size(), 1);
            EXPECT_EQ(type.bases()[0].name(), "object");
            saw_parent = true;
        }
    }
    EXPECT_TRUE(saw_child);
    EXPECT_TRUE(saw_parent);
}

TEST(LokiSemanticParser, StrictModeChecksTaskQuantifierTypesAcrossRepositories)
{
    auto options = semantic::ParserOptions {};
    options.strict = true;
    options.add_action_costs = false;
    auto parser = semantic::Parser(fixture_path("quantified_types"), options);

    EXPECT_NO_THROW(parser.parse_task(fixture_path("quantified_types", "task.pddl")));
}

TEST(LokiSemanticParser, PreservesComplementaryLiteralsInConjunctions)
{
    auto options = semantic::ParserOptions {};
    options.add_action_costs = false;
    auto parser = semantic::Parser(fixture_path("complementary_literals"), options);
    const auto action = parser.get_domain().actions()[0];
    ASSERT_TRUE(action.precondition().has_value());
    auto conjuncts = std::size_t { 0 };
    std::visit(
        [&](const auto& node)
        {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::AndTag>>>)
                conjuncts = node.conditions().size();
        },
        action.precondition().value());
    // A complementary literal pair must not be canonicalized away.
    EXPECT_EQ(conjuncts, 3);
}

TEST(LokiSemanticParser, NormalizesMultiExpressionArityAndSortsOperandsWithoutDeduplicating)
{
    constexpr auto source = R"((define (domain arithmetic-arity)
  (:requirements :strips :numeric-fluents)
  (:functions (empty-add) (empty-mul) (unary-add) (unary-mul) (multi))
  (:action a-empty-add :parameters () :effect (assign (empty-add) (+)))
  (:action b-empty-mul :parameters () :effect (assign (empty-mul) (*)))
  (:action c-unary-add :parameters () :effect (assign (unary-add) (+ 7)))
  (:action d-unary-mul :parameters () :effect (assign (unary-mul) (* 8)))
  (:action e-multi :parameters () :effect (assign (multi) (+ 3 1 2 1)))
)
)";

    auto options = semantic::ParserOptions {};
    options.add_action_costs = false;
    const auto parser = semantic::Parser(std::string(source), options);
    const auto text = fmt::format("{}", parser.get_domain());

    EXPECT_NE(text.find("(assign (empty-add) 0)"), std::string::npos);
    EXPECT_NE(text.find("(assign (empty-mul) 1)"), std::string::npos);
    EXPECT_NE(text.find("(assign (unary-add) 7)"), std::string::npos);
    EXPECT_NE(text.find("(assign (unary-mul) 8)"), std::string::npos);
    EXPECT_NE(text.find("(assign (multi) (+ 1 1 2 3))"), std::string::npos);
}

TEST(LokiSemanticParser, ParsesAndTranslatesDistinctTasksAfterOneDomain)
{
    semantic::Parser parser(fixture_path("many_tasks"));

    const auto domain = parser.get_domain();
    const auto translation = semantic::translate(domain);

    const auto first_task = parser.parse_task(fixture_path("many_tasks", "task.pddl"));
    const auto first_translated_result = semantic::translate(first_task, translation);
    const auto first_translated = first_translated_result.get_translated_task();
    const auto second_task = parser.parse_task(fixture_path("many_tasks", "task2.pddl"));
    const auto second_translated_result = semantic::translate(second_task, translation);
    const auto second_translated = second_translated_result.get_translated_task();

    auto has_object = [](formalism::TaskView task, const std::string& name)
    {
        for (auto object : task.objects())
            if (std::string(object.name()) == name)
                return true;
        return false;
    };

    EXPECT_NE(first_translated.context().layer(), translation.get_translated_domain().context().layer());
    EXPECT_NE(second_translated.context().layer(), translation.get_translated_domain().context().layer());
    EXPECT_TRUE(first_translated.domain().context().contains(translation.get_translated_domain()));
    EXPECT_TRUE(second_translated.domain().context().contains(translation.get_translated_domain()));
    EXPECT_TRUE(has_object(first_translated, "a"));
    EXPECT_TRUE(has_object(second_translated, "b"));
    EXPECT_NE(&first_translated.context(), &second_translated.context());
    EXPECT_NE(std::string(first_translated.name()), std::string(second_translated.name()));
}

TEST(LokiSemanticParser, FailedTaskDoesNotLeakVariableScopes)
{
    auto options = semantic::ParserOptions {};
    options.add_action_costs = false;
    auto parser = semantic::Parser(fixture_path("task_isolation"), options);

    EXPECT_THROW(parser.parse_task(fixture_path("task_isolation", "bad-task.pddl")), semantic::UndefinedVariableError);

    EXPECT_THROW(parser.parse_task(fixture_path("task_isolation", "leaked-variable-task.pddl")), semantic::UndefinedVariableError);

    const auto task = parser.parse_task(fixture_path("task_isolation", "task.pddl"));
    EXPECT_EQ(task.name(), "good");
}

TEST(LokiSemanticParser, FlattensTypedGroupsAndInterleavedDeclarations)
{
    const auto source = std::string(R"((define (domain grouped)
  (:requirements :strips :typing :derived-predicates)
  (:types child sibling - parent parent)
  (:constants one two - child loose)
  (:predicates (p ?x ?y - child ?z - parent ?tail) (q ?x ?y - child) (r ?x - parent))
  (:action first :parameters (?x ?y - child ?z - parent ?tail)
    :precondition (q ?x ?y) :effect (p ?x ?y ?z ?tail))
  (:derived (q ?x ?y - child) (and))
  (:action second :parameters () :effect (and))
  (:derived (r ?x - parent) (and))))");
    const auto options = semantic::ParserOptions { .strict = true, .add_action_costs = false };
    auto parser = semantic::Parser(source, options);
    const auto domain = parser.get_domain();
    ASSERT_EQ(domain.actions().size(), 2);
    ASSERT_EQ(domain.axioms().size(), 2);
    const auto object_types = [](const auto& objects)
    {
        auto result = std::map<std::string, std::set<std::string>> {};
        for (const auto object : objects)
            for (const auto type : object.types())
                result[std::string(object.name())].insert(std::string(type.name()));
        return result;
    };
    const auto expected_constants = std::map<std::string, std::set<std::string>> { { "one", { "child" } }, { "two", { "child" } }, { "loose", { "object" } } };
    EXPECT_EQ(object_types(domain.constants()), expected_constants);
    auto type_bases = urd::Store<urd::map<std::string, std::string>> {};
    for (const auto type : domain.types())
        if (type.bases().size() == 1)
            type_bases.emplace(std::string(type.name()), std::string(type.bases()[0].name()));
    EXPECT_EQ(type_bases.at("child"), "parent");
    EXPECT_EQ(type_bases.at("sibling"), "parent");
    EXPECT_EQ(type_bases.at("parent"), "object");
    for (const auto action : domain.actions())
        if (action.name() == "first")
        {
            auto parameters = urd::Store<urd::sequence<std::pair<std::string, std::string>>> {};
            for (const auto parameter : action.parameters())
            {
                ASSERT_EQ(parameter.types().size(), 1);
                parameters.emplace_back(std::string(parameter.variable().name()), std::string(parameter.types()[0].name()));
            }
            const auto expected = urd::Store<urd::sequence<std::pair<std::string, std::string>>> { { "?x", "child" },
                                                                                                   { "?y", "child" },
                                                                                                   { "?z", "parent" },
                                                                                                   { "?tail", "object" } };
            EXPECT_EQ(parameters, expected);
        }
    auto found_grouped_axiom = false;
    for (const auto axiom : domain.axioms())
        if (axiom.head().atom().predicate().name() == "q")
        {
            found_grouped_axiom = true;
            EXPECT_EQ(axiom.arity(), 2);
            EXPECT_EQ(axiom.original_arity(), 2);
            ASSERT_EQ(axiom.parameters().size(), 2);
            EXPECT_EQ(axiom.parameters()[0].variable().name(), "?x");
            EXPECT_EQ(axiom.parameters()[1].variable().name(), "?y");
            EXPECT_EQ(axiom.head().atom().terms().size(), 2);
        }
    EXPECT_TRUE(found_grouped_axiom);
    const auto task = parser.parse_task(std::string("(define (problem grouped-task) (:domain grouped) "
                                                    "(:objects a b - child tail) (:init) (:goal (q a b)))"));
    const auto expected_objects = std::map<std::string, std::set<std::string>> { { "a", { "child" } }, { "b", { "child" } }, { "tail", { "object" } } };
    EXPECT_EQ(object_types(task.objects()), expected_objects);
}

TEST(LokiSemanticParser, GroupedDuplicatesAndCyclesRetainIdentifierDiagnostics)
{
    const auto options = semantic::ParserOptions { .strict = true, .add_action_costs = false };
    const auto prefix = std::string("(define (domain grouped) (:requirements :strips :typing) ");
    const auto expect_duplicate = [&]<typename Error>(const std::string& suffix, const std::string& name)
    {
        const auto source = prefix + suffix;
        try
        {
            const auto parser = semantic::Parser(source, options);
            FAIL() << "Expected grouped duplicate to be rejected";
        }
        catch (const Error& error)
        {
            const auto& diagnostic = error.diagnostic();
            ASSERT_TRUE(diagnostic.location);
            EXPECT_EQ(diagnostic.location->begin(), source.rfind(name));
            EXPECT_EQ(diagnostic.location->end(), source.rfind(name) + name.size());
            EXPECT_EQ(diagnostic.location->source()->text(), source);
        }
    };
    expect_duplicate.template operator()<semantic::DuplicateTypeError>("(:types child sibling - parent parent child))", "child");
    expect_duplicate.template operator()<semantic::DuplicateObjectError>("(:types child) (:constants one two one - child))", "one");
    expect_duplicate.template operator()<semantic::DuplicateVariableError>("(:types child) (:action a :parameters (?x ?y - child ?x) :effect (and)))", "?x");
    const auto cycle = prefix + "(:types a b - c c - a))";
    try
    {
        const auto parser = semantic::Parser(cycle, options);
        FAIL() << "Expected grouped type cycle to be rejected";
    }
    catch (const ygg::diagnostics::SemanticError& error)
    {
        EXPECT_NE(error.message().find("Cyclic type hierarchy"), std::string::npos);
        ASSERT_TRUE(error.diagnostic().location);
        EXPECT_EQ(error.diagnostic().location->begin(), cycle.find("a b"));
        EXPECT_EQ(error.diagnostic().location->end(), cycle.find("a b") + 1);
    }
}

}  // namespace loki::tests
