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

#include <loki/formalism/function_expression_view.hpp>
#include <loki/formalism/function_term_view.hpp>
#include <loki/formalism/initial_function_value_view.hpp>

namespace nb = nanobind;
using namespace nb::literals;

namespace loki::formalism
{

void bind_initial_function_value(nb::module_& m, RepositoryBinding& repository)
{
    ygg::bind_index<ygg::Index<formalism::InitialFunctionValue>>(m, "InitialFunctionValueIndex");

    {
        using V = Data<formalism::InitialFunctionValue>;
        bind_data<V>(m, "InitialFunctionValueData")
            .def(nb::init<formalism::FunctionTermView, formalism::FunctionExpressionView>(), "function"_a, "value"_a)
            .def_rw("function", &V::function)
            .def_rw("value", &V::value);
    }

    {
        using V = formalism::InitialFunctionValueView;
        auto cls = nb::class_<V>(m, "InitialFunctionValue");
        cls.def("get_index", &V::get_index)
            .def("get_function", &V::get_function, nb::keep_alive<0, 1>())
            .def("get_value", &V::get_value, nb::keep_alive<0, 1>());
        ygg::add_print(cls);
        ygg::add_comparison(cls);
        ygg::add_hash(cls);
    }

    bind_insert<formalism::InitialFunctionValue>(repository);
}

}  // namespace loki::formalism
