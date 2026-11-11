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
#include <yggdrasil/diagnostics/errors.hpp>

#if defined(__clang__) || defined(__GNUC__)
#define LOKI_SEMANTIC_ERROR_API __attribute__((visibility("default")))
#else
#define LOKI_SEMANTIC_ERROR_API
#endif

namespace loki::semantic
{

class LOKI_SEMANTIC_ERROR_API UnsupportedRequirementError : public ygg::diagnostics::SemanticError
{
public:
    explicit UnsupportedRequirementError(const std::string& requirement) : ygg::diagnostics::SemanticError("Unsupported requirement: :" + requirement) {}
};

class LOKI_SEMANTIC_ERROR_API MissingRequirementError : public ygg::diagnostics::SemanticError
{
public:
    explicit MissingRequirementError(const std::string& requirement) : ygg::diagnostics::SemanticError("Missing required PDDL requirement: :" + requirement) {}
};

class LOKI_SEMANTIC_ERROR_API UnusedRequirementError : public ygg::diagnostics::SemanticError
{
public:
    explicit UnusedRequirementError(const std::string& requirement) : ygg::diagnostics::SemanticError("Unused PDDL requirement: :" + requirement) {}
};

class LOKI_SEMANTIC_ERROR_API AggregateRequirementError : public ygg::diagnostics::SemanticError
{
public:
    explicit AggregateRequirementError(const std::string& requirement) :
        ygg::diagnostics::SemanticError("Aggregate PDDL requirement must be declared atomically: :" + requirement)
    {
    }
};

class LOKI_SEMANTIC_ERROR_API RedundantRequirementError : public ygg::diagnostics::SemanticError
{
public:
    explicit RedundantRequirementError(const std::string& requirement) :
        ygg::diagnostics::SemanticError("Task requirement already declared by the domain: :" + requirement)
    {
    }
};

class LOKI_SEMANTIC_ERROR_API UndefinedTypeError : public ygg::diagnostics::SemanticError
{
public:
    explicit UndefinedTypeError(const std::string& name) : ygg::diagnostics::SemanticError("Undefined type: " + name) {}
};

class LOKI_SEMANTIC_ERROR_API UndefinedPredicateError : public ygg::diagnostics::SemanticError
{
public:
    explicit UndefinedPredicateError(const std::string& name) : ygg::diagnostics::SemanticError("Undefined predicate: " + name) {}
};

class LOKI_SEMANTIC_ERROR_API UndefinedObjectError : public ygg::diagnostics::SemanticError
{
public:
    explicit UndefinedObjectError(const std::string& name) : ygg::diagnostics::SemanticError("Undefined object: " + name) {}
};

class LOKI_SEMANTIC_ERROR_API UndefinedVariableError : public ygg::diagnostics::SemanticError
{
public:
    explicit UndefinedVariableError(const std::string& name) : ygg::diagnostics::SemanticError("Undefined variable: " + name) {}
};

class LOKI_SEMANTIC_ERROR_API UndefinedFunctionError : public ygg::diagnostics::SemanticError
{
public:
    explicit UndefinedFunctionError(const std::string& name) : ygg::diagnostics::SemanticError("Undefined function: " + name) {}
};

class LOKI_SEMANTIC_ERROR_API DuplicateDefinitionError : public ygg::diagnostics::SemanticError
{
protected:
    DuplicateDefinitionError(const std::string& kind, const std::string& name) : ygg::diagnostics::SemanticError("Duplicate " + kind + " definition: " + name)
    {
    }
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

class LOKI_SEMANTIC_ERROR_API ArityMismatchError : public ygg::diagnostics::SemanticError
{
public:
    ArityMismatchError(const std::string& name, std::size_t expected, std::size_t actual) :
        ygg::diagnostics::SemanticError("Arity mismatch for " + name + ": expected " + std::to_string(expected) + ", got " + std::to_string(actual))
    {
    }
};

class LOKI_SEMANTIC_ERROR_API TypeMismatchError : public ygg::diagnostics::SemanticError
{
public:
    explicit TypeMismatchError(const std::string& name) : ygg::diagnostics::SemanticError("Type mismatch for argument of " + name) {}
};

class LOKI_SEMANTIC_ERROR_API InvalidMetricError : public ygg::diagnostics::SemanticError
{
public:
    explicit InvalidMetricError(const std::string& optimization) : ygg::diagnostics::SemanticError("Invalid metric optimization: " + optimization) {}
};

class LOKI_SEMANTIC_ERROR_API InvalidNumericConstraintError : public ygg::diagnostics::SemanticError
{
public:
    explicit InvalidNumericConstraintError(const std::string& op) : ygg::diagnostics::SemanticError("Invalid numeric constraint comparator: " + op) {}
};

class LOKI_SEMANTIC_ERROR_API InvalidNumericEffectError : public ygg::diagnostics::SemanticError
{
public:
    explicit InvalidNumericEffectError(const std::string& op) : ygg::diagnostics::SemanticError("Invalid numeric effect operator: " + op) {}
};

class LOKI_SEMANTIC_ERROR_API InvalidProbabilisticEffectError : public ygg::diagnostics::SemanticError
{
public:
    explicit InvalidProbabilisticEffectError(const std::string& message) : ygg::diagnostics::SemanticError("Invalid probabilistic effect: " + message) {}
};

class LOKI_SEMANTIC_ERROR_API InvalidEqualityError : public ygg::diagnostics::SemanticError
{
public:
    explicit InvalidEqualityError(const std::string& message) : ygg::diagnostics::SemanticError("Invalid equality expression: " + message) {}
};

class LOKI_SEMANTIC_ERROR_API MismatchedDomainError : public ygg::diagnostics::SemanticError
{
public:
    MismatchedDomainError(const std::string& expected, const std::string& actual) :
        ygg::diagnostics::SemanticError("Task references domain '" + actual + "' but parser holds domain '" + expected + "'.")
    {
    }
};

class LOKI_SEMANTIC_ERROR_API MissingDomainError : public ygg::diagnostics::SemanticError
{
public:
    explicit MissingDomainError(std::string message) : ygg::diagnostics::SemanticError(std::move(message)) {}
};

}  // namespace loki::semantic

#undef LOKI_SEMANTIC_ERROR_API

#endif
