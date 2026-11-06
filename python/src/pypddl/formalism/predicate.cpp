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
#include <yggdrasil/python/type_casters.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <urd/python/sequences/sequence.hpp>

#include <loki/formalism/parameter_view.hpp>
#include <loki/formalism/predicate_view.hpp>

namespace nb = nanobind;

namespace loki::formalism
{

void bind_predicate(nb::module_& m, RepositoryBinding& repository)
{
    using namespace nb::literals;
    urd::python::bind_index<urd::index<formalism::Predicate>>(m, "PredicateIndex");

    {
        using V = urd::data<formalism::Predicate>;
        urd::python::bind_data<V>(m, "PredicateData")
            .def(nb::init<std::string, urd::flat_vector<urd::index<formalism::Parameter>>>(), "name"_a, "parameters"_a = urd::flat_vector<urd::index<formalism::Parameter>> {})
            .def(urd::python::init_from_views<const std::string&, const urd::Store<urd::sequence<formalism::ParameterView>>&>(),
                 "name"_a,
                 "parameters"_a = urd::Store<urd::sequence<formalism::ParameterView>> {})
            .def_rw("name", &V::name)
            .def_rw("parameters", &V::parameters);
    }

    {
        using V = formalism::PredicateView;
        auto cls = nb::class_<V>(m, "Predicate");
        cls.def("get_index", &V::index).def("get_name", &V::get_name).def("get_parameters", &V::get_parameters).def("get_arity", &V::get_arity);
        ygg::add_print(cls);
        urd::python::add_comparison(cls);
        urd::python::add_hash(cls);
    }

    bind_insert<formalism::Predicate>(repository);
}

}  // namespace loki::formalism
