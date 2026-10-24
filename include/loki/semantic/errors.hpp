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

#ifndef LOKI_SEMANTIC_ERRORS_HPP_
#define LOKI_SEMANTIC_ERRORS_HPP_

#include <stdexcept>
#include <string>
#include <utility>
#include <yggdrasil/diagnostics/diagnostic.hpp>

#if defined(__clang__) || defined(__GNUC__)
#define LOKI_SEMANTIC_ERROR_API __attribute__((visibility("default")))
#else
#define LOKI_SEMANTIC_ERROR_API
#endif

namespace loki::semantic
{

class LOKI_SEMANTIC_ERROR_API SemanticError : public std::runtime_error
{
public:
    explicit SemanticError(std::string message) : std::runtime_error(message), m_diagnostic { std::move(message) }, m_display_message(m_diagnostic.message) {}

    [[nodiscard]] const char* what() const noexcept override { return m_display_message.c_str(); }
    [[nodiscard]] const std::string& message() const noexcept { return m_diagnostic.message; }
    [[nodiscard]] const ygg::diagnostics::Diagnostic& diagnostic() const noexcept { return m_diagnostic; }

    void set_diagnostic(ygg::diagnostics::Diagnostic diagnostic)
    {
        m_diagnostic = std::move(diagnostic);
        m_display_message = ygg::diagnostics::format_diagnostic(m_diagnostic);
    }

