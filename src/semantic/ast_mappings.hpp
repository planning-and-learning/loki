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

#ifndef LOKI_SEMANTIC_AST_MAPPINGS_HPP_
#define LOKI_SEMANTIC_AST_MAPPINGS_HPP_

#include "loki/ast/ast_fwd.hpp"
#include "loki/formalism/enums.hpp"

#include <vector>

namespace loki::semantic
{

struct DiagnosticContext;

// PDDL names in the AST <-> enum mappings, with diagnostics for unknown names.
formalism::RequirementKind requirement_kind(const ast::Requirement& node, const DiagnosticContext& diagnostics);
std::vector<formalism::RequirementKind> requirement_capabilities(const ast::Requirement& node, const DiagnosticContext& diagnostics);
formalism::BinaryComparator comparator(const ast::ConditionNumericConstraint& node, const DiagnosticContext& diagnostics);
formalism::NumericEffectOperator numeric_effect_operator(const ast::EffectNumeric& node, const DiagnosticContext& diagnostics);

}  // namespace loki::semantic

#endif
