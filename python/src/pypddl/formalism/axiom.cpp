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

#include <loki/formalism/axiom_view.hpp>
#include <loki/formalism/condition_view.hpp>
#include <loki/formalism/literal_view.hpp>
#include <loki/formalism/parameter_view.hpp>

namespace nb = nanobind;
using namespace nb::literals;

namespace loki::formalism
{

void bind_axiom(nb::module_& m, RepositoryBinding& repository)
{
    ygg::bind_index<ygg::Index<formalism::Axiom>>(m, "AxiomIndex");

    {
        using V = Data<formalism::Axiom>;
        bind_data<V>(m, "AxiomData")
            .def(nb::init<ygg::IndexList<formalism::Parameter>, ygg::Index<formalism::Literal>, ygg::Index<formalism::Condition>>(),
                 "parameters"_a,
                 "head"_a,
                 "condition"_a)
            .def(nb::init<ygg::IndexList<formalism::Parameter>, ygg::uint_t, ygg::Index<formalism::Literal>, ygg::Index<formalism::Condition>>(),
                 "parameters"_a,
                 "original_arity"_a,
                 "head"_a,
                 "condition"_a)
            .def(nb::init<const std::vector<formalism::ParameterView>&, formalism::LiteralView, formalism::ConditionView>(),
                 "parameters"_a,
                 "head"_a,
                 "condition"_a)
            .def(nb::init<const std::vector<formalism::ParameterView>&, ygg::uint_t, formalism::LiteralView, formalism::ConditionView>(),
                 "parameters"_a,
                 "original_arity"_a,
                 "head"_a,
                 "condition"_a)
            .def_rw("parameters", &V::parameters)
            .def_rw("original_arity", &V::original_arity)
            .def_rw("head", &V::head)
            .def_rw("condition", &V::condition);
    }

    {
        using V = formalism::AxiomView;
        auto cls = nb::class_<V>(m, "Axiom");
        cls.def("get_index", &V::get_index)
            .def("get_parameters", &V::get_parameters)
            .def("get_arity", &V::get_arity)
            .def("get_original_arity", &V::get_original_arity)
            .def("get_head", &V::get_head, nb::keep_alive<0, 1>())
            .def("get_condition", &V::get_condition, nb::keep_alive<0, 1>());
        ygg::add_print(cls);
        ygg::add_comparison(cls);
        ygg::add_hash(cls);
    }

    bind_insert<formalism::Axiom>(repository);
}

}  // namespace loki::formalism
