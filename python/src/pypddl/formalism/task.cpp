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

#include <loki/formalism/axiom_view.hpp>
#include <loki/formalism/domain_view.hpp>
#include <loki/formalism/initial_function_value_view.hpp>
#include <loki/formalism/literal_view.hpp>
#include <loki/formalism/metric_view.hpp>
#include <loki/formalism/object_view.hpp>
#include <loki/formalism/predicate_view.hpp>
#include <loki/formalism/requirement_view.hpp>
#include <loki/formalism/task_view.hpp>

namespace nb = nanobind;

namespace loki::formalism
{

void bind_task(nb::module_& m, RepositoryBinding& repository)
{
    using namespace nb::literals;
    urd::python::bind_index<urd::index<formalism::Task>>(m, "TaskIndex");

    {
        using V = urd::data<formalism::Task>;
        urd::python::bind_data<V>(m, "TaskData")
            .def(nb::init<std::string,
                          urd::index<formalism::Domain>,
                          urd::flat_vector<urd::index<formalism::Requirement>>,
                          urd::flat_vector<urd::index<formalism::Type>>,
                          urd::flat_vector<urd::index<formalism::Predicate>>,
                          urd::flat_vector<urd::index<formalism::Object>>,
                          urd::flat_vector<urd::index<formalism::Literal>>,
                          urd::flat_vector<urd::index<formalism::InitialFunctionValue>>,
                          cista::optional<formalism::ConditionVariant>,
                          cista::optional<urd::index<formalism::Metric>>,
                          urd::flat_vector<urd::index<formalism::Axiom>>>(),
                 "name"_a,
                 "domain"_a,
                 "requirements"_a = urd::flat_vector<urd::index<formalism::Requirement>> {},
                 "types"_a = urd::flat_vector<urd::index<formalism::Type>> {},
                 "predicates"_a = urd::flat_vector<urd::index<formalism::Predicate>> {},
                 "objects"_a = urd::flat_vector<urd::index<formalism::Object>> {},
                 "initial_literals"_a = urd::flat_vector<urd::index<formalism::Literal>> {},
                 "initial_function_values"_a = urd::flat_vector<urd::index<formalism::InitialFunctionValue>> {},
                 "goal"_a.none() = cista::optional<formalism::ConditionVariant> {},
                 "metric"_a.none() = cista::optional<urd::index<formalism::Metric>> {},
                 "axioms"_a = urd::flat_vector<urd::index<formalism::Axiom>> {})
            .def(urd::python::init_from_views<const std::string&,
                          formalism::DomainView,
                          const urd::Store<urd::sequence<formalism::RequirementView>>&,
                          const urd::Store<urd::sequence<formalism::TypeView>>&,
                          const urd::Store<urd::sequence<formalism::PredicateView>>&,
                          const urd::Store<urd::sequence<formalism::ObjectView>>&,
                          const urd::Store<urd::sequence<formalism::LiteralView>>&,
                          const urd::Store<urd::sequence<formalism::InitialFunctionValueView>>&,
                          const std::optional<formalism::ConditionView>&,
                          const std::optional<formalism::MetricView>&,
                          const urd::Store<urd::sequence<formalism::AxiomView>>&>(),
                 "name"_a,
                 "domain"_a,
                 "requirements"_a = urd::Store<urd::sequence<formalism::RequirementView>> {},
                 "types"_a = urd::Store<urd::sequence<formalism::TypeView>> {},
                 "predicates"_a = urd::Store<urd::sequence<formalism::PredicateView>> {},
                 "objects"_a = urd::Store<urd::sequence<formalism::ObjectView>> {},
                 "initial_literals"_a = urd::Store<urd::sequence<formalism::LiteralView>> {},
                 "initial_function_values"_a = urd::Store<urd::sequence<formalism::InitialFunctionValueView>> {},
                 "goal"_a = std::optional<formalism::ConditionView> {},
                 "metric"_a = std::optional<formalism::MetricView> {},
                 "axioms"_a = urd::Store<urd::sequence<formalism::AxiomView>> {})
            .def_rw("name", &V::name)
            .def_rw("domain", &V::domain)
            .def_rw("requirements", &V::requirements)
            .def_rw("types", &V::types)
            .def_rw("predicates", &V::predicates)
            .def_rw("objects", &V::objects)
            .def_rw("initial_literals", &V::initial_literals)
            .def_rw("initial_function_values", &V::initial_function_values)
            .def_rw("goal", &V::goal)
            .def_rw("metric", &V::metric)
            .def_rw("axioms", &V::axioms);
    }

    {
        using V = formalism::TaskView;
        auto cls = nb::class_<V>(m, "Task");
        cls.def("get_index", &V::index)
            .def("get_name", &V::get_name)
            .def("get_domain", &V::get_domain, nb::keep_alive<0, 1>())
            .def("get_requirements", &V::get_requirements)
            .def("get_types", &V::get_types)
            .def("get_predicates", &V::get_predicates)
            .def("get_objects", &V::get_objects)
            .def("get_initial_literals", &V::get_initial_literals)
            .def("get_initial_function_values", &V::get_initial_function_values)
            .def("get_goal", &V::get_goal, nb::keep_alive<0, 1>())
            .def("get_metric", &V::get_metric, nb::keep_alive<0, 1>())
            .def("get_axioms", &V::get_axioms);
        ygg::add_print(cls);
        urd::python::add_comparison(cls);
        urd::python::add_hash(cls);
    }

    bind_insert<formalism::Task>(repository);
}

}  // namespace loki::formalism
