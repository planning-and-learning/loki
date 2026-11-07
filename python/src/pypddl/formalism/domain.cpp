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
#include <loki/formalism/axiom_view.hpp>
#include <loki/formalism/domain_view.hpp>
#include <loki/formalism/function_skeleton_view.hpp>
#include <loki/formalism/object_view.hpp>
#include <loki/formalism/predicate_view.hpp>
#include <loki/formalism/requirement_view.hpp>
#include <loki/formalism/type_view.hpp>

namespace nb = nanobind;

namespace loki::formalism
{

void bind_domain(nb::module_& m, RepositoryBinding& repository)
{
    using namespace nb::literals;
    urd::python::bind_index<urd::index<formalism::Domain>>(m, "DomainIndex");

    {
        using V = urd::data<formalism::Domain>;
        urd::python::bind_data<V>(m, "DomainData")
            .def(nb::init<std::string,
                          urd::flat_vector<urd::index<formalism::Requirement>>,
                          urd::flat_vector<urd::index<formalism::Type>>,
                          urd::flat_vector<urd::index<formalism::Object>>,
                          urd::flat_vector<urd::index<formalism::Predicate>>,
                          urd::flat_vector<urd::index<formalism::FunctionSkeleton>>,
                          urd::flat_vector<urd::index<formalism::Action>>,
                          urd::flat_vector<urd::index<formalism::Axiom>>>(),
                 "name"_a,
                 "requirements"_a = urd::flat_vector<urd::index<formalism::Requirement>> {},
                 "types"_a = urd::flat_vector<urd::index<formalism::Type>> {},
                 "constants"_a = urd::flat_vector<urd::index<formalism::Object>> {},
                 "predicates"_a = urd::flat_vector<urd::index<formalism::Predicate>> {},
                 "functions"_a = urd::flat_vector<urd::index<formalism::FunctionSkeleton>> {},
                 "actions"_a = urd::flat_vector<urd::index<formalism::Action>> {},
                 "axioms"_a = urd::flat_vector<urd::index<formalism::Axiom>> {})
            .def(urd::python::init_from_views<const std::string&,
                          const urd::Store<urd::sequence<formalism::RequirementView>>&,
                          const urd::Store<urd::sequence<formalism::TypeView>>&,
                          const urd::Store<urd::sequence<formalism::ObjectView>>&,
                          const urd::Store<urd::sequence<formalism::PredicateView>>&,
                          const urd::Store<urd::sequence<formalism::FunctionSkeletonView>>&,
                          const urd::Store<urd::sequence<formalism::ActionView>>&,
                          const urd::Store<urd::sequence<formalism::AxiomView>>&>(),
                 "name"_a,
                 "requirements"_a = urd::Store<urd::sequence<formalism::RequirementView>> {},
                 "types"_a = urd::Store<urd::sequence<formalism::TypeView>> {},
                 "constants"_a = urd::Store<urd::sequence<formalism::ObjectView>> {},
                 "predicates"_a = urd::Store<urd::sequence<formalism::PredicateView>> {},
                 "functions"_a = urd::Store<urd::sequence<formalism::FunctionSkeletonView>> {},
                 "actions"_a = urd::Store<urd::sequence<formalism::ActionView>> {},
                 "axioms"_a = urd::Store<urd::sequence<formalism::AxiomView>> {})
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
        cls.def("get_index", &V::index)
            .def("get_name", &V::get_name)
            .def("get_requirements", &V::get_requirements)
            .def("get_types", &V::get_types)
            .def("get_constants", &V::get_constants)
            .def("get_predicates", &V::get_predicates)
            .def("get_functions", &V::get_functions)
            .def("get_actions", &V::get_actions)
            .def("get_axioms", &V::get_axioms);
        ygg::add_print(cls);
        urd::python::add_comparison(cls);
        urd::python::add_hash(cls);
    }

    bind_insert<formalism::Domain>(repository);
}

}  // namespace loki::formalism
