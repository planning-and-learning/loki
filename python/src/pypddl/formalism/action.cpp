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

#include <loki/formalism/action_view.hpp>
#include <loki/formalism/condition_view.hpp>
#include <loki/formalism/effect_view.hpp>
#include <loki/formalism/parameter_view.hpp>

namespace nb = nanobind;
using namespace nb::literals;

namespace loki::formalism
{

void bind_action(nb::module_& m, RepositoryBinding& repository)
{
    ygg::bind_index<ygg::Index<formalism::Action>>(m, "ActionIndex");

    {
        using V = Data<formalism::Action>;
        bind_data<V>(m, "ActionData")
            .def(nb::init<std::string,
                          ygg::IndexList<formalism::Parameter>,
                          cista::optional<ygg::Index<formalism::Condition>>,
                          cista::optional<ygg::Index<formalism::Effect>>>(),
                 "name"_a,
                 "parameters"_a = ygg::IndexList<formalism::Parameter> {},
                 "precondition"_a.none() = cista::optional<ygg::Index<formalism::Condition>> {},
                 "effect"_a.none() = cista::optional<ygg::Index<formalism::Effect>> {})
            .def(nb::init<std::string,
                          std::string,
                          ygg::IndexList<formalism::Parameter>,
                          ygg::uint_t,
                          cista::optional<ygg::Index<formalism::Condition>>,
                          cista::optional<ygg::Index<formalism::Effect>>>(),
                 "name"_a,
                 "original_name"_a,
                 "parameters"_a,
                 "original_arity"_a,
                 "precondition"_a.none() = cista::optional<ygg::Index<formalism::Condition>> {},
                 "effect"_a.none() = cista::optional<ygg::Index<formalism::Effect>> {})
            .def(nb::init<const std::string&,
                          const std::vector<formalism::ParameterView>&,
                          const std::optional<formalism::ConditionView>&,
                          const std::optional<formalism::EffectView>&>(),
                 "name"_a,
                 "parameters"_a = std::vector<formalism::ParameterView> {},
                 "precondition"_a = std::optional<formalism::ConditionView> {},
                 "effect"_a = std::optional<formalism::EffectView> {})
            .def(nb::init<const std::string&,
                          const std::string&,
                          const std::vector<formalism::ParameterView>&,
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
        cls.def("get_index", &V::get_index)
            .def("get_name", &V::get_name)
            .def("get_original_name", &V::get_original_name)
            .def("get_parameters", &V::get_parameters)
            .def("get_arity", &V::get_arity)
            .def("get_original_arity", &V::get_original_arity)
            .def("get_precondition", &V::get_precondition, nb::keep_alive<0, 1>())
            .def("get_effect", &V::get_effect, nb::keep_alive<0, 1>());
        ygg::add_print(cls);
        ygg::add_comparison(cls);
        ygg::add_hash(cls);
    }

    bind_insert<formalism::Action>(repository);
}

}  // namespace loki::formalism
