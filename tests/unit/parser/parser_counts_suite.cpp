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

#include <filesystem>
#include <gtest/gtest.h>
#include <iostream>
#include <loki/ast/formatter.hpp>
#include <loki/formalism/domain_view.hpp>
#include <loki/formalism/repository.hpp>
#include <loki/formalism/task_view.hpp>
#include <loki/parser/parser.hpp>
#include <loki/parser/parser_def.hpp>
#include <loki/semantic/options.hpp>
#include <loki/semantic/parser.hpp>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <yggdrasil/serialization/json_suite.hpp>

namespace loki::tests
{
namespace
{

// A named X3 rule hides its definition's has_action flag, so check each RHS.
template<typename... Parsers>
constexpr bool no_semantic_actions(const Parsers&...)
{ return (!Parsers::has_action && ...); }

static_assert(no_semantic_actions(parser::rules::identifier_def,
                                  parser::rules::type_expression_def,
                                  parser::rules::typed_name_group.rhs,
                                  parser::rules::typed_variable_group.rhs,
                                  parser::rules::typed_name_list_def,
                                  parser::rules::typed_variable_list_def,
                                  parser::rules::term_def,
                                  parser::rules::atom_def,
                                  parser::rules::literal_def,
                                  parser::rules::function_term_def,
                                  parser::rules::function_expression_def,
                                  parser::rules::condition_empty.rhs,
                                  parser::rules::condition_def,
                                  parser::rules::effect_def,
                                  parser::rules::predicate_declaration_def,
                                  parser::rules::function_declaration_def,
                                  parser::rules::action_def,
                                  parser::rules::axiom_def,
                                  parser::rules::metric_def,
                                  parser::rules::initial_function_value_def,
                                  parser::rules::domain_def,
                                  parser::rules::task_def,
                                  parser::rules::file_def,
                                  parser::rules::detail::symbol_character,
                                  parser::rules::detail::symbol,
                                  parser::rules::detail::operator_symbol,
                                  parser::rules::detail::variable_identifier_def,
                                  parser::rules::detail::type_reference_def,
                                  parser::rules::detail::type_expressions.rhs,
                                  parser::rules::detail::either_type_def,
                                  parser::rules::detail::function_expression_number_def,
                                  parser::rules::detail::function_expression_function_def,
                                  parser::rules::detail::function_expression_unary_def,
                                  parser::rules::detail::function_expression_binary_def,
                                  parser::rules::detail::function_expression_multi_def,
                                  parser::rules::detail::condition_literal_def,
                                  parser::rules::detail::condition_and_def,
                                  parser::rules::detail::condition_or_def,
                                  parser::rules::detail::condition_not_def,
                                  parser::rules::detail::condition_imply_def,
                                  parser::rules::detail::condition_exists_def,
                                  parser::rules::detail::condition_forall_def,
                                  parser::rules::detail::condition_numeric_constraint_def,
                                  parser::rules::detail::effect_literal_def,
                                  parser::rules::detail::effect_and_def,
                                  parser::rules::detail::effect_numeric_def,
                                  parser::rules::detail::effect_forall_def,
                                  parser::rules::detail::effect_when_def,
                                  parser::rules::detail::effect_one_of_def,
                                  parser::rules::detail::probabilistic_effect_alternative_def,
                                  parser::rules::detail::effect_probabilistic_def,
                                  parser::rules::detail::requirement_name.rhs,
                                  parser::rules::detail::requirement_section_def,
                                  parser::rules::detail::type_section_def,
                                  parser::rules::detail::constant_section_def,
                                  parser::rules::detail::predicate_section_def,
                                  parser::rules::detail::function_section_def,
                                  parser::rules::detail::object_section_def,
                                  parser::rules::detail::initial_section_def));
static_assert(!decltype(parser::rules::detail::keyword("keyword").rhs)::has_action);

struct DomainExpectation
{
    std::size_t requirements;
    std::size_t types;
    std::size_t constants;
    std::size_t predicates;
    std::size_t functions;
    std::size_t actions;
    std::size_t axioms;
};

struct TaskExpectation
{
    std::size_t requirements;
    std::size_t objects;
    std::size_t initial_literals;
    std::size_t initial_function_values;
    std::size_t predicates;
    std::size_t axioms;
    bool has_goal;
    bool has_metric;
};

struct ConfigurationExpectation
{
    semantic::ParserOptions options;
    DomainExpectation domain;
    TaskExpectation task;
};

struct ParserCountExpectation
{
    std::string name;
    std::filesystem::path domain_file;
    std::filesystem::path task_file;
    std::vector<ConfigurationExpectation> configurations;
};

void expect_counts(formalism::DomainView domain, formalism::TaskView task, const ConfigurationExpectation& expected)
{
    EXPECT_EQ(domain.get_requirements().size(), expected.domain.requirements);
    EXPECT_EQ(domain.get_types().size(), expected.domain.types);
    EXPECT_EQ(domain.get_constants().size(), expected.domain.constants);
    EXPECT_EQ(domain.get_predicates().size(), expected.domain.predicates);
    EXPECT_EQ(domain.get_functions().size(), expected.domain.functions);
    EXPECT_EQ(domain.get_actions().size(), expected.domain.actions);
    EXPECT_EQ(domain.get_axioms().size(), expected.domain.axioms);
    EXPECT_EQ(task.get_requirements().size(), expected.task.requirements);
    EXPECT_EQ(task.get_objects().size(), expected.task.objects);
    EXPECT_EQ(task.get_initial_literals().size(), expected.task.initial_literals);
    EXPECT_EQ(task.get_initial_function_values().size(), expected.task.initial_function_values);
    EXPECT_EQ(task.get_predicates().size(), expected.task.predicates);
    EXPECT_EQ(task.get_axioms().size(), expected.task.axioms);
    EXPECT_EQ(static_cast<bool>(task.get_goal()), expected.task.has_goal);
    EXPECT_EQ(static_cast<bool>(task.get_metric()), expected.task.has_metric);
}

template<typename Node, typename ParseFn>
void expect_parse_format_reparse(const std::filesystem::path& path, ParseFn parse)
{
    const auto source = read_text(path);
    auto first = source.cbegin();
    const auto last = source.cend();
    parser::ErrorHandlerType error_handler(first, last, std::cerr);

    auto ast = Node {};
    ASSERT_TRUE(parse(source, ast, error_handler)) << path;

    const auto printed = fmt::format("{}", ast);
    auto printed_first = printed.cbegin();
    const auto printed_last = printed.cend();
    parser::ErrorHandlerType printed_error_handler(printed_first, printed_last, std::cerr);

    auto reparsed = Node {};
    EXPECT_TRUE(parse(printed, reparsed, printed_error_handler)) << path << '\n' << printed;
}

ConfigurationExpectation parse_configuration(const boost::json::object& configuration)
{
    const auto& options_object = ygg::common::as_object(configuration, "parser_options", "case.configuration");
    const auto& domain = ygg::common::as_object(configuration, "domain", "case.configuration");
    const auto& task = ygg::common::as_object(configuration, "task", "case.configuration");
    auto options = semantic::ParserOptions {};
    options.strict = ygg::common::as_bool(options_object, "strict", "case.configuration.parser_options");
    options.add_action_costs = ygg::common::as_bool(options_object, "add_action_costs", "case.configuration.parser_options");
    return ConfigurationExpectation { options,
                                      DomainExpectation { ygg::common::as_size(domain, "requirements", "case.configuration.domain"),
                                                          ygg::common::as_size(domain, "types", "case.configuration.domain"),
                                                          ygg::common::as_size(domain, "constants", "case.configuration.domain"),
                                                          ygg::common::as_size(domain, "predicates", "case.configuration.domain"),
                                                          ygg::common::as_size(domain, "functions", "case.configuration.domain"),
                                                          ygg::common::as_size(domain, "actions", "case.configuration.domain"),
                                                          ygg::common::as_size(domain, "axioms", "case.configuration.domain") },
                                      TaskExpectation { ygg::common::as_size(task, "requirements", "case.configuration.task"),
                                                        ygg::common::as_size(task, "objects", "case.configuration.task"),
                                                        ygg::common::as_size(task, "initial_literals", "case.configuration.task"),
                                                        ygg::common::as_size(task, "initial_function_values", "case.configuration.task"),
                                                        ygg::common::as_size(task, "predicates", "case.configuration.task"),
                                                        ygg::common::as_size(task, "axioms", "case.configuration.task"),
                                                        ygg::common::as_bool(task, "has_goal", "case.configuration.task"),
                                                        ygg::common::as_bool(task, "has_metric", "case.configuration.task") } };
}

std::vector<ParserCountExpectation> load_expectations()
{
    const auto suite_value = ygg::common::load_json_file(ygg::common::root_path() / "tests/fixtures/parser_counts_suite.json");
    const auto& suite = ygg::common::as_object(suite_value, "suite");
    const auto benchmark_root = std::filesystem::path(std::string(BENCHMARKS_DIR));
    auto result = std::vector<ParserCountExpectation> {};
    for (const auto& case_value : ygg::common::as_array(suite, "cases", "suite"))
    {
        const auto& object = ygg::common::as_object(case_value, "case");
        auto item = ParserCountExpectation { ygg::common::as_string(object, "name", "case"),
                                             ygg::common::resolve_path(benchmark_root, ygg::common::as_string(object, "domain_file", "case")),
                                             ygg::common::resolve_path(benchmark_root, ygg::common::as_string(object, "task_file", "case")),
                                             {} };
        for (const auto& configuration_value : ygg::common::as_array(object, "configurations", "case"))
            item.configurations.push_back(parse_configuration(ygg::common::as_object(configuration_value, "case.configuration")));
        result.push_back(std::move(item));
    }
    return result;
}

}  // namespace

TEST(LokiParserCountsSuite, GeneratedSuiteCoversEveryBenchmarkProblem)
{
    const auto benchmark_root = std::filesystem::path(std::string(BENCHMARKS_DIR));
    auto listed = std::set<std::string> {};
    for (const auto& item : load_expectations())
        listed.insert(std::filesystem::relative(item.domain_file, benchmark_root).generic_string() + "|"
                      + std::filesystem::relative(item.task_file, benchmark_root).generic_string());

    auto discovered = std::set<std::string> {};
    for (const auto& tree : { benchmark_root / "classical" / "tests", benchmark_root / "numeric" / "tests" })
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(tree))
        {
            if (!entry.is_regular_file() || entry.path().filename() != "domain.pddl")
                continue;
            for (const auto& task_entry : std::filesystem::directory_iterator(entry.path().parent_path()))
            {
                if (task_entry.is_regular_file() && task_entry.path().extension() == ".pddl" && task_entry.path().filename() != "domain.pddl")
                    discovered.insert(std::filesystem::relative(entry.path(), benchmark_root).generic_string() + "|"
                                      + std::filesystem::relative(task_entry.path(), benchmark_root).generic_string());
            }
        }
    }

    EXPECT_EQ(listed, discovered);
}

