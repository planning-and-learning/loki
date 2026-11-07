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
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <urd/python/sequences/sequence.hpp>

#include <loki/formalism/effect_one_of_view.hpp>

namespace nb = nanobind;

namespace loki::formalism
{

void bind_effect_one_of(nb::module_& m, RepositoryBinding& repository)
{
    using namespace nb::literals;
    urd::python::bind_index<urd::index<formalism::Effect<formalism::OneOfTag>>>(m, "EffectOneOfIndex");

    {
        using V = urd::data<formalism::Effect<formalism::OneOfTag>>;
        urd::python::bind_data<V>(m, "EffectOneOfData")
            .def(nb::init<formalism::EffectList>(), "effects"_a)
            .def(urd::python::init_from_views<const urd::Store<urd::sequence<formalism::EffectView>>&>(), "effects"_a)
            .def_rw("effects", &V::effects);
    }

    {
        using V = formalism::EntityView<formalism::Effect<formalism::OneOfTag>>;
        auto cls = nb::class_<V>(m, "EffectOneOf");
        cls.def("get_index", &V::index).def("get_effects", &V::get_effects);
        ygg::add_print(cls);
        urd::python::add_comparison(cls);
        urd::python::add_hash(cls);
    }

    bind_insert<formalism::Effect<formalism::OneOfTag>>(repository);
}

}  // namespace loki::formalism
