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

#ifndef LOKI_SEMANTIC_TRANSLATOR_INFER_REQUIREMENTS_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_INFER_REQUIREMENTS_HPP_

#include "loki/formalism/repository.hpp"

#include <vector>

namespace loki::semantic::detail
{

/// The atomic requirements that the strict parser demands for the domain, sorted: exactly those its constructs use, plus :strips.
std::vector<formalism::RequirementKind> infer_requirements(formalism::DomainView domain);

/// The atomic requirements that the strict parser demands for the task beyond those of `domain`, sorted.
std::vector<formalism::RequirementKind> infer_requirements(formalism::TaskView task, formalism::DomainView domain);

}  // namespace loki::semantic::detail

#endif
