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

#include <loki/formalism/function_skeleton_view.hpp>
#include <loki/formalism/function_term_view.hpp>

namespace nb = nanobind;

namespace loki::formalism
{

void bind_function_term(nb::module_& m, RepositoryBinding& repository)
{
    using namespace nb::literals;
    urd::python::bind_index<urd::index<formalism::FunctionTerm>>(m, "FunctionTermIndex");

    {
        using V = urd::data<formalism::FunctionTerm>;
        urd::python::bind_data<V>(m, "FunctionTermData")
            .def(nb::init<urd::index<formalism::FunctionSkeleton>, formalism::TermList>(),
                 "function"_a,
                 "terms"_a = formalism::TermList {})
            .def(urd::python::init_from_views<formalism::FunctionSkeletonView, const urd::Store<urd::sequence<formalism::TermView>>&>(),
                 "function"_a,
                 "terms"_a = urd::Store<urd::sequence<formalism::TermView>> {})
            .def_rw("function", &V::function)
            .def_rw("terms", &V::terms);
    }

    {
        using V = formalism::FunctionTermView;
        auto cls = nb::class_<V>(m, "FunctionTerm");
        cls.def("get_index", &V::index).def("get_function", &V::get_function, nb::keep_alive<0, 1>()).def("get_terms", &V::get_terms);
        ygg::add_print(cls);
        urd::python::add_comparison(cls);
        urd::python::add_hash(cls);
    }

    bind_insert<formalism::FunctionTerm>(repository);
}

}  // namespace loki::formalism
