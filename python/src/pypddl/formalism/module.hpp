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
// The type casters of every TU of this module (each TU binding Stores includes this header): one set, so that all
// TUs see the same casters.
#include <urd/python/flat.hpp>
#include <urd/python/maps/flat_map.hpp>
#include <urd/python/maps/index_map.hpp>
#include <urd/python/maps/ordered_map.hpp>
#include <urd/python/maps/parallel_hash_map.hpp>
#include <urd/python/maps/unordered_flat_map.hpp>
#include <urd/python/ranges.hpp>
#include <urd/python/sequences/segmented.hpp>
#include <urd/python/sequences/sequence.hpp>
#include <urd/python/values/dense_set.hpp>
#include <urd/python/values/flat_set.hpp>
#include <urd/python/values/indexed.hpp>
#include <urd/python/values/parallel_hash_set.hpp>
#include <urd/python/values/sparse_set.hpp>
#include <urd/python/values/unordered_flat_set.hpp>
#include <yggdrasil/python/type_casters/closed_interval.hpp>

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
