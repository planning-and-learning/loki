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

#include <loki/formalism/effect_and_view.hpp>
#include <loki/formalism/effect_view.hpp>

namespace nb = nanobind;
using namespace nb::literals;

namespace loki::formalism
{

void bind_effect_and(nb::module_& m, RepositoryBinding& repository)
{
    ygg::bind_index<ygg::Index<formalism::EffectAnd>>(m, "EffectAndIndex");

    {
        using V = Data<formalism::EffectAnd>;
        bind_data<V>(m, "EffectAndData")
            .def(nb::init<ygg::IndexList<formalism::Effect>>(), "effects"_a)
            .def(nb::init<const std::vector<formalism::EffectView>&>(), "effects"_a)
            .def_rw("effects", &V::effects);
    }

    {
        using V = formalism::EffectAndView;
        auto cls = nb::class_<V>(m, "EffectAnd");
        cls.def("get_index", &V::get_index).def("get_effects", &V::get_effects);
        ygg::add_print(cls);
        ygg::add_comparison(cls);
        ygg::add_hash(cls);
    }

    bind_insert<formalism::EffectAnd>(repository);
}

}  // namespace loki::formalism