    void set_display_message(std::string display_message) { m_display_message = std::move(display_message); }

private:
    ygg::diagnostics::Diagnostic m_diagnostic;
    std::string m_display_message;
};

class LOKI_SEMANTIC_ERROR_API ParseError : public SemanticError
{
public:
    explicit ParseError(std::string message) : SemanticError(std::move(message)) {}
};

class LOKI_SEMANTIC_ERROR_API UnsupportedRequirementError : public SemanticError
{
public:
    explicit UnsupportedRequirementError(const std::string& requirement) : SemanticError("Unsupported requirement: :" + requirement) {}
};

class LOKI_SEMANTIC_ERROR_API MissingRequirementError : public SemanticError
{
public:
    explicit MissingRequirementError(const std::string& requirement) : SemanticError("Missing required PDDL requirement: :" + requirement) {}
};

class LOKI_SEMANTIC_ERROR_API UnusedRequirementError : public SemanticError
{
public:
    explicit UnusedRequirementError(const std::string& requirement) : SemanticError("Unused PDDL requirement: :" + requirement) {}
};

class LOKI_SEMANTIC_ERROR_API AggregateRequirementError : public SemanticError
{
public:
    explicit AggregateRequirementError(const std::string& requirement) :
        SemanticError("Aggregate PDDL requirement must be declared atomically: :" + requirement)
    {
    }
};

class LOKI_SEMANTIC_ERROR_API RedundantRequirementError : public SemanticError
{
public:
    explicit RedundantRequirementError(const std::string& requirement) : SemanticError("Task requirement already declared by the domain: :" + requirement) {}
};

class LOKI_SEMANTIC_ERROR_API UndefinedTypeError : public SemanticError
{
public:
    explicit UndefinedTypeError(const std::string& name) : SemanticError("Undefined type: " + name) {}
};

class LOKI_SEMANTIC_ERROR_API UndefinedPredicateError : public SemanticError
{
public:
    explicit UndefinedPredicateError(const std::string& name) : SemanticError("Undefined predicate: " + name) {}
};

class LOKI_SEMANTIC_ERROR_API UndefinedObjectError : public SemanticError
{
public:
    explicit UndefinedObjectError(const std::string& name) : SemanticError("Undefined object: " + name) {}
};

class LOKI_SEMANTIC_ERROR_API UndefinedVariableError : public SemanticError
{
public:
    explicit UndefinedVariableError(const std::string& name) : SemanticError("Undefined variable: " + name) {}
};

class LOKI_SEMANTIC_ERROR_API UndefinedFunctionError : public SemanticError
{
public:
    explicit UndefinedFunctionError(const std::string& name) : SemanticError("Undefined function: " + name) {}
};

class LOKI_SEMANTIC_ERROR_API DuplicateDefinitionError : public SemanticError
{
protected:
    DuplicateDefinitionError(const std::string& kind, const std::string& name) : SemanticError("Duplicate " + kind + " definition: " + name) {}
};

class LOKI_SEMANTIC_ERROR_API DuplicateTypeError : public DuplicateDefinitionError
{
public:
    explicit DuplicateTypeError(const std::string& name) : DuplicateDefinitionError("type", name) {}
};

class LOKI_SEMANTIC_ERROR_API DuplicatePredicateError : public DuplicateDefinitionError
{
public:
    explicit DuplicatePredicateError(const std::string& name) : DuplicateDefinitionError("predicate", name) {}
};

class LOKI_SEMANTIC_ERROR_API DuplicateObjectError : public DuplicateDefinitionError
{
public:
    explicit DuplicateObjectError(const std::string& name) : DuplicateDefinitionError("object", name) {}
};

class LOKI_SEMANTIC_ERROR_API DuplicateVariableError : public DuplicateDefinitionError
{
public:
    explicit DuplicateVariableError(const std::string& name) : DuplicateDefinitionError("variable", name) {}
};

class LOKI_SEMANTIC_ERROR_API DuplicateFunctionError : public DuplicateDefinitionError
{
public:
    explicit DuplicateFunctionError(const std::string& name) : DuplicateDefinitionError("function", name) {}
};

class LOKI_SEMANTIC_ERROR_API ArityMismatchError : public SemanticError
{
public:
    ArityMismatchError(const std::string& name, std::size_t expected, std::size_t actual) :
        SemanticError("Arity mismatch for " + name + ": expected " + std::to_string(expected) + ", got " + std::to_string(actual))
    {
    }
};

class LOKI_SEMANTIC_ERROR_API TypeMismatchError : public SemanticError
{
public:
    explicit TypeMismatchError(const std::string& name) : SemanticError("Type mismatch for argument of " + name) {}
};

class LOKI_SEMANTIC_ERROR_API InvalidMetricError : public SemanticError
{
public:
    explicit InvalidMetricError(const std::string& optimization) : SemanticError("Invalid metric optimization: " + optimization) {}
};

class LOKI_SEMANTIC_ERROR_API InvalidNumericConstraintError : public SemanticError
{
public:
    explicit InvalidNumericConstraintError(const std::string& op) : SemanticError("Invalid numeric constraint comparator: " + op) {}
};

class LOKI_SEMANTIC_ERROR_API InvalidNumericEffectError : public SemanticError
{
public:
    explicit InvalidNumericEffectError(const std::string& op) : SemanticError("Invalid numeric effect operator: " + op) {}
};

class LOKI_SEMANTIC_ERROR_API InvalidProbabilisticEffectError : public SemanticError
{
public:
    explicit InvalidProbabilisticEffectError(const std::string& message) : SemanticError("Invalid probabilistic effect: " + message) {}
};

class LOKI_SEMANTIC_ERROR_API InvalidEqualityError : public SemanticError
{
public:
    explicit InvalidEqualityError(const std::string& message) : SemanticError("Invalid equality expression: " + message) {}
};

class LOKI_SEMANTIC_ERROR_API MismatchedDomainError : public SemanticError
{
public:
    MismatchedDomainError(const std::string& expected, const std::string& actual) :
        SemanticError("Task references domain '" + actual + "' but parser holds domain '" + expected + "'.")
    {
    }
};

class LOKI_SEMANTIC_ERROR_API MissingDomainError : public SemanticError
{
public:
    explicit MissingDomainError(std::string message) : SemanticError(std::move(message)) {}
};

}  // namespace loki::semantic

#undef LOKI_SEMANTIC_ERROR_API

#endif
