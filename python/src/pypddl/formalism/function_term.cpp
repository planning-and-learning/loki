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

#include <loki/formalism/function_skeleton_view.hpp>
#include <loki/formalism/function_term_view.hpp>
#include <loki/formalism/term_view.hpp>

namespace nb = nanobind;
using namespace nb::literals;

namespace loki::formalism
{

void bind_function_term(nb::module_& m, RepositoryBinding& repository)
{
    ygg::bind_index<ygg::Index<formalism::FunctionTerm>>(m, "FunctionTermIndex");

    {
        using V = Data<formalism::FunctionTerm>;
        bind_data<V>(m, "FunctionTermData")
            .def(nb::init<ygg::Index<formalism::FunctionSkeleton>, ygg::IndexList<formalism::Term>>(),
                 "function"_a,
                 "terms"_a = ygg::IndexList<formalism::Term> {})
            .def(nb::init<formalism::FunctionSkeletonView, const std::vector<formalism::TermView>&>(),
                 "function"_a,
                 "terms"_a = std::vector<formalism::TermView> {})
            .def_rw("function", &V::function)
            .def_rw("terms", &V::terms);
    }

    {
        using V = formalism::FunctionTermView;
        auto cls = nb::class_<V>(m, "FunctionTerm");
        cls.def("get_index", &V::get_index).def("get_function", &V::get_function, nb::keep_alive<0, 1>()).def("get_terms", &V::get_terms);
        ygg::add_print(cls);
        ygg::add_comparison(cls);
        ygg::add_hash(cls);
    }

    bind_insert<formalism::FunctionTerm>(repository);
}

}  // namespace loki::formalism
