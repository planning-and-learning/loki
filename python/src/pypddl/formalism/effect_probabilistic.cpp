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

#include <loki/formalism/effect_probabilistic_alternative_view.hpp>
#include <loki/formalism/effect_probabilistic_view.hpp>

namespace nb = nanobind;

namespace loki::formalism
{

void bind_effect_probabilistic(nb::module_& m, RepositoryBinding& repository)
{
    using namespace nb::literals;
    urd::python::bind_index<urd::index<formalism::Effect<formalism::ProbabilisticTag>>>(m, "EffectProbabilisticIndex");

    {
        using V = urd::data<formalism::Effect<formalism::ProbabilisticTag>>;
        urd::python::bind_data<V>(m, "EffectProbabilisticData")
            .def(nb::init<urd::flat_vector<urd::index<formalism::EffectProbabilisticAlternative>>>(), "alternatives"_a)
            .def(urd::python::init_from_views<const urd::Store<urd::sequence<formalism::EffectProbabilisticAlternativeView>>&>(), "alternatives"_a)
            .def_rw("alternatives", &V::alternatives);
    }

    {
        using V = formalism::EntityView<formalism::Effect<formalism::ProbabilisticTag>>;
        auto cls = nb::class_<V>(m, "EffectProbabilistic");
        cls.def("get_index", &V::index).def("get_alternatives", &V::get_alternatives);
        ygg::add_print(cls);
        urd::python::add_comparison(cls);
        urd::python::add_hash(cls);
    }

    bind_insert<formalism::Effect<formalism::ProbabilisticTag>>(repository);
}

}  // namespace loki::formalism
