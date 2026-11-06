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

#include <loki/formalism/binary_function_expression_view.hpp>

namespace nb = nanobind;

namespace loki::formalism
{

void bind_binary_function_expression(nb::module_& m, RepositoryBinding& repository)
{
    using namespace nb::literals;
    urd::python::bind_index<urd::index<formalism::FunctionExpression<formalism::BinaryTag>>>(m, "BinaryFunctionExpressionIndex");

    {
        using V = urd::data<formalism::FunctionExpression<formalism::BinaryTag>>;
        urd::python::bind_data<V>(m, "BinaryFunctionExpressionData")
            .def(nb::init<formalism::BinaryArithmeticOperator, formalism::FunctionExpressionVariant, formalism::FunctionExpressionVariant>(),
                 "operator"_a,
                 "left"_a,
                 "right"_a)
            .def(urd::python::init_from_views<formalism::BinaryArithmeticOperator, formalism::FunctionExpressionView, formalism::FunctionExpressionView>(),
                 "operator"_a,
                 "left"_a,
                 "right"_a)
            .def_rw("operator", &V::op)
            .def_rw("left", &V::left)
            .def_rw("right", &V::right);
    }

    {
        using V = formalism::EntityView<formalism::FunctionExpression<formalism::BinaryTag>>;
        auto cls = nb::class_<V>(m, "BinaryFunctionExpression");
        cls.def("get_index", &V::index)
            .def("get_operator", &V::get_operator)
            .def("get_left", &V::get_left, nb::keep_alive<0, 1>())
            .def("get_right", &V::get_right, nb::keep_alive<0, 1>());
        ygg::add_print(cls);
        urd::python::add_comparison(cls);
        urd::python::add_hash(cls);
    }

    bind_insert<formalism::FunctionExpression<formalism::BinaryTag>>(repository);
}

}  // namespace loki::formalism
