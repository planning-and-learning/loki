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

#include <loki/formalism/atom_view.hpp>
#include <loki/formalism/literal_view.hpp>

namespace nb = nanobind;
using namespace nb::literals;

namespace loki::formalism
{

void bind_literal(nb::module_& m, RepositoryBinding& repository)
{
    ygg::bind_index<ygg::Index<formalism::Literal>>(m, "LiteralIndex");

    {
        using V = Data<formalism::Literal>;
        bind_data<V>(m, "LiteralData")
            .def(nb::init<ygg::Index<formalism::Atom>, bool>(), "atom"_a, "polarity"_a = true)
            .def(nb::init<formalism::AtomView, bool>(), "atom"_a, "polarity"_a = true)
            .def_rw("atom", &V::atom)
            .def_rw("polarity", &V::m_polarity);
    }

    {
        using V = formalism::LiteralView;
        auto cls = nb::class_<V>(m, "Literal");
        cls.def("get_index", &V::get_index).def("get_atom", &V::get_atom, nb::keep_alive<0, 1>()).def("get_polarity", &V::get_polarity);
        ygg::add_print(cls);
        ygg::add_comparison(cls);
        ygg::add_hash(cls);
    }

    bind_insert<formalism::Literal>(repository);
}

}  // namespace loki::formalism
