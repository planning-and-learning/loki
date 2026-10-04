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

#include <loki/formalism/condition_numeric_constraint_view.hpp>
#include <loki/formalism/function_expression_view.hpp>

namespace nb = nanobind;
using namespace nb::literals;

namespace loki::formalism
{

void bind_condition_numeric_constraint(nb::module_& m, RepositoryBinding& repository)
{
    ygg::bind_index<ygg::Index<formalism::ConditionNumericConstraint>>(m, "ConditionNumericConstraintIndex");

    {
        using V = Data<formalism::ConditionNumericConstraint>;
        bind_data<V>(m, "ConditionNumericConstraintData")
            .def(nb::init<formalism::BinaryComparator, formalism::FunctionExpressionView, formalism::FunctionExpressionView>(),
                 "comparator"_a,
                 "left"_a,
                 "right"_a)
            .def_rw("comparator", &V::comparator)
            .def_rw("left", &V::left)
            .def_rw("right", &V::right);
    }

    {
        using V = formalism::ConditionNumericConstraintView;
        auto cls = nb::class_<V>(m, "ConditionNumericConstraint");
        cls.def("get_index", &V::get_index)
            .def("get_comparator", &V::get_comparator)
            .def("get_left", &V::get_left, nb::keep_alive<0, 1>())
            .def("get_right", &V::get_right, nb::keep_alive<0, 1>());
        ygg::add_print(cls);
        ygg::add_comparison(cls);
        ygg::add_hash(cls);
    }

    bind_insert<formalism::ConditionNumericConstraint>(repository);
}

}  // namespace loki::formalism
