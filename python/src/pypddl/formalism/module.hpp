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

#ifndef LOKI_PYTHON_PYPDDL_FORMALISM_MODULE_HPP_
#define LOKI_PYTHON_PYPDDL_FORMALISM_MODULE_HPP_

#include <nanobind/nanobind.h>

namespace nb = nanobind;

namespace loki::formalism
{

void bind_formalism(nb::module_& m);
void bind_semantic_errors(nb::module_& m);
void bind_semantic_parser(nb::module_& m);
void bind_semantic_translator(nb::module_& m);
void bind_module_definitions(nb::module_& m);

}  // namespace loki::formalism

#endif
