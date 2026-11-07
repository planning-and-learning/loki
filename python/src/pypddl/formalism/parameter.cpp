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

#include "bindings.hpp"
#include "pypddl/formalism/module.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <urd/python/sequences/sequence.hpp>

#include <loki/formalism/parameter_view.hpp>
#include <loki/formalism/type_view.hpp>
#include <loki/formalism/variable_view.hpp>

namespace nb = nanobind;

namespace loki::formalism
{

void bind_parameter(nb::module_& m, RepositoryBinding& repository)
{
    using namespace nb::literals;
    urd::python::bind_index<urd::index<formalism::Parameter>>(m, "ParameterIndex");

    {
        using V = urd::data<formalism::Parameter>;
        urd::python::bind_data<V>(m, "ParameterData")
            .def(nb::init<urd::index<formalism::Variable>, urd::flat_vector<urd::index<formalism::Type>>>(), "variable"_a, "types"_a = urd::flat_vector<urd::index<formalism::Type>> {})
            .def(urd::python::init_from_views<formalism::VariableView, const urd::Store<urd::sequence<formalism::TypeView>>&>(), "variable"_a, "types"_a = urd::Store<urd::sequence<formalism::TypeView>> {})
            .def_rw("variable", &V::variable)
            .def_rw("types", &V::types);
    }

    {
        using V = formalism::ParameterView;
        auto cls = nb::class_<V>(m, "Parameter");
        cls.def("get_index", &V::index).def("get_variable", &V::get_variable, nb::keep_alive<0, 1>()).def("get_types", &V::get_types);
        ygg::add_print(cls);
        urd::python::add_comparison(cls);
        urd::python::add_hash(cls);
    }

    bind_insert<formalism::Parameter>(repository);
}

}  // namespace loki::formalism
