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

#ifndef LOKI_TESTS_BENCHMARK_UTILS_HPP_
#define LOKI_TESTS_BENCHMARK_UTILS_HPP_

#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <yggdrasil/serialization/json_suite.hpp>

namespace loki::tests
{

struct SuiteCase
{
    std::string name;
    std::filesystem::path domain_file;
    std::optional<std::filesystem::path> task_file;
};

inline std::vector<SuiteCase> load_suite_cases(const std::filesystem::path& suite_json, const std::optional<std::filesystem::path>& prefix = std::nullopt)
{
    const auto suite_value = ygg::common::load_json_file(suite_json);
    const auto& suite = ygg::common::as_object(suite_value, "suite");
    const auto base = prefix ? *prefix : ygg::common::suite_prefix_path(suite);
    auto result = std::vector<SuiteCase> {};
    for (const auto& case_value : ygg::common::as_array(suite, "cases", "suite"))
    {
        const auto& object = ygg::common::as_object(case_value, "case");
        auto item = SuiteCase { ygg::common::as_string(object, "name", "case"),
                                ygg::common::resolve_path(base, ygg::common::as_string(object, "domain_file", "case")),
                                std::nullopt };
        if (object.if_contains("task_file"))
            item.task_file = ygg::common::resolve_path(base, ygg::common::as_string(object, "task_file", "case"));
        result.push_back(std::move(item));
    }
    return result;
}

inline std::vector<SuiteCase> benchmark_suite_cases()
{ return load_suite_cases(ygg::common::root_path() / "tests/fixtures/parser_counts_suite.json", std::filesystem::path(BENCHMARKS_DIR)); }

inline std::vector<SuiteCase> fixture_suite_cases() { return load_suite_cases(ygg::common::root_path() / "tests/fixtures/suite.json"); }

// Benchmark cases plus the loki mini-corpus: the exhaustive input set for positive invariants.
inline std::vector<SuiteCase> all_positive_cases()
{
    auto result = benchmark_suite_cases();
    auto fixtures = fixture_suite_cases();
    result.insert(result.end(), fixtures.begin(), fixtures.end());
    return result;
}

inline std::filesystem::path fixture_path(const std::string& case_dir, const std::string& file = "domain.pddl")
{ return ygg::common::root_path() / "tests/fixtures" / case_dir / file; }

inline std::string read_text(const std::filesystem::path& path)
{
    auto in = std::ifstream(path);
    if (!in)
        throw std::runtime_error("Failed to open test fixture: " + path.string());
    auto out = std::ostringstream {};
    out << in.rdbuf();
    return out.str();
}

}  // namespace loki::tests

#endif
