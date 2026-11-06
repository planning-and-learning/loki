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

#include <loki/formalism/effect_numeric_view.hpp>
#include <loki/formalism/function_term_view.hpp>

namespace nb = nanobind;

namespace loki::formalism
{

void bind_effect_numeric(nb::module_& m, RepositoryBinding& repository)
{
    using namespace nb::literals;
    urd::python::bind_index<urd::index<formalism::Effect<formalism::NumericTag>>>(m, "EffectNumericIndex");

    {
        using V = urd::data<formalism::Effect<formalism::NumericTag>>;
        urd::python::bind_data<V>(m, "EffectNumericData")
            .def(nb::init<formalism::NumericEffectOperator, urd::index<formalism::FunctionTerm>, formalism::FunctionExpressionVariant>(),
                 "operator"_a,
                 "function"_a,
                 "expression"_a)
            .def(urd::python::init_from_views<formalism::NumericEffectOperator, formalism::FunctionTermView, formalism::FunctionExpressionView>(),
                 "operator"_a,
                 "function"_a,
                 "expression"_a)
            .def_rw("operator", &V::op)
            .def_rw("function", &V::function)
            .def_rw("expression", &V::expression);
    }

    {
        using V = formalism::EntityView<formalism::Effect<formalism::NumericTag>>;
        auto cls = nb::class_<V>(m, "EffectNumeric");
        cls.def("get_index", &V::index)
            .def("get_operator", &V::get_operator)
            .def("get_function", &V::get_function, nb::keep_alive<0, 1>())
            .def("get_expression", &V::get_expression, nb::keep_alive<0, 1>());
        ygg::add_print(cls);
        urd::python::add_comparison(cls);
        urd::python::add_hash(cls);
    }

    bind_insert<formalism::Effect<formalism::NumericTag>>(repository);
}

}  // namespace loki::formalism
