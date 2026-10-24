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

#ifndef LOKI_SEMANTIC_MAPPINGS_HPP_
#define LOKI_SEMANTIC_MAPPINGS_HPP_

#include "loki/formalism/enums.hpp"

#include <cista/containers/string.h>
#include <string>
#include <vector>

namespace loki::semantic
{

// Lowercase an owned PDDL source buffer before parsing.
std::string lowercase(std::string text);
// Semantic keys strip the requirement marker after source normalization.
std::string key(std::string text);
cista::offset::string to_cista(const std::string& text);

// PDDL name <-> enum mappings.
std::vector<formalism::RequirementKind> requirement_capabilities(formalism::RequirementKind kind);
std::string requirement_name(formalism::RequirementKind kind);
formalism::BinaryArithmeticOperator binary_operator(std::string op);
formalism::MultiArithmeticOperator multi_operator(std::string op);

}  // namespace loki::semantic

#endif
