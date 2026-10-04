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
#include <loki/formalism/metric_view.hpp>

namespace nb = nanobind;
using namespace nb::literals;

namespace loki::formalism
{

void bind_metric(nb::module_& m, RepositoryBinding& repository)
{
    ygg::bind_index<ygg::Index<formalism::Metric>>(m, "MetricIndex");

    {
        using V = Data<formalism::Metric>;
        bind_data<V>(m, "MetricData")
            .def(nb::init<formalism::OptimizationDirection, formalism::FunctionExpressionView>(), "optimization_direction"_a, "expression"_a)
            .def_rw("optimization_direction", &V::optimization_direction)
            .def_rw("expression", &V::expression);
    }

    {
        using V = formalism::MetricView;
        auto cls = nb::class_<V>(m, "Metric");
        cls.def("get_index", &V::get_index)
            .def("get_optimization_direction", &V::get_optimization_direction)
            .def("get_expression", &V::get_expression, nb::keep_alive<0, 1>());
        ygg::add_print(cls);
        ygg::add_comparison(cls);
        ygg::add_hash(cls);
    }

    bind_insert<formalism::Metric>(repository);
}

}  // namespace loki::formalism
