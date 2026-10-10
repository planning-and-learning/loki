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

#include <loki/formalism/type_view.hpp>

namespace nb = nanobind;
using namespace nb::literals;

namespace loki::formalism
{

void bind_type(nb::module_& m, RepositoryBinding& repository)
{
    ygg::bind_index<ygg::Index<formalism::Type>>(m, "TypeIndex");

    {
        using V = Data<formalism::Type>;
        bind_data<V>(m, "TypeData")
            .def(nb::init<std::string, ygg::IndexList<formalism::Type>>(), "name"_a, "bases"_a = ygg::IndexList<formalism::Type> {})
            .def(nb::init<const std::string&, const std::vector<formalism::TypeView>&>(), "name"_a, "bases"_a = std::vector<formalism::TypeView> {})
            .def_rw("name", &V::name)
            .def_rw("bases", &V::bases);
    }

    {
        using V = formalism::TypeView;
        auto cls = nb::class_<V>(m, "Type");
        cls.def("get_index", &V::get_index).def("get_name", &V::get_name).def("get_bases", &V::get_bases);
        ygg::add_print(cls);
        ygg::add_comparison(cls);
        ygg::add_hash(cls);
    }

    bind_insert<formalism::Type>(repository);
}

}  // namespace loki::formalism
