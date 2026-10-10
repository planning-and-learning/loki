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

#include <loki/formalism/condition_view.hpp>
#include <loki/formalism/effect_view.hpp>
#include <loki/formalism/effect_when_view.hpp>

namespace nb = nanobind;
using namespace nb::literals;

namespace loki::formalism
{

void bind_effect_when(nb::module_& m, RepositoryBinding& repository)
{
    ygg::bind_index<ygg::Index<formalism::Effect<formalism::WhenTag>>>(m, "EffectWhenIndex");

    {
        using V = Data<formalism::Effect<formalism::WhenTag>>;
        bind_data<V>(m, "EffectWhenData")
            .def(nb::init<ygg::Index<formalism::Condition<>>, ygg::Index<formalism::Effect<>>>(), "condition"_a, "effect"_a)
            .def(nb::init<formalism::ConditionView, formalism::EffectView>(), "condition"_a, "effect"_a)
            .def_rw("condition", &V::condition)
            .def_rw("effect", &V::effect);
    }

    {
        using V = formalism::EntityView<formalism::Effect<formalism::WhenTag>>;
        auto cls = nb::class_<V>(m, "EffectWhen");
        cls.def("get_index", &V::get_index)
            .def("get_condition", &V::get_condition, nb::keep_alive<0, 1>())
            .def("get_effect", &V::get_effect, nb::keep_alive<0, 1>());
        ygg::add_print(cls);
        ygg::add_comparison(cls);
        ygg::add_hash(cls);
    }

    bind_insert<formalism::Effect<formalism::WhenTag>>(repository);
}

}  // namespace loki::formalism
