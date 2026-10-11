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

#include <filesystem>
#include <gtest/gtest.h>
#include <loki/formalism/formatter.hpp>
#include <loki/formalism/repository.hpp>
#include <loki/semantic/options.hpp>
#include <loki/semantic/parser.hpp>
#include <loki/semantic/translator.hpp>
#include <string>
#include <vector>
#include <yggdrasil/io/project_path.hpp>
#include <yggdrasil/serialization/json.hpp>

namespace loki::tests
{
namespace
{

struct ParserSuiteCase
{
    std::string name;
    std::filesystem::path domain_file;
    std::filesystem::path task_file;
};

struct ConfigurationExpectation
{
    semantic::ParserOptions parser_options;
    semantic::TranslatorOptions options;
    std::size_t domain_types;
    std::size_t domain_predicates;
    std::size_t domain_functions;
    std::size_t domain_actions;
    std::size_t domain_axioms;
    std::size_t task_objects;
    std::size_t task_predicates;
    std::size_t task_initial_literals;
    std::size_t task_initial_function_values;
    std::size_t task_actions;
    std::size_t task_axioms;
};

struct TranslationCountExpectation
{
    std::string name;
    std::vector<ConfigurationExpectation> configurations;
};

ParserSuiteCase parse_parser_case(const boost::json::object& object)
{
    return ParserSuiteCase { ygg::common::as_string(object, "name", "case"),
                             ygg::common::resolve_path(std::filesystem::path(BENCHMARKS_DIR), ygg::common::as_string(object, "domain_file", "case")),
                             ygg::common::resolve_path(std::filesystem::path(BENCHMARKS_DIR), ygg::common::as_string(object, "task_file", "case")) };
}

std::vector<ParserSuiteCase> load_parser_cases()
{
    const auto suite_value = ygg::common::load_json_file(ygg::common::root_path() / "tests/fixtures/parser_counts_suite.json");
    const auto& suite = ygg::common::as_object(suite_value, "suite");
    auto result = std::vector<ParserSuiteCase> {};
    for (const auto& case_value : ygg::common::as_array(suite, "cases", "suite"))
        result.push_back(parse_parser_case(ygg::common::as_object(case_value, "case")));
    return result;
}

TranslationCountExpectation parse_expectation(const boost::json::object& object)
{
    auto result = TranslationCountExpectation { ygg::common::as_string(object, "name", "case"), {} };
    for (const auto& configuration_value : ygg::common::as_array(object, "configurations", "case"))
    {
        const auto& configuration = ygg::common::as_object(configuration_value, "case.configuration");
        const auto& parser_options_object = ygg::common::as_object(configuration, "parser_options", "case.configuration");
        const auto& options = ygg::common::as_object(configuration, "options", "case.configuration");
        const auto& domain = ygg::common::as_object(configuration, "translated_domain", "case.configuration");
        const auto& task = ygg::common::as_object(configuration, "translated_task", "case.configuration");
        auto parser_options = semantic::ParserOptions {};
        parser_options.add_action_costs = ygg::common::as_bool(parser_options_object, "add_action_costs", "case.configuration.parser_options");
        auto translator_options = semantic::TranslatorOptions {};
        translator_options.compile_typing = ygg::common::as_bool(options, "compile_typing", "case.configuration.options");
        translator_options.compile_conditional_effects = ygg::common::as_bool(options, "compile_conditional_effects", "case.configuration.options");
        translator_options.materialize_equality = ygg::common::as_bool(options, "materialize_equality", "case.configuration.options");
        result.configurations.push_back(ConfigurationExpectation { parser_options,
                                                                   translator_options,
                                                                   ygg::common::as_size(domain, "types", "case.translated_domain"),
                                                                   ygg::common::as_size(domain, "predicates", "case.translated_domain"),
                                                                   ygg::common::as_size(domain, "functions", "case.translated_domain"),
                                                                   ygg::common::as_size(domain, "actions", "case.translated_domain"),
                                                                   ygg::common::as_size(domain, "axioms", "case.translated_domain"),
                                                                   ygg::common::as_size(task, "objects", "case.translated_task"),
                                                                   ygg::common::as_size(task, "predicates", "case.translated_task"),
                                                                   ygg::common::as_size(task, "initial_literals", "case.translated_task"),
                                                                   ygg::common::as_size(task, "initial_function_values", "case.translated_task"),
                                                                   ygg::common::as_size(task, "actions", "case.translated_task"),
                                                                   ygg::common::as_size(task, "axioms", "case.translated_task") });
    }
    return result;
}

std::vector<TranslationCountExpectation> load_expectations()
{
    const auto suite_value = ygg::common::load_json_file(ygg::common::root_path() / "tests/fixtures/translation_counts_suite.json");
    const auto& suite = ygg::common::as_object(suite_value, "suite");
    auto result = std::vector<TranslationCountExpectation> {};
    for (const auto& case_value : ygg::common::as_array(suite, "cases", "suite"))
        result.push_back(parse_expectation(ygg::common::as_object(case_value, "case")));
    return result;
}

}  // namespace

TEST(LokiSemanticTranslationCountsSuite, TranslatedBenchmarkCountsStayStable)
{
    const auto cases = load_parser_cases();
    const auto expectations = load_expectations();
    ASSERT_EQ(cases.size(), expectations.size());
    for (auto i = std::size_t { 0 }; i < cases.size(); ++i)
    {
        const auto& item = cases[i];
        const auto& expected = expectations[i];
        SCOPED_TRACE(item.name);
        ASSERT_EQ(item.name, expected.name);

        for (const auto& configuration : expected.configurations)
        {
            SCOPED_TRACE("add_action_costs=" + std::to_string(configuration.parser_options.add_action_costs)
                         + " compile_typing=" + std::to_string(configuration.options.compile_typing)
                         + " compile_conditional_effects=" + std::to_string(configuration.options.compile_conditional_effects)
                         + " materialize_equality=" + std::to_string(configuration.options.materialize_equality));
            auto parser = semantic::Parser(item.domain_file, configuration.parser_options);
            const auto task = parser.parse_task(item.task_file);
            const auto domain_translation = semantic::translate(parser.get_domain(), configuration.options);
            const auto translated_domain = domain_translation.get_translated_domain();
            const auto task_translation = semantic::translate(task, domain_translation, configuration.options);
            const auto translated_task = task_translation.get_translated_task();

            EXPECT_EQ(translated_domain.get_types().size(), configuration.domain_types);
            EXPECT_EQ(translated_domain.get_predicates().size(), configuration.domain_predicates);
            EXPECT_EQ(translated_domain.get_functions().size(), configuration.domain_functions);
            EXPECT_EQ(translated_domain.get_actions().size(), configuration.domain_actions);
            EXPECT_EQ(translated_domain.get_axioms().size(), configuration.domain_axioms);
            EXPECT_EQ(translated_task.get_objects().size(), configuration.task_objects);
            EXPECT_EQ(translated_task.get_predicates().size(), configuration.task_predicates);
            EXPECT_EQ(translated_task.get_initial_literals().size(), configuration.task_initial_literals);
            EXPECT_EQ(translated_task.get_initial_function_values().size(), configuration.task_initial_function_values);
            EXPECT_EQ(translated_task.get_domain().get_actions().size(), configuration.task_actions);
            EXPECT_EQ(translated_task.get_axioms().size(), configuration.task_axioms);

            // The formatted translation must reparse under the same parser options with stable counts.
            const auto domain_text = formalism::format::to_string(translated_domain);
            const auto task_text = formalism::format::to_string(translated_task);
            auto reparsed = semantic::Parser(domain_text, configuration.parser_options);
            const auto reparsed_domain = reparsed.get_domain();
            const auto reparsed_task = reparsed.parse_task(task_text);
            EXPECT_EQ(reparsed_domain.get_name(), translated_domain.get_name());
            EXPECT_EQ(reparsed_task.get_name(), translated_task.get_name());
            // Reparsing re-interns the built-in object and number types that typing compilation removed.
            EXPECT_EQ(reparsed_domain.get_types().size(), configuration.options.compile_typing ? 2 : configuration.domain_types);
            EXPECT_EQ(reparsed_domain.get_predicates().size(), configuration.domain_predicates);
            EXPECT_EQ(reparsed_domain.get_functions().size(), configuration.domain_functions);
            EXPECT_EQ(reparsed_domain.get_actions().size(), configuration.domain_actions);
            EXPECT_EQ(reparsed_domain.get_axioms().size(), configuration.domain_axioms);
            EXPECT_EQ(reparsed_task.get_objects().size(), configuration.task_objects);
            // Problem PDDL has no :predicates section; generated goal predicates are re-declared
            // into the domain symbol table when the reparser reads the task axioms.
            EXPECT_TRUE(reparsed_task.get_predicates().empty());
            EXPECT_EQ(reparsed_task.get_initial_literals().size(), configuration.task_initial_literals);
            EXPECT_EQ(reparsed_task.get_initial_function_values().size(), configuration.task_initial_function_values);
            EXPECT_EQ(reparsed_task.get_domain().get_actions().size(), configuration.task_actions);
            EXPECT_EQ(reparsed_task.get_axioms().size(), configuration.task_axioms);

            // Retranslating the reparsed output must be a fixpoint: same counts as the first translation.
            const auto retranslation = semantic::translate(reparsed_domain, configuration.options);
            const auto retranslated_domain = retranslation.get_translated_domain();
            const auto task_retranslation = semantic::translate(reparsed_task, retranslation, configuration.options);
            const auto retranslated_task = task_retranslation.get_translated_task();

            EXPECT_EQ(retranslated_domain.get_types().size(), configuration.domain_types);
            EXPECT_EQ(retranslated_domain.get_predicates().size(), configuration.domain_predicates);
            EXPECT_EQ(retranslated_domain.get_functions().size(), configuration.domain_functions);
            EXPECT_EQ(retranslated_domain.get_actions().size(), configuration.domain_actions);
            EXPECT_EQ(retranslated_domain.get_axioms().size(), configuration.domain_axioms);
            EXPECT_EQ(retranslated_task.get_objects().size(), configuration.task_objects);
            // Goal simplification never axiomatizes literals or conjunctions of literals; after the
            // round-trip every goal is in that form, so no goal predicates are regenerated.
            EXPECT_TRUE(retranslated_task.get_predicates().empty());
            EXPECT_EQ(retranslated_task.get_initial_literals().size(), configuration.task_initial_literals);
            EXPECT_EQ(retranslated_task.get_initial_function_values().size(), configuration.task_initial_function_values);
            EXPECT_EQ(retranslated_task.get_domain().get_actions().size(), configuration.task_actions);
            EXPECT_EQ(retranslated_task.get_axioms().size(), configuration.task_axioms);

            // Text stability: retranslation must reproduce the exact string representations.
            EXPECT_TRUE(formalism::format::to_string(retranslated_domain) == domain_text) << "domain text not stable under retranslation";
            EXPECT_TRUE(formalism::format::to_string(retranslated_task) == task_text) << "task text not stable under retranslation";
        }
    }
}

}  // namespace loki::tests
