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

#include <loki/formalism/function_expression_number_view.hpp>

namespace nb = nanobind;

namespace loki::formalism
{

void bind_function_expression_number(nb::module_& m, RepositoryBinding& repository)
{
    using namespace nb::literals;
    urd::python::bind_index<urd::index<formalism::FunctionExpression<formalism::NumberTag>>>(m, "FunctionExpressionNumberIndex");

    {
        using V = urd::data<formalism::FunctionExpression<formalism::NumberTag>>;
        urd::python::bind_data<V>(m, "FunctionExpressionNumberData").def(nb::init<double>(), "value"_a).def_rw("value", &V::value);
    }

    {
        using V = formalism::EntityView<formalism::FunctionExpression<formalism::NumberTag>>;
        auto cls = nb::class_<V>(m, "FunctionExpressionNumber");
        cls.def("get_index", &V::index).def("get_value", &V::get_value);
        ygg::add_print(cls);
        urd::python::add_comparison(cls);
        urd::python::add_hash(cls);
    }

    bind_insert<formalism::FunctionExpression<formalism::NumberTag>>(repository);
}

}  // namespace loki::formalism
