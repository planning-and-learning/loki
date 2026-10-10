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
#include <loki/formalism/predicate_view.hpp>
#include <loki/formalism/term_view.hpp>

namespace nb = nanobind;
using namespace nb::literals;

namespace loki::formalism
{

void bind_atom(nb::module_& m, RepositoryBinding& repository)
{
    ygg::bind_index<ygg::Index<formalism::Atom>>(m, "AtomIndex");

    {
        using V = Data<formalism::Atom>;
        bind_data<V>(m, "AtomData")
            .def(nb::init<ygg::Index<formalism::Predicate>, ygg::IndexList<formalism::Term>>(), "predicate"_a, "terms"_a = ygg::IndexList<formalism::Term> {})
            .def(nb::init<formalism::PredicateView, const std::vector<formalism::TermView>&>(), "predicate"_a, "terms"_a = std::vector<formalism::TermView> {})
            .def_rw("predicate", &V::predicate)
            .def_rw("terms", &V::terms);
    }

    {
        using V = formalism::AtomView;
        auto cls = nb::class_<V>(m, "Atom");
        cls.def("get_index", &V::get_index).def("get_predicate", &V::get_predicate, nb::keep_alive<0, 1>()).def("get_terms", &V::get_terms);
        ygg::add_print(cls);
        ygg::add_comparison(cls);
        ygg::add_hash(cls);
    }

    bind_insert<formalism::Atom>(repository);
}

}  // namespace loki::formalism
