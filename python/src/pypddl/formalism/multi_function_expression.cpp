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
#include <loki/formalism/multi_function_expression_view.hpp>

namespace nb = nanobind;
using namespace nb::literals;

namespace loki::formalism
{

void bind_multi_function_expression(nb::module_& m, RepositoryBinding& repository)
{
    ygg::bind_index<ygg::Index<formalism::MultiFunctionExpression>>(m, "MultiFunctionExpressionIndex");

    {
        using V = Data<formalism::MultiFunctionExpression>;
        bind_data<V>(m, "MultiFunctionExpressionData")
            .def(nb::init<formalism::MultiArithmeticOperator, ygg::IndexList<formalism::FunctionExpression>>(), "operator"_a, "args"_a)
            .def(nb::init<formalism::MultiArithmeticOperator, const std::vector<formalism::FunctionExpressionView>&>(),
                 "operator"_a,
                 "args"_a = std::vector<formalism::FunctionExpressionView> {})
            .def_rw("operator", &V::op)
            .def_rw("args", &V::args);
    }

    {
        using V = formalism::MultiFunctionExpressionView;
        const auto attach_owner = nb::cpp_function([](nb::object child, nb::object) { return child; }, nb::keep_alive<0, 2>());
        auto cls = nb::class_<V>(m, "MultiFunctionExpression");
        cls.def("get_index", &V::get_index)
            .def("get_operator", &V::get_operator)
            .def("get_args",
                 [attach_owner](const V& self)
                 {
                     auto result = nb::list();
                     const auto parent = nb::find(self);
                     for (const auto expression : self.get_args())
                     {
                         result.append(attach_owner(nb::cast(expression), parent));
                     }
                     return result;
                 });
        ygg::add_print(cls);
        ygg::add_comparison(cls);
        ygg::add_hash(cls);
    }

    bind_insert<formalism::MultiFunctionExpression>(repository);
}

}  // namespace loki::formalism
