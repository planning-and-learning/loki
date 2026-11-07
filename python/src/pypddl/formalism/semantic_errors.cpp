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

#include "module.hpp"

#include <loki/semantic/errors.hpp>
#include <yggdrasil/python/diagnostics.hpp>

namespace nb = nanobind;

namespace loki::formalism
{

void bind_semantic_errors(nb::module_& m)
{
    auto semantic_error = ygg::bind_diagnostic_exception<semantic::SemanticError>(m, "SemanticError");

    ygg::bind_diagnostic_exception<semantic::ParseError>(m, "ParseError", semantic_error.ptr());
    ygg::bind_diagnostic_exception<semantic::MissingDomainError>(m, "MissingDomainError", semantic_error.ptr());
    ygg::bind_diagnostic_exception<semantic::MismatchedDomainError>(m, "MismatchedDomainError", semantic_error.ptr());

    ygg::bind_diagnostic_exception<semantic::UnsupportedRequirementError>(m, "UnsupportedRequirementError", semantic_error.ptr());
    ygg::bind_diagnostic_exception<semantic::MissingRequirementError>(m, "MissingRequirementError", semantic_error.ptr());
    ygg::bind_diagnostic_exception<semantic::UnusedRequirementError>(m, "UnusedRequirementError", semantic_error.ptr());
    ygg::bind_diagnostic_exception<semantic::AggregateRequirementError>(m, "AggregateRequirementError", semantic_error.ptr());
    ygg::bind_diagnostic_exception<semantic::RedundantRequirementError>(m, "RedundantRequirementError", semantic_error.ptr());

    ygg::bind_diagnostic_exception<semantic::UndefinedTypeError>(m, "UndefinedTypeError", semantic_error.ptr());
    ygg::bind_diagnostic_exception<semantic::UndefinedPredicateError>(m, "UndefinedPredicateError", semantic_error.ptr());
    ygg::bind_diagnostic_exception<semantic::UndefinedObjectError>(m, "UndefinedObjectError", semantic_error.ptr());
    ygg::bind_diagnostic_exception<semantic::UndefinedVariableError>(m, "UndefinedVariableError", semantic_error.ptr());
    ygg::bind_diagnostic_exception<semantic::UndefinedFunctionError>(m, "UndefinedFunctionError", semantic_error.ptr());

    auto duplicate_error = ygg::bind_diagnostic_exception<semantic::DuplicateDefinitionError>(m, "DuplicateDefinitionError", semantic_error.ptr());
    ygg::bind_diagnostic_exception<semantic::DuplicateTypeError>(m, "DuplicateTypeError", duplicate_error.ptr());
    ygg::bind_diagnostic_exception<semantic::DuplicatePredicateError>(m, "DuplicatePredicateError", duplicate_error.ptr());
    ygg::bind_diagnostic_exception<semantic::DuplicateObjectError>(m, "DuplicateObjectError", duplicate_error.ptr());
    ygg::bind_diagnostic_exception<semantic::DuplicateVariableError>(m, "DuplicateVariableError", duplicate_error.ptr());
    ygg::bind_diagnostic_exception<semantic::DuplicateFunctionError>(m, "DuplicateFunctionError", duplicate_error.ptr());

    ygg::bind_diagnostic_exception<semantic::ArityMismatchError>(m, "ArityMismatchError", semantic_error.ptr());
    ygg::bind_diagnostic_exception<semantic::TypeMismatchError>(m, "TypeMismatchError", semantic_error.ptr());

    ygg::bind_diagnostic_exception<semantic::InvalidMetricError>(m, "InvalidMetricError", semantic_error.ptr());
    ygg::bind_diagnostic_exception<semantic::InvalidNumericConstraintError>(m, "InvalidNumericConstraintError", semantic_error.ptr());
    ygg::bind_diagnostic_exception<semantic::InvalidNumericEffectError>(m, "InvalidNumericEffectError", semantic_error.ptr());
    ygg::bind_diagnostic_exception<semantic::InvalidProbabilisticEffectError>(m, "InvalidProbabilisticEffectError", semantic_error.ptr());
    ygg::bind_diagnostic_exception<semantic::InvalidEqualityError>(m, "InvalidEqualityError", semantic_error.ptr());
}

}  // namespace loki::formalism
