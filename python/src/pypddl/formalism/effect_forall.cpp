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

#include <loki/formalism/effect_forall_view.hpp>
#include <loki/formalism/parameter_view.hpp>

namespace nb = nanobind;

namespace loki::formalism
{

void bind_effect_forall(nb::module_& m, RepositoryBinding& repository)
{
    using namespace nb::literals;
    urd::python::bind_index<urd::index<formalism::Effect<formalism::ForallTag>>>(m, "EffectForallIndex");

    {
        using V = urd::data<formalism::Effect<formalism::ForallTag>>;
        urd::python::bind_data<V>(m, "EffectForallData")
            .def(nb::init<urd::flat_vector<urd::index<formalism::Parameter>>, formalism::EffectVariant>(), "parameters"_a, "effect"_a)
            .def(urd::python::init_from_views<const urd::Store<urd::sequence<formalism::ParameterView>>&, formalism::EffectView>(), "parameters"_a, "effect"_a)
            .def_rw("parameters", &V::parameters)
            .def_rw("effect", &V::effect);
    }

    {
        using V = formalism::EntityView<formalism::Effect<formalism::ForallTag>>;
        auto cls = nb::class_<V>(m, "EffectForall");
        cls.def("get_index", &V::index).def("get_parameters", &V::get_parameters).def("get_effect", &V::get_effect, nb::keep_alive<0, 1>());
        ygg::add_print(cls);
        urd::python::add_comparison(cls);
        urd::python::add_hash(cls);
    }

    bind_insert<formalism::Effect<formalism::ForallTag>>(repository);
}

}  // namespace loki::formalism