TEST(LokiParserCountsSuite, RawBenchmarkAstsRoundTrip)
{
    const auto cases = load_expectations();
    ASSERT_FALSE(cases.empty());
    for (const auto& item : cases)
    {
        SCOPED_TRACE(item.name);
        expect_parse_format_reparse<ast::Domain>(item.domain_file, parser::parse_domain);
        expect_parse_format_reparse<ast::Task>(item.task_file, parser::parse_task);
    }
}

TEST(LokiParserCountsSuite, RawArithmeticAstPreservesEmptyAndUnaryMultiExpressions)
{
    struct Case
    {
        const char* source;
        const char* op;
        std::size_t arity;
    };

    for (const auto& item : { Case { "(+)", "+", 0 }, Case { "(+ 2)", "+", 1 }, Case { "(*)", "*", 0 }, Case { "(* 2)", "*", 1 } })
    {
        const auto source = std::string(item.source);
        auto first = source.cbegin();
        const auto last = source.cend();
        parser::ErrorHandlerType error_handler(first, last, std::cerr);
        auto expression = ast::FunctionExpression {};

        ASSERT_TRUE(parser::parse_full(source, parser::function_expression(), expression, error_handler));
        const auto* multi = boost::get<ast::FunctionExpressionMulti>(&expression.get());
        ASSERT_NE(multi, nullptr);
        EXPECT_EQ(multi->op, item.op);
        EXPECT_EQ(multi->expressions.size(), item.arity);
        EXPECT_EQ(fmt::format("{}", expression), source);
    }
}

