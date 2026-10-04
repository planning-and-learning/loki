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

#ifndef PYPDDL_FORMALISM_BINDINGS_HPP_
#define PYPDDL_FORMALISM_BINDINGS_HPP_

#include <loki/formalism/formatter.hpp>
#include <loki/formalism/repository.hpp>
#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/owner.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace loki::formalism
{

using RepositoryBinding = nanobind::class_<Repository>;

template<typename T>
using Data = ygg::Data<T>;

template<typename T>
void bind_insert(RepositoryBinding& repository)
{
    namespace nb = nanobind;
    const auto retain_owner = ygg::python::make_owner_retainer();
    repository.def(
        "insert",
        [retain_owner](nb::typed<nb::handle, Repository> owner, Data<T> data) -> nb::typed<nb::tuple, EntityView<T>, bool>
        {
            return nb::borrow<nb::typed<nb::tuple, EntityView<T>, bool>>(
                ygg::python::cast_with_owner(formalism::insert(nb::cast<Repository&>(owner), data), owner, retain_owner));
        },
        nb::arg("data"));
}

template<typename V>
auto bind_data(nanobind::module_& m, const char* name)
{
    auto cls = nanobind::class_<V>(m, name);
    ygg::add_comparison(cls);
    return cls;
}

void bind_formalism(nanobind::module_& m);

void bind_requirement(nanobind::module_& m, RepositoryBinding& repository);
void bind_type(nanobind::module_& m, RepositoryBinding& repository);
void bind_object(nanobind::module_& m, RepositoryBinding& repository);
void bind_variable(nanobind::module_& m, RepositoryBinding& repository);
void bind_parameter(nanobind::module_& m, RepositoryBinding& repository);
void bind_predicate(nanobind::module_& m, RepositoryBinding& repository);
void bind_function_skeleton(nanobind::module_& m, RepositoryBinding& repository);
void bind_term(nanobind::module_& m, RepositoryBinding& repository);
void bind_atom(nanobind::module_& m, RepositoryBinding& repository);
void bind_literal(nanobind::module_& m, RepositoryBinding& repository);
void bind_function_expression_number(nanobind::module_& m, RepositoryBinding& repository);
void bind_function_term(nanobind::module_& m, RepositoryBinding& repository);
void bind_unary_function_expression(nanobind::module_& m, RepositoryBinding& repository);
void bind_binary_function_expression(nanobind::module_& m, RepositoryBinding& repository);
void bind_multi_function_expression(nanobind::module_& m, RepositoryBinding& repository);
void bind_function_expression(nanobind::module_& m, RepositoryBinding& repository);
void bind_condition_literal(nanobind::module_& m, RepositoryBinding& repository);
void bind_condition_and(nanobind::module_& m, RepositoryBinding& repository);
void bind_condition_or(nanobind::module_& m, RepositoryBinding& repository);
void bind_condition_not(nanobind::module_& m, RepositoryBinding& repository);
void bind_condition_imply(nanobind::module_& m, RepositoryBinding& repository);
void bind_condition_exists(nanobind::module_& m, RepositoryBinding& repository);
void bind_condition_forall(nanobind::module_& m, RepositoryBinding& repository);
void bind_condition_numeric_constraint(nanobind::module_& m, RepositoryBinding& repository);
void bind_condition(nanobind::module_& m, RepositoryBinding& repository);
void bind_effect_literal(nanobind::module_& m, RepositoryBinding& repository);
void bind_effect_and(nanobind::module_& m, RepositoryBinding& repository);
void bind_effect_numeric(nanobind::module_& m, RepositoryBinding& repository);
void bind_effect_forall(nanobind::module_& m, RepositoryBinding& repository);
void bind_effect_when(nanobind::module_& m, RepositoryBinding& repository);
void bind_effect_one_of(nanobind::module_& m, RepositoryBinding& repository);
void bind_effect_probabilistic_alternative(nanobind::module_& m, RepositoryBinding& repository);
void bind_effect_probabilistic(nanobind::module_& m, RepositoryBinding& repository);
void bind_effect(nanobind::module_& m, RepositoryBinding& repository);
void bind_action(nanobind::module_& m, RepositoryBinding& repository);
void bind_axiom(nanobind::module_& m, RepositoryBinding& repository);
void bind_metric(nanobind::module_& m, RepositoryBinding& repository);
void bind_initial_function_value(nanobind::module_& m, RepositoryBinding& repository);
void bind_domain(nanobind::module_& m, RepositoryBinding& repository);
void bind_task(nanobind::module_& m, RepositoryBinding& repository);

}  // namespace loki::formalism

#endif
