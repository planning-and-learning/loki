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
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

#include <filesystem>
#include <gtest/gtest.h>
#include <loki/formalism/task_view.hpp>
#include <loki/semantic/errors.hpp>
#include <loki/semantic/options.hpp>
#include <loki/semantic/parser.hpp>
#include <optional>
#include <string>
#include <typeinfo>
#include <vector>
#include <yggdrasil/serialization/json_suite.hpp>

namespace loki::tests
{
namespace
{

struct NegativeSuiteCase
{
    std::string name;
    bool strict = false;
    std::filesystem::path domain_file;
    std::filesystem::path task_file;
    std::optional<std::string> expected_error;
    std::optional<std::string> expected_message;
    std::optional<std::size_t> expected_line;
    std::optional<std::size_t> expected_column;
};

NegativeSuiteCase parse_case(const boost::json::object& object, const std::filesystem::path& base)
{
    auto result = NegativeSuiteCase {};
    result.name = ygg::common::as_string(object, "name", "case");
    result.strict = ygg::common::find_bool(object, "strict", "case").value_or(false);
    result.domain_file = ygg::common::resolve_path(base, ygg::common::as_string(object, "domain_file", "case"));
    if (object.if_contains("task_file"))
        result.task_file = ygg::common::resolve_path(base, ygg::common::as_string(object, "task_file", "case"));
    if (const auto* expected_error = object.if_contains("expected_error"))
        result.expected_error = std::string(expected_error->as_string());
    if (const auto* expected_message = object.if_contains("expected_message"))
        result.expected_message = std::string(expected_message->as_string());
    if (const auto* expected_line = object.if_contains("expected_line"))
        result.expected_line = static_cast<std::size_t>(expected_line->as_int64());
    if (const auto* expected_column = object.if_contains("expected_column"))
        result.expected_column = static_cast<std::size_t>(expected_column->as_int64());
    return result;
}

urd::Store<urd::sequence<NegativeSuiteCase>> load_cases()
{
    const auto suite_value = ygg::common::load_json_file(ygg::common::root_path() / "tests/fixtures/negative_suite.json");
    const auto& suite = ygg::common::as_object(suite_value, "suite");
    const auto base = ygg::common::suite_prefix_path(suite);
    auto result = urd::Store<urd::sequence<NegativeSuiteCase>> {};
    for (const auto& case_value : ygg::common::as_array(suite, "cases", "suite"))
        result.push_back(parse_case(ygg::common::as_object(case_value, "case"), base));
    return result;
}

bool matches_error_type(const semantic::SemanticError& error, const std::string& expected_error)
{
    if (expected_error == "SemanticError")
        return true;
    if (expected_error == "ParseError")
        return dynamic_cast<const semantic::ParseError*>(&error) != nullptr;
    if (expected_error == "MissingDomainError")
        return dynamic_cast<const semantic::MissingDomainError*>(&error) != nullptr;
    if (expected_error == "MismatchedDomainError")
        return dynamic_cast<const semantic::MismatchedDomainError*>(&error) != nullptr;
    if (expected_error == "UnsupportedRequirementError")
        return dynamic_cast<const semantic::UnsupportedRequirementError*>(&error) != nullptr;
    if (expected_error == "MissingRequirementError")
        return dynamic_cast<const semantic::MissingRequirementError*>(&error) != nullptr;
    if (expected_error == "UnusedRequirementError")
        return dynamic_cast<const semantic::UnusedRequirementError*>(&error) != nullptr;
    if (expected_error == "AggregateRequirementError")
        return dynamic_cast<const semantic::AggregateRequirementError*>(&error) != nullptr;
    if (expected_error == "RedundantRequirementError")
        return dynamic_cast<const semantic::RedundantRequirementError*>(&error) != nullptr;
    if (expected_error == "UndefinedTypeError")
        return dynamic_cast<const semantic::UndefinedTypeError*>(&error) != nullptr;
    if (expected_error == "UndefinedPredicateError")
        return dynamic_cast<const semantic::UndefinedPredicateError*>(&error) != nullptr;
    if (expected_error == "UndefinedObjectError")
        return dynamic_cast<const semantic::UndefinedObjectError*>(&error) != nullptr;
    if (expected_error == "UndefinedVariableError")
        return dynamic_cast<const semantic::UndefinedVariableError*>(&error) != nullptr;
    if (expected_error == "UndefinedFunctionError")
        return dynamic_cast<const semantic::UndefinedFunctionError*>(&error) != nullptr;
    if (expected_error == "DuplicateDefinitionError")
        return dynamic_cast<const semantic::DuplicateDefinitionError*>(&error) != nullptr;
    if (expected_error == "DuplicateTypeError")
        return dynamic_cast<const semantic::DuplicateTypeError*>(&error) != nullptr;
    if (expected_error == "DuplicatePredicateError")
        return dynamic_cast<const semantic::DuplicatePredicateError*>(&error) != nullptr;
    if (expected_error == "DuplicateObjectError")
        return dynamic_cast<const semantic::DuplicateObjectError*>(&error) != nullptr;
    if (expected_error == "DuplicateVariableError")
        return dynamic_cast<const semantic::DuplicateVariableError*>(&error) != nullptr;
    if (expected_error == "DuplicateFunctionError")
        return dynamic_cast<const semantic::DuplicateFunctionError*>(&error) != nullptr;
    if (expected_error == "ArityMismatchError")
        return dynamic_cast<const semantic::ArityMismatchError*>(&error) != nullptr;
    if (expected_error == "TypeMismatchError")
        return dynamic_cast<const semantic::TypeMismatchError*>(&error) != nullptr;
    if (expected_error == "InvalidMetricError")
        return dynamic_cast<const semantic::InvalidMetricError*>(&error) != nullptr;
    if (expected_error == "InvalidNumericConstraintError")
        return dynamic_cast<const semantic::InvalidNumericConstraintError*>(&error) != nullptr;
    if (expected_error == "InvalidNumericEffectError")
        return dynamic_cast<const semantic::InvalidNumericEffectError*>(&error) != nullptr;
    if (expected_error == "InvalidProbabilisticEffectError")
        return dynamic_cast<const semantic::InvalidProbabilisticEffectError*>(&error) != nullptr;
    if (expected_error == "InvalidEqualityError")
        return dynamic_cast<const semantic::InvalidEqualityError*>(&error) != nullptr;
    ADD_FAILURE() << "Unknown expected_error in negative suite: " << expected_error;
    return false;
}

}  // namespace

TEST(LokiSemanticNegativeSuite, ReportsExpectedSemanticErrors)
{
    for (const auto& item : load_cases())
    {
        SCOPED_TRACE(item.name);
        auto options = semantic::ParserOptions {};
        options.strict = item.strict;
        // Negative cases assert enforcement without action-costs completion.
        options.add_action_costs = false;
        try
        {
            auto parser = semantic::Parser(ygg::common::read_file(item.domain_file), options);
            if (!item.task_file.empty())
                parser.parse_task(ygg::common::read_file(item.task_file));
            FAIL() << "Expected semantic error";
        }
        catch (const semantic::SemanticError& error)
        {
            const auto message = std::string(error.what());
            if (item.expected_error)
            {
                EXPECT_TRUE(matches_error_type(error, *item.expected_error)) << "expected " << *item.expected_error << ", got " << typeid(error).name();
            }
            if (item.expected_message)
            {
                EXPECT_NE(message.find(*item.expected_message), std::string::npos) << message;
            }
            if (item.expected_line)
            {
                EXPECT_NE(message.find("In line " + std::to_string(*item.expected_line) + ":"), std::string::npos) << message;
            }
            if (item.expected_column)
            {
                ASSERT_TRUE(error.diagnostic().location) << message;
                EXPECT_EQ(error.diagnostic().location->column(), *item.expected_column) << message;
            }
        }
    }
}

}  // namespace loki::tests