TEST(LokiParserCountsSuite, SemanticBenchmarkCountsStayStable)
{
    const auto cases = load_expectations();
    ASSERT_FALSE(cases.empty());
    for (const auto& item : cases)
    {
        SCOPED_TRACE(item.name);
        ASSERT_EQ(item.configurations.size(), 4);
        for (const auto& expected : item.configurations)
        {
            SCOPED_TRACE("strict=" + std::to_string(expected.options.strict) + " add_action_costs=" + std::to_string(expected.options.add_action_costs));
            auto semantic_parser = semantic::Parser(item.domain_file, expected.options);
            const auto domain = semantic_parser.get_domain();
            const auto task = semantic_parser.parse_task(item.task_file);
            expect_counts(domain, task, expected);
        }
    }
}

TEST(LokiParserCountsSuite, GroupedAstRetainsNamesTypesSpansAndDeclarationOrder)
{
    const auto source = std::string(R"((define (domain grouped)
  (:requirements :strips :typing :derived-predicates)
  (:types child sibling - parent parent)
  (:constants one two - child loose)
  (:predicates (p ?x ?y - child ?z - parent ?tail))
  (:functions (cost ?x ?y - child ?tail) - number)
  (:action first :parameters (?x ?y - child ?z - parent ?tail) :effect (and))
  (:derived (q ?x ?y - child ?tail) (and))
  (:action second :parameters () :effect (and))
  (:derived (r ?x - parent) (and))))");
    parser::ErrorHandlerType handler(source.cbegin(), source.cend(), std::cerr);
    auto domain = ast::Domain {};
    ASSERT_TRUE(parser::parse_domain(source, domain, handler));
    auto expect_span = [&](const boost::spirit::x3::position_tagged& node, const std::string& expected)
    {
        const auto diagnostic = handler.make_diagnostic(node, "span");
        ASSERT_TRUE(diagnostic.location);
        const auto& span = *diagnostic.location;
        EXPECT_EQ(span.source()->text().substr(span.begin(), span.end() - span.begin()), expected);
    };
    expect_span(domain.name, "grouped");
    ASSERT_EQ(domain.requirements.size(), 3);
    const auto& requirement = domain.requirements[0];
    EXPECT_EQ(requirement.name.text, "strips");
    expect_span(requirement.name, "strips");
    EXPECT_EQ(fmt::format("{}", requirement), ":strips");
    ASSERT_EQ(domain.types.size(), 2);
    ASSERT_EQ(domain.types[0].names.size(), 2);
    EXPECT_EQ(domain.types[0].names[0].text, "child");
    EXPECT_EQ(domain.types[0].names[1].text, "sibling");
    ASSERT_TRUE(domain.types[0].type);
    expect_span(*domain.types[0].type, "parent");
    expect_span(domain.types[0], "child sibling - parent");
    EXPECT_FALSE(domain.types[1].type);
    ASSERT_EQ(domain.constants.size(), 2);
    ASSERT_EQ(domain.constants[0].names.size(), 2);
    expect_span(domain.constants[0].names[1], "two");
    EXPECT_FALSE(domain.constants[1].type);
    ASSERT_EQ(domain.predicates.size(), 1);
    ASSERT_EQ(domain.predicates[0].parameters.size(), 3);
    ASSERT_EQ(domain.functions.size(), 1);
    ASSERT_EQ(domain.functions[0].parameters.size(), 2);
    ASSERT_TRUE(domain.functions[0].type);
    expect_span(*domain.functions[0].type, "number");

    ASSERT_EQ(domain.declarations.size(), 4);
    const auto* first = boost::get<ast::Action>(&domain.declarations[0].get());
    const auto* first_axiom = boost::get<ast::Axiom>(&domain.declarations[1].get());
    const auto* second = boost::get<ast::Action>(&domain.declarations[2].get());
    const auto* second_axiom = boost::get<ast::Axiom>(&domain.declarations[3].get());
    ASSERT_NE(first, nullptr);
    ASSERT_NE(first_axiom, nullptr);
    ASSERT_NE(second, nullptr);
    ASSERT_NE(second_axiom, nullptr);
    EXPECT_EQ(first->name.text, "first");
    EXPECT_EQ(first_axiom->head.name.text, "q");
    EXPECT_EQ(second->name.text, "second");
    EXPECT_EQ(second_axiom->head.name.text, "r");
    ASSERT_EQ(first->parameters.size(), 3);
    const auto& group = first->parameters[0];
    ASSERT_EQ(group.names.size(), 2);
    EXPECT_EQ(group.names[0].text, "?x");
    EXPECT_EQ(group.names[1].text, "?y");
    expect_span(group.names[0], "?x");
    expect_span(group.names[1], "?y");
    expect_span(group, "?x ?y - child");
    ASSERT_TRUE(group.type);
    const auto* type = boost::get<ast::TypeReference>(&group.type->get());
    ASSERT_NE(type, nullptr);
    expect_span(*group.type, "child");
    expect_span(type->name, "child");
    EXPECT_FALSE(first->parameters[2].type);
    expect_span(first->parameters[2].names[0], "?tail");
    expect_span(first->name, "first");
    expect_span(first_axiom->head.name, "q");

    const auto printed = fmt::format("{}", domain);
    EXPECT_LT(printed.find("(:action first"), printed.find("(:derived (q"));
    EXPECT_LT(printed.find("(:derived (q"), printed.find("(:action second"));
    EXPECT_LT(printed.find("(:action second"), printed.find("(:derived (r"));
    parser::ErrorHandlerType printed_handler(printed.cbegin(), printed.cend(), std::cerr);
    auto reparsed = ast::Domain {};
    ASSERT_TRUE(parser::parse_domain(printed, reparsed, printed_handler));
    EXPECT_EQ(fmt::format("{}", reparsed), printed);
    EXPECT_EQ(reparsed.declarations.size(), domain.declarations.size());
    EXPECT_EQ(reparsed.types.size(), domain.types.size());
}

