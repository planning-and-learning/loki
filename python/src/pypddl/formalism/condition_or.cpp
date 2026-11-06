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
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <urd/python/sequences/sequence.hpp>

#include <loki/formalism/condition_or_view.hpp>

namespace nb = nanobind;

namespace loki::formalism
{

void bind_condition_or(nb::module_& m, RepositoryBinding& repository)
{
    using namespace nb::literals;
    urd::python::bind_index<urd::index<formalism::Condition<formalism::OrTag>>>(m, "ConditionOrIndex");

    {
        using V = urd::data<formalism::Condition<formalism::OrTag>>;
        urd::python::bind_data<V>(m, "ConditionOrData")
            .def(nb::init<formalism::ConditionList>(), "conditions"_a)
            .def(urd::python::init_from_views<const urd::Store<urd::sequence<formalism::ConditionView>>&>(), "conditions"_a)
            .def_rw("conditions", &V::conditions);
    }

    {
        using V = formalism::EntityView<formalism::Condition<formalism::OrTag>>;
        auto cls = nb::class_<V>(m, "ConditionOr");
        cls.def("get_index", &V::index).def("get_conditions", &V::get_conditions);
        ygg::add_print(cls);
        urd::python::add_comparison(cls);
        urd::python::add_hash(cls);
    }

    bind_insert<formalism::Condition<formalism::OrTag>>(repository);
}

}  // namespace loki::formalism
