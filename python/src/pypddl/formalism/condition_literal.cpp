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
#include "pypddl/formalism/module.hpp"

#include <loki/formalism/condition_literal_view.hpp>
#include <loki/formalism/literal_view.hpp>

namespace nb = nanobind;

namespace loki::formalism
{

void bind_condition_literal(nb::module_& m, RepositoryBinding& repository)
{
    using namespace nb::literals;
    urd::python::bind_index<urd::index<formalism::Condition<formalism::LiteralTag>>>(m, "ConditionLiteralIndex");

    {
        using V = urd::data<formalism::Condition<formalism::LiteralTag>>;
        urd::python::bind_data<V>(m, "ConditionLiteralData")
            .def(nb::init<urd::index<formalism::Literal>>(), "literal"_a)
            .def(urd::python::init_from_views<formalism::LiteralView>(), "literal"_a)
            .def_rw("literal", &V::literal);
    }

    {
        using V = formalism::EntityView<formalism::Condition<formalism::LiteralTag>>;
        auto cls = nb::class_<V>(m, "ConditionLiteral");
        cls.def("get_index", &V::index).def("get_literal", &V::get_literal, nb::keep_alive<0, 1>());
        ygg::add_print(cls);
        urd::python::add_comparison(cls);
        urd::python::add_hash(cls);
    }

    bind_insert<formalism::Condition<formalism::LiteralTag>>(repository);
}

}  // namespace loki::formalism
