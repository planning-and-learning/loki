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

#include <gtest/gtest.h>
#include <loki/formalism/task_view.hpp>
#include <loki/parser/parser.hpp>
#include <loki/semantic/errors.hpp>
#include <loki/semantic/parser.hpp>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>

namespace loki::tests
{

TEST(LokiSemanticDiagnostics, ReportsSyntaxFailureMessage)
{
    try
    {
        auto parser = semantic::Parser(read_text(fixture_path("broken-syntax")));
        FAIL() << "Expected parse error";
    }
    catch (const semantic::ParseError& error)
    {
        const auto message = std::string(error.what());
        EXPECT_NE(message.find("while parsing domain definition"), std::string::npos);
        EXPECT_NE(message.find("In line 4:"), std::string::npos);
        EXPECT_NE(message.find("^_"), std::string::npos);
        ASSERT_TRUE(error.diagnostic().location);
        EXPECT_EQ(error.diagnostic().location->line(), 4);
    }
}

TEST(LokiSemanticDiagnostics, IncludesFileNameForPathDiagnostics)
{
    const auto domain_path = fixture_path("broken-syntax");
    try
    {
        auto parser = semantic::Parser(domain_path);
        FAIL() << "Expected parse error";
    }
    catch (const semantic::ParseError& error)
    {
        const auto message = std::string(error.what());
        EXPECT_NE(message.find("while parsing domain definition"), std::string::npos);
        EXPECT_NE(message.find("In file " + domain_path.string() + ", line 4:"), std::string::npos);
        ASSERT_TRUE(error.diagnostic().location);
        EXPECT_EQ(error.diagnostic().location->source()->filename(), domain_path.string());
    }

    const auto task_path = fixture_path("repo-mismatch", "task.pddl");
    try
    {
        auto parser = semantic::Parser(fixture_path("facade"));
        parser.parse_task(task_path);
        FAIL() << "Expected mismatched domain error";
    }
    catch (const semantic::MismatchedDomainError& error)
    {
        const auto message = std::string(error.what());
        EXPECT_NE(message.find("Task references domain 'second-translation-domain'"), std::string::npos);
        EXPECT_NE(message.find("In file " + task_path.string() + ", line 2:"), std::string::npos);
    }
}

TEST(LokiSemanticDiagnostics, MissingClosingParenthesisIncludesContextAndOpeningLocation)
{
    const auto source = std::string("(define (domain sample)\n  (:predicates (p)) ; ignored closing )\n");
    try
    {
        auto parser = semantic::Parser(source);
        FAIL() << "Expected parse error";
    }
    catch (const semantic::ParseError& error)
    {
        const auto& diagnostic = error.diagnostic();
        EXPECT_NE(diagnostic.message.find("while parsing domain definition"), std::string::npos);
        EXPECT_NE(diagnostic.message.find("')'"), std::string::npos);
        ASSERT_TRUE(diagnostic.location);
        EXPECT_EQ(diagnostic.location->begin(), source.size());
        EXPECT_EQ(diagnostic.location->line(), 3);
        EXPECT_EQ(diagnostic.location->column(), 1);
        ASSERT_EQ(diagnostic.notes.size(), 1);
        ASSERT_TRUE(diagnostic.notes.front().location);
        EXPECT_EQ(diagnostic.notes.front().message, "domain definition starts here");
        EXPECT_EQ(diagnostic.notes.front().location->begin(), 0);
        EXPECT_EQ(diagnostic.notes.front().location->source(), diagnostic.location->source());
        EXPECT_EQ(std::string(error.what()), ygg::diagnostics::format_diagnostic(diagnostic));
    }
}

TEST(LokiSemanticDiagnostics, NestedExpressionKeepsItsFirstFailureAndContext)
{
    const auto source = std::string("  (and (p) ; ) is a comment\n");
    auto output = std::ostringstream {};
    auto handler = parser::ErrorHandlerType(source.cbegin(), source.cend(), output);
    auto condition = ast::Condition {};
    EXPECT_FALSE(parser::parse_full(source, parser::condition(), condition, handler));
    ASSERT_TRUE(handler.last_error());
    const auto diagnostic = handler.diagnostic("fallback", source.cbegin());
    EXPECT_NE(diagnostic.message.find("while parsing and condition"), std::string::npos);
    ASSERT_TRUE(diagnostic.location);
    EXPECT_EQ(diagnostic.location.value().begin(), source.size());
    ASSERT_EQ(diagnostic.notes.size(), 1);
    ASSERT_TRUE(diagnostic.notes.front().location);
    EXPECT_EQ(diagnostic.notes.front().location.value().begin(), 2);
}

TEST(LokiSemanticDiagnostics, OnlyIdentifiedChildConditionsOwnTheirErrors)
{
    struct Case
    {
        std::string source;
        std::string owner;
        std::string opening;
        std::string offending;
    };
    for (const auto& item : {
             Case { "(and bare)", "and condition", "(and", "bare" },
             Case { "(and (", "and condition", "(and", "(" },
             Case { "(not bare)", "not condition", "(not", "bare" },
             Case { "(and (not bare))", "not condition", "(not", "bare" },
             Case { "(and (not (", "not condition", "(not", "(" },
             Case { "(and (not (p bare (q))))", "atom", "(p", "(q)" },
             Case { "(exists bare (p))", "exists condition", "(exists", "bare" },
             Case { "(exists (bare) (p))", "exists parameters", "(bare", "bare" },
             Case { "(imply (p) bare)", "imply condition", "(imply", "bare" },
         })
    {
        SCOPED_TRACE(item.source);
        auto output = std::ostringstream {};
        auto handler = parser::ErrorHandlerType(item.source.cbegin(), item.source.cend(), output);
        auto condition = ast::Condition {};
        EXPECT_FALSE(parser::parse_full(item.source, parser::condition(), condition, handler));
        ASSERT_TRUE(handler.last_error());
        const auto diagnostic = handler.diagnostic("fallback", item.source.cbegin());
        EXPECT_NE(diagnostic.message.find("while parsing " + item.owner), std::string::npos);
        ASSERT_TRUE(diagnostic.location);
        const auto opening = item.source.find(item.opening);
        EXPECT_EQ(diagnostic.location.value().begin(), item.source.find(item.offending, opening + 1));
        ASSERT_EQ(diagnostic.notes.size(), 1);
        EXPECT_EQ(diagnostic.notes.front().message, item.owner + " starts here");
        ASSERT_TRUE(diagnostic.notes.front().location);
        EXPECT_EQ(diagnostic.notes.front().location.value().begin(), opening);
    }
}

TEST(LokiSemanticDiagnostics, OnlyIdentifiedChildEffectsOwnTheirErrors)
{
    for (const auto& [source, owner, opening] : {
             std::tuple { std::string("(when bare (p))"), "when effect", "(when" },
             std::tuple { std::string("(when (not bare) (p))"), "not condition", "(not" },
             std::tuple { std::string("(forall bare (p))"), "forall effect", "(forall" },
             std::tuple { std::string("(forall (bare) (p))"), "forall parameters", "(bare" },
             std::tuple { std::string("(forall (?x) bare)"), "forall effect", "(forall" },
             std::tuple { std::string("(assign bare 1)"), "numeric effect", "(assign" },
             std::tuple { std::string("(not bare)"), "negated literal", "(not" },
         })
    {
        SCOPED_TRACE(source);
        auto output = std::ostringstream {};
        auto handler = parser::ErrorHandlerType(source.cbegin(), source.cend(), output);
        auto effect = ast::Effect {};
        EXPECT_FALSE(parser::parse_full(source, parser::effect(), effect, handler));
        ASSERT_TRUE(handler.last_error());
        const auto diagnostic = handler.diagnostic("fallback", source.cbegin());
        EXPECT_NE(diagnostic.message.find(std::string("while parsing ") + owner), std::string::npos);
        ASSERT_TRUE(diagnostic.location);
        EXPECT_EQ(diagnostic.location.value().begin(), source.find("bare"));
        ASSERT_EQ(diagnostic.notes.size(), 1);
        ASSERT_TRUE(diagnostic.notes.front().location);
        EXPECT_EQ(diagnostic.notes.front().location.value().begin(), source.find(opening));
    }
}

TEST(LokiSemanticDiagnostics, ExpectedSubjectsHaveReadableGrammarNames)
{
    for (const auto& [source, expected] : {
             std::pair { std::string("(define (domain))"), "Expected identifier while parsing domain header" },
             std::pair { std::string("(define (domain d) (:action))"), "Expected identifier while parsing action" },
             std::pair { std::string("(define (domain d) (:action a))"), "Expected :parameters while parsing action" },
             std::pair { std::string("(define (domain d) (:action a :parameters bare))"), "Expected action parameters while parsing action" },
             std::pair { std::string("(define (domain d) (:action a :parameters () :precondition))"), "Expected condition while parsing action" },
             std::pair { std::string("(define (domain d) (:action a :parameters () :precondition (exists bare (p))))"),
                         "Expected exists parameters while parsing exists condition" },
             std::pair { std::string("(define (domain d) (:types a - (either)))"), "Expected one or more type expressions while parsing either type" },
             std::pair { std::string("(define (problem p) (:init))"), "Expected problem domain while parsing problem definition" },
             std::pair { std::string("(define (problem p) (:domain) (:init))"), "Expected identifier while parsing problem domain" },
             std::pair { std::string("(define (problem p) (:domain d))"), "Expected init section while parsing problem definition" },
         })
    {
        SCOPED_TRACE(source);
        auto output = std::ostringstream {};
        auto handler = parser::ErrorHandlerType(source.cbegin(), source.cend(), output);
        auto file = ast::File {};
        EXPECT_FALSE(parser::parse_file(source, file, handler));
        ASSERT_TRUE(handler.last_error());
        EXPECT_EQ(handler.diagnostic("fallback", source.cbegin()).message, expected);
    }
}

TEST(LokiSemanticDiagnostics, RequirementPrefixesAreSyntaxRatherThanNameCleanup)
{
    for (const auto& [entry, expected] : {
             std::pair { std::string("typing"), "')'" },
             std::pair { std::string(":"), "requirement name" },
             std::pair { std::string("::typing"), "requirement name" },
             std::pair { std::string(": typing"), "requirement name" },
             std::pair { std::string(":\ttyping"), "requirement name" },
             std::pair { std::string(":; comment\ntyping"), "requirement name" },
             std::pair { std::string(":ty:ping"), "requirement name" },
             std::pair { std::string(":typing:"), "requirement name" },
         })
        for (const auto& source :
             { "(define (domain d) (:requirements " + entry + "))", "(define (problem p) (:domain d) (:requirements " + entry + ") (:init))" })
        {
            SCOPED_TRACE(source);
            auto output = std::ostringstream {};
            auto handler = parser::ErrorHandlerType(source.cbegin(), source.cend(), output);
            auto file = ast::File {};
            EXPECT_FALSE(parser::parse_file(source, file, handler));
            ASSERT_TRUE(handler.last_error());
            const auto diagnostic = handler.diagnostic("fallback", source.cbegin());
            EXPECT_EQ(diagnostic.message, std::string("Expected ") + expected + " while parsing requirements section");
            ASSERT_EQ(diagnostic.notes.size(), 1);
            ASSERT_TRUE(diagnostic.notes.front().location);
            EXPECT_EQ(diagnostic.notes.front().location.value().begin(), source.find("(:requirements"));
        }
}

TEST(LokiSemanticDiagnostics, UnknownRequirementKeepsItsCategoryAndBareNameSpan)
{
    for (const bool strict : { false, true })
        for (const bool task : { false, true })
        {
            SCOPED_TRACE(strict);
            SCOPED_TRACE(task);
            const auto options = semantic::ParserOptions { .strict = strict, .add_action_costs = false };
            const auto source = task ? std::string("(define (problem p) (:domain d) (:requirements :future-feature) (:init))") :
                                       std::string("(define (domain d) (:requirements :strips :future-feature))");
            try
            {
                if (task)
                {
                    auto parser = semantic::Parser(std::string("(define (domain d) (:requirements :strips))"), options);
                    parser.parse_task(source);
                }
                else
                {
                    auto parser = semantic::Parser(source, options);
                }
                FAIL() << "Expected unsupported requirement";
            }
            catch (const semantic::UnsupportedRequirementError& error)
            {
                EXPECT_EQ(error.message(), "Unsupported requirement: :future-feature");
                ASSERT_TRUE(error.diagnostic().location);
                const auto& span = *error.diagnostic().location;
                EXPECT_EQ(span.begin(), source.find(":future-feature") + 1);
                EXPECT_EQ(span.end(), span.begin() + std::string("future-feature").size());
                EXPECT_EQ(span.source()->text().substr(span.begin(), span.end() - span.begin()), "future-feature");
            }
        }
}

TEST(LokiSemanticDiagnostics, UnexpectedAndTrailingInputPointAtTheOffendingToken)
{
    for (const auto& source : { std::string("(define (domain sample)\n  (:unknown))"), std::string("(define (domain sample)) ; comment\nextra") })
    {
        SCOPED_TRACE(source);
        try
        {
            auto parser = semantic::Parser(source);
            FAIL() << "Expected parse error";
        }
        catch (const semantic::ParseError& error)
        {
            ASSERT_TRUE(error.diagnostic().location);
            const auto offset = source.find("(:unknown") != std::string::npos ? source.find("(:unknown") : source.find("extra");
            EXPECT_EQ(error.diagnostic().location->begin(), offset);
            EXPECT_EQ(error.diagnostic().location->line(), 2);
            if (source.find("extra") != std::string::npos)
            {
                EXPECT_EQ(error.message(), "Unexpected trailing input.");
            }
        }
    }
}

TEST(LokiSemanticDiagnostics, KeywordPrefixesAndAmbiguousArithmeticStillParse)
{
    for (const auto& source : { std::string("(andrew)"),
                                std::string("(ordinary)"),
                                std::string("(nothing)"),
                                std::string("(increase_count)"),
                                std::string("(minuscular)"),
                                std::string("(= ?x ?y)"),
                                std::string("(not (= ?x ?y))"),
                                std::string("(= (fuel) 3)"),
                                std::string("(forall (?x) (p ?x))") })
    {
        SCOPED_TRACE(source);
        auto output = std::ostringstream {};
        auto handler = parser::ErrorHandlerType(source.cbegin(), source.cend(), output);
        auto condition = ast::Condition {};
        EXPECT_TRUE(parser::parse_full(source, parser::condition(), condition, handler));
        EXPECT_FALSE(handler.last_error());
        EXPECT_TRUE(output.str().empty());
    }
    for (const auto& source : { std::string("(- 1)"),
                                std::string("(- 1 2)"),
                                std::string("(/ 4 2)"),
                                std::string("(+ 1 2)"),
                                std::string("(-1)"),
                                std::string("(-1 2)"),
                                std::string("(/4 2)"),
                                std::string("(+1 2)"),
                                std::string("(*1 2)"),
                                std::string("(--1)"),
                                std::string("(- (- 1) (- 2 3))"),
                                std::string("(minus 1)"),
                                std::string("(minuscular)") })
    {
        SCOPED_TRACE(source);
        auto output = std::ostringstream {};
        auto handler = parser::ErrorHandlerType(source.cbegin(), source.cend(), output);
        auto expression = ast::FunctionExpression {};
        EXPECT_TRUE(parser::parse_full(source, parser::function_expression(), expression, handler));
        EXPECT_FALSE(handler.last_error());
        EXPECT_TRUE(output.str().empty());
    }
    for (const auto& source : { std::string("(define (domain d))"),
                                std::string("(define (problem p) (:domain d) (:init))"),
                                std::string("(define (problem p) (:domain d) (:init (= (fuel) 3) (= a b)))") })
    {
        SCOPED_TRACE(source);
        auto output = std::ostringstream {};
        auto handler = parser::ErrorHandlerType(source.cbegin(), source.cend(), output);
        auto file = ast::File {};
        EXPECT_TRUE(parser::parse_file(source, file, handler));
        EXPECT_FALSE(handler.last_error());
    }
}

TEST(LokiSemanticDiagnostics, ArithmeticErrorsKeepTheInnermostIdentifiedExpression)
{
    for (const auto& [source, owner, opening] : {
             std::tuple { std::string("(- bare)"), "unary expression", "(-" },
             std::tuple { std::string("(/ bare 2)"), "binary expression", "(/" },
             std::tuple { std::string("(- (/ 1 bare) 2)"), "binary expression", "(/" },
             std::tuple { std::string("(+ (minus bare))"), "unary expression", "(minus" },
         })
    {
        SCOPED_TRACE(source);
        auto output = std::ostringstream {};
        auto handler = parser::ErrorHandlerType(source.cbegin(), source.cend(), output);
        auto expression = ast::FunctionExpression {};
        EXPECT_FALSE(parser::parse_full(source, parser::function_expression(), expression, handler));
        ASSERT_TRUE(handler.last_error());
        const auto diagnostic = handler.diagnostic("fallback", source.cbegin());
        EXPECT_NE(diagnostic.message.find(std::string("while parsing ") + owner), std::string::npos);
        ASSERT_TRUE(diagnostic.location);
        EXPECT_EQ(diagnostic.location.value().begin(), source.find("bare"));
        ASSERT_EQ(diagnostic.notes.size(), 1);
        ASSERT_TRUE(diagnostic.notes.front().location);
        EXPECT_EQ(diagnostic.notes.front().location.value().begin(), source.find(opening));
    }
}

TEST(LokiSemanticDiagnostics, TypedSemanticErrorOwnsItsSourceAfterParserDestruction)
{
    const auto error = []
    {
        const auto source = std::string("(define (domain sample) (:requirements :strips)\n (:predicates (p))\n"
                                        " (:action a :parameters () :precondition (missing) :effect (p)))");
        try
        {
            auto parser = semantic::Parser(source, semantic::ParserOptions { .strict = true, .add_action_costs = false });
        }
        catch (const semantic::UndefinedPredicateError& error)
        {
            return error;
        }
        throw std::logic_error("Expected UndefinedPredicateError");
    }();
    const auto& diagnostic = error.diagnostic();
    EXPECT_EQ(diagnostic.message, "Undefined predicate: missing");
    ASSERT_TRUE(diagnostic.location);
    const auto& source = diagnostic.location.value().source()->text();
    EXPECT_EQ(diagnostic.location.value().begin(), source.find("missing"));
    EXPECT_EQ(diagnostic.location.value().end(), source.find("missing") + 7);
    EXPECT_EQ(diagnostic.location.value().line(), 3);
    EXPECT_EQ(std::string(error.what()), ygg::diagnostics::format_diagnostic(diagnostic));
}

}  // namespace loki::tests
