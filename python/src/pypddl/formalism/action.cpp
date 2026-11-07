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

#include <loki/formalism/action_view.hpp>
#include <loki/formalism/parameter_view.hpp>

namespace nb = nanobind;

namespace loki::formalism
{

void bind_action(nb::module_& m, RepositoryBinding& repository)
{
    using namespace nb::literals;
    urd::python::bind_index<urd::index<formalism::Action>>(m, "ActionIndex");

    {
        using V = urd::data<formalism::Action>;
        urd::python::bind_data<V>(m, "ActionData")
            .def(nb::init<std::string,
                          urd::flat_vector<urd::index<formalism::Parameter>>,
                          cista::optional<formalism::ConditionVariant>,
                          cista::optional<formalism::EffectVariant>>(),
                 "name"_a,
                 "parameters"_a = urd::flat_vector<urd::index<formalism::Parameter>> {},
                 "precondition"_a.none() = cista::optional<formalism::ConditionVariant> {},
                 "effect"_a.none() = cista::optional<formalism::EffectVariant> {})
            .def(nb::init<std::string,
                          std::string,
                          urd::flat_vector<urd::index<formalism::Parameter>>,
                          ygg::uint_t,
                          cista::optional<formalism::ConditionVariant>,
                          cista::optional<formalism::EffectVariant>>(),
                 "name"_a,
                 "original_name"_a,
                 "parameters"_a,
                 "original_arity"_a,
                 "precondition"_a.none() = cista::optional<formalism::ConditionVariant> {},
                 "effect"_a.none() = cista::optional<formalism::EffectVariant> {})
            .def(urd::python::init_from_views<const std::string&,
                          const urd::Store<urd::sequence<formalism::ParameterView>>&,
                          const std::optional<formalism::ConditionView>&,
                          const std::optional<formalism::EffectView>&>(),
                 "name"_a,
                 "parameters"_a = urd::Store<urd::sequence<formalism::ParameterView>> {},
                 "precondition"_a = std::optional<formalism::ConditionView> {},
                 "effect"_a = std::optional<formalism::EffectView> {})
            .def(urd::python::init_from_views<const std::string&,
                          const std::string&,
                          const urd::Store<urd::sequence<formalism::ParameterView>>&,
                          ygg::uint_t,
                          const std::optional<formalism::ConditionView>&,
                          const std::optional<formalism::EffectView>&>(),
                 "name"_a,
                 "original_name"_a,
                 "parameters"_a,
                 "original_arity"_a,
                 "precondition"_a = std::optional<formalism::ConditionView> {},
                 "effect"_a = std::optional<formalism::EffectView> {})
            .def_rw("name", &V::name)
            .def_rw("original_name", &V::original_name)
            .def_rw("parameters", &V::parameters)
            .def_rw("original_arity", &V::original_arity)
            .def_rw("precondition", &V::precondition)
            .def_rw("effect", &V::effect);
    }

    {
        using V = formalism::ActionView;
        auto cls = nb::class_<V>(m, "Action");
        cls.def("get_index", &V::index)
            .def("get_name", &V::get_name)
            .def("get_original_name", &V::get_original_name)
            .def("get_parameters", &V::get_parameters)
            .def("get_arity", &V::get_arity)
            .def("get_original_arity", &V::get_original_arity)
            .def("get_precondition", &V::get_precondition, nb::keep_alive<0, 1>())
            .def("get_effect", &V::get_effect, nb::keep_alive<0, 1>());
        ygg::add_print(cls);
        urd::python::add_comparison(cls);
        urd::python::add_hash(cls);
    }

    bind_insert<formalism::Action>(repository);
}

}  // namespace loki::formalism