TEST(LokiParserCountsSuite, TaskObjectGroupsAndTaskAxiomsRoundTrip)
{
    const auto source = std::string(R"((define (problem grouped-task) (:domain grouped)
  (:requirements :strips :typing)
  (:objects one two - child loose) (:init) (:goal (and))
  (:derived (q ?x ?y - child ?tail) (and))))");
    parser::ErrorHandlerType handler(source.cbegin(), source.cend(), std::cerr);
    auto task = ast::Task {};
    ASSERT_TRUE(parser::parse_task(source, task, handler));
    ASSERT_EQ(task.requirements.size(), 2);
    EXPECT_EQ(task.requirements[0].name.text, "strips");
    EXPECT_EQ(task.requirements[1].name.text, "typing");
    EXPECT_EQ(fmt::format("{}", task.requirements[1]), ":typing");
    ASSERT_EQ(task.objects.size(), 2);
    ASSERT_EQ(task.objects[0].names.size(), 2);
    ASSERT_TRUE(task.objects[0].type);
    EXPECT_FALSE(task.objects[1].type);
    ASSERT_EQ(task.axioms.size(), 1);
    ASSERT_EQ(task.axioms[0].head.parameters.size(), 2);
    const auto printed = fmt::format("{}", task);
    parser::ErrorHandlerType printed_handler(printed.cbegin(), printed.cend(), std::cerr);
    auto reparsed = ast::Task {};
    ASSERT_TRUE(parser::parse_task(printed, reparsed, printed_handler));
    EXPECT_EQ(fmt::format("{}", reparsed), printed);
    EXPECT_EQ(reparsed.axioms.size(), task.axioms.size());
}

}  // namespace loki::tests
