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

#include <argparse/argparse.hpp>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <loki/formalism/formatter.hpp>
#include <loki/semantic/options.hpp>
#include <loki/semantic/parser.hpp>
#include <loki/semantic/translator.hpp>
#include <stdexcept>
#include <string>
#include <string_view>

#ifndef LOKI_VERSION
#define LOKI_VERSION "unknown"
#endif

static constexpr auto version = LOKI_VERSION;

static void add_version(argparse::ArgumentParser& program)
{
    program.add_argument("-V", "--version")
        .nargs(0)
        .action(
            [&](const auto&)
            {
                std::cout << version << std::endl;
                std::exit(0);
            })
        .help("Print version information and exit.");
}

static void write_file(const std::filesystem::path& path, std::string_view text)
{
    auto out = std::ofstream(path);
    if (!out)
        throw std::runtime_error("Could not open output file: " + path.string());

    out << text;
    if (!out)
        throw std::runtime_error("Could not write output file: " + path.string());
}

static int run(const argparse::ArgumentParser& program)
{
    const auto domain_filepath = std::filesystem::path(program.get<std::string>("domain"));
    const auto problem_filepath = std::filesystem::path(program.get<std::string>("problem"));
    const auto verbose = program.get<bool>("--verbose");

    if (problem_filepath.empty() && program.is_used("--out-problem"))
        throw std::runtime_error("--out-problem requires a problem file.");

    auto parser_options = loki::semantic::ParserOptions();
    parser_options.strict = program.get<bool>("--strict");
    parser_options.add_action_costs = program.get<bool>("--add-action-costs");

    auto translator_options = loki::semantic::TranslatorOptions();
    translator_options.compile_typing = program.get<bool>("--compile-typing");
    translator_options.compile_conditional_effects = program.get<bool>("--compile-conditional-effects");
    translator_options.materialize_equality = program.get<bool>("--materialize-equality");
    translator_options.normalize_arithmetic_expressions = program.get<bool>("--normalize-arithmetic-expressions");

    auto parser = loki::semantic::Parser(domain_filepath, parser_options);
    const auto domain = parser.get_domain();

    const auto domain_translation_result = loki::semantic::translate(domain, translator_options);
    const auto translated_domain_text = loki::formalism::format::to_string(domain_translation_result.get_translated_domain());
    if (verbose)
        std::cout << translated_domain_text << std::endl;

    if (program.is_used("--out-domain"))
        write_file(program.get<std::string>("--out-domain"), translated_domain_text);

    if (!problem_filepath.empty())
    {
        auto task = parser.parse_task(problem_filepath);

        const auto translated_task_result = loki::semantic::translate(task, domain_translation_result, translator_options);
        const auto translated_task_text = loki::formalism::format::to_string(translated_task_result.get_translated_task());
        if (verbose)
            std::cout << translated_task_text << std::endl;

        if (program.is_used("--out-problem"))
            write_file(program.get<std::string>("--out-problem"), translated_task_text);
    }

    return 0;
}

int main(int argc, char** argv)
{
    auto program = argparse::ArgumentParser("loki", version, argparse::default_arguments::help);
    add_version(program);

    program.add_argument("domain").required().help("The path to the PDDL domain file.");
    program.add_argument("problem").default_value(std::string {}).help("The path to the PDDL problem file.");
    program.add_argument("-d", "--out-domain").help("The path to the output PDDL domain file.");
    program.add_argument("-p", "--out-problem").help("The path to the output PDDL problem file.");
    program.add_argument("-s", "--strict")
        .default_value(false)
        .implicit_value(true)
        .help("Enable strict semantic checks for requirements, arity, and type compatibility.");
    program.add_argument("-v", "--verbose").default_value(false).implicit_value(true).help("Enable verbose console prints.");
    program.add_argument("--add-action-costs")
        .default_value(false)
        .implicit_value(true)
        .help("Complete missing :action-costs artifacts; without :action-costs, inject the requirement, total-cost, and unit-cost effects.");
    program.add_argument("--compile-typing")
        .default_value(false)
        .implicit_value(true)
        .help("Compile typing away into type predicates and remove type annotations.");
    program.add_argument("--compile-conditional-effects")
        .default_value(false)
        .implicit_value(true)
        .help("Split actions to eliminate top-level conditional effects after effect normalization.");
    program.add_argument("--materialize-equality")
        .default_value(false)
        .implicit_value(true)
        .help("Add the = predicate and (= o o) initial literals for consumers without native equality handling.");
    program.add_argument("--normalize-arithmetic-expressions")
        .default_value(false)
        .implicit_value(true)
        .help("Normalize addition and multiplication modulo associativity, commutativity, and identity elements.");

    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err)
    {
        std::cerr << err.what() << "\n";
        std::cerr << program;
        return 1;
    }

    try
    {
        return run(program);
    }
    catch (const std::exception& err)
    {
        std::cerr << err.what() << "\n";
        return 1;
    }
}
