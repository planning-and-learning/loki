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
#include <loki/formalism/axiom_view.hpp>
#include <loki/formalism/domain_view.hpp>
#include <loki/formalism/function_skeleton_view.hpp>
#include <loki/formalism/object_view.hpp>
#include <loki/formalism/predicate_view.hpp>
#include <loki/formalism/requirement_view.hpp>
#include <loki/formalism/type_view.hpp>

namespace nb = nanobind;
using namespace nb::literals;

namespace loki::formalism
{

void bind_domain(nb::module_& m, RepositoryBinding& repository)
{
    ygg::bind_index<ygg::Index<formalism::Domain>>(m, "DomainIndex");

    {
        using V = Data<formalism::Domain>;
        bind_data<V>(m, "DomainData")
            .def(nb::init<const std::string&,
                          const std::vector<formalism::RequirementView>&,
                          const std::vector<formalism::TypeView>&,
                          const std::vector<formalism::ObjectView>&,
                          const std::vector<formalism::PredicateView>&,
                          const std::vector<formalism::FunctionSkeletonView>&,
                          const std::vector<formalism::ActionView>&,
                          const std::vector<formalism::AxiomView>&>(),
                 "name"_a,
                 "requirements"_a = std::vector<formalism::RequirementView> {},
                 "types"_a = std::vector<formalism::TypeView> {},
                 "constants"_a = std::vector<formalism::ObjectView> {},
                 "predicates"_a = std::vector<formalism::PredicateView> {},
                 "functions"_a = std::vector<formalism::FunctionSkeletonView> {},
                 "actions"_a = std::vector<formalism::ActionView> {},
                 "axioms"_a = std::vector<formalism::AxiomView> {})
            .def_rw("name", &V::name)
            .def_rw("requirements", &V::requirements)
            .def_rw("types", &V::types)
            .def_rw("constants", &V::constants)
            .def_rw("predicates", &V::predicates)
            .def_rw("functions", &V::functions)
            .def_rw("actions", &V::actions)
            .def_rw("axioms", &V::axioms);
    }

    {
        using V = formalism::DomainView;
        auto cls = nb::class_<V>(m, "Domain");
        cls.def("get_index", &V::get_index)
            .def("get_name", &V::get_name)
            .def("get_requirements", &V::get_requirements)
            .def("get_types", &V::get_types)
            .def("get_constants", &V::get_constants)
            .def("get_predicates", &V::get_predicates)
            .def("get_functions", &V::get_functions)
            .def("get_actions", &V::get_actions)
            .def("get_axioms", &V::get_axioms);
        ygg::add_print(cls);
        ygg::add_comparison(cls);
        ygg::add_hash(cls);
    }

    bind_insert<formalism::Domain>(repository);
}

}  // namespace loki::formalism
