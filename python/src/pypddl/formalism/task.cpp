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

#include <loki/formalism/axiom_view.hpp>
#include <loki/formalism/condition_view.hpp>
#include <loki/formalism/domain_view.hpp>
#include <loki/formalism/initial_function_value_view.hpp>
#include <loki/formalism/literal_view.hpp>
#include <loki/formalism/metric_view.hpp>
#include <loki/formalism/object_view.hpp>
#include <loki/formalism/predicate_view.hpp>
#include <loki/formalism/requirement_view.hpp>
#include <loki/formalism/task_view.hpp>

namespace nb = nanobind;
using namespace nb::literals;

namespace loki::formalism
{

void bind_task(nb::module_& m, RepositoryBinding& repository)
{
    ygg::bind_index<ygg::Index<formalism::Task>>(m, "TaskIndex");

    {
        using V = Data<formalism::Task>;
        bind_data<V>(m, "TaskData")
            .def(nb::init<std::string,
                          ygg::Index<formalism::Domain>,
                          ygg::IndexList<formalism::Requirement>,
                          ygg::IndexList<formalism::Object>,
                          ygg::IndexList<formalism::Literal>,
                          ygg::IndexList<formalism::InitialFunctionValue>,
                          cista::optional<ygg::Index<formalism::Condition>>,
                          cista::optional<ygg::Index<formalism::Metric>>,
                          ygg::IndexList<formalism::Predicate>,
                          ygg::IndexList<formalism::Axiom>>(),
                 "name"_a,
                 "domain"_a,
                 "requirements"_a = ygg::IndexList<formalism::Requirement> {},
                 "objects"_a = ygg::IndexList<formalism::Object> {},
                 "initial_literals"_a = ygg::IndexList<formalism::Literal> {},
                 "initial_function_values"_a = ygg::IndexList<formalism::InitialFunctionValue> {},
                 "goal"_a.none() = cista::optional<ygg::Index<formalism::Condition>> {},
                 "metric"_a.none() = cista::optional<ygg::Index<formalism::Metric>> {},
                 "predicates"_a = ygg::IndexList<formalism::Predicate> {},
                 "axioms"_a = ygg::IndexList<formalism::Axiom> {})
            .def(nb::init<const std::string&,
                          formalism::DomainView,
                          const std::vector<formalism::RequirementView>&,
                          const std::vector<formalism::ObjectView>&,
                          const std::vector<formalism::LiteralView>&,
                          const std::vector<formalism::InitialFunctionValueView>&,
                          const std::optional<formalism::ConditionView>&,
                          const std::optional<formalism::MetricView>&,
                          const std::vector<formalism::PredicateView>&,
                          const std::vector<formalism::AxiomView>&>(),
                 "name"_a,
                 "domain"_a,
                 "requirements"_a = std::vector<formalism::RequirementView> {},
                 "objects"_a = std::vector<formalism::ObjectView> {},
                 "initial_literals"_a = std::vector<formalism::LiteralView> {},
                 "initial_function_values"_a = std::vector<formalism::InitialFunctionValueView> {},
                 "goal"_a = std::optional<formalism::ConditionView> {},
                 "metric"_a = std::optional<formalism::MetricView> {},
                 "predicates"_a = std::vector<formalism::PredicateView> {},
                 "axioms"_a = std::vector<formalism::AxiomView> {})
            .def_rw("name", &V::name)
            .def_rw("domain", &V::domain)
            .def_rw("requirements", &V::requirements)
            .def_rw("objects", &V::objects)
            .def_rw("initial_literals", &V::initial_literals)
            .def_rw("initial_function_values", &V::initial_function_values)
            .def_rw("goal", &V::goal)
            .def_rw("metric", &V::metric)
            .def_rw("predicates", &V::predicates)
            .def_rw("axioms", &V::axioms);
    }

    {
        using V = formalism::TaskView;
        auto cls = nb::class_<V>(m, "Task");
        cls.def("get_index", &V::get_index)
            .def("get_name", &V::get_name)
            .def("get_domain", &V::get_domain, nb::keep_alive<0, 1>())
            .def("get_requirements", &V::get_requirements)
            .def("get_objects", &V::get_objects)
            .def("get_initial_literals", &V::get_initial_literals)
            .def("get_initial_function_values", &V::get_initial_function_values)
            .def("get_goal", &V::get_goal, nb::keep_alive<0, 1>())
            .def("get_metric", &V::get_metric, nb::keep_alive<0, 1>())
            .def("get_predicates", &V::get_predicates)
            .def("get_axioms", &V::get_axioms);
        ygg::add_print(cls);
        ygg::add_comparison(cls);
        ygg::add_hash(cls);
    }

    bind_insert<formalism::Task>(repository);
}

}  // namespace loki::formalism
