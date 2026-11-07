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

#include <loki/formalism/object_view.hpp>
#include <loki/formalism/type_view.hpp>

namespace nb = nanobind;

namespace loki::formalism
{

void bind_object(nb::module_& m, RepositoryBinding& repository)
{
    using namespace nb::literals;
    urd::python::bind_index<urd::index<formalism::Object>>(m, "ObjectIndex");

    {
        using V = urd::data<formalism::Object>;
        urd::python::bind_data<V>(m, "ObjectData")
            .def(nb::init<std::string, urd::flat_vector<urd::index<formalism::Type>>>(), "name"_a, "types"_a = urd::flat_vector<urd::index<formalism::Type>> {})
            .def(urd::python::init_from_views<const std::string&, const urd::Store<urd::sequence<formalism::TypeView>>&>(), "name"_a, "types"_a = urd::Store<urd::sequence<formalism::TypeView>> {})
            .def_rw("name", &V::name)
            .def_rw("types", &V::types);
    }

    {
        using V = formalism::ObjectView;
        auto cls = nb::class_<V>(m, "Object");
        cls.def("get_index", &V::index).def("get_name", &V::get_name).def("get_types", &V::get_types);
        ygg::add_print(cls);
        urd::python::add_comparison(cls);
        urd::python::add_hash(cls);
    }

    bind_insert<formalism::Object>(repository);
}

}  // namespace loki::formalism
