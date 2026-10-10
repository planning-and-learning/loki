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

#ifndef LOKI_FORMALISM_TASK_DATA_HPP_
#define LOKI_FORMALISM_TASK_DATA_HPP_

#include "loki/formalism/axiom_index.hpp"
#include "loki/formalism/condition_index.hpp"
#include "loki/formalism/domain_index.hpp"
#include "loki/formalism/initial_function_value_index.hpp"
#include "loki/formalism/literal_index.hpp"
#include "loki/formalism/metric_index.hpp"
#include "loki/formalism/object_index.hpp"
#include "loki/formalism/predicate_index.hpp"
#include "loki/formalism/requirement_index.hpp"
#include "loki/formalism/task_index.hpp"

#include <cista/containers/optional.h>
#include <cista/containers/string.h>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<::loki::formalism::Task>
{
    ygg::Index<::loki::formalism::Task> index;
    ::cista::offset::string name;
    ygg::Index<::loki::formalism::Domain> domain;
    ygg::IndexList<::loki::formalism::Requirement> requirements;
    ygg::IndexList<::loki::formalism::Object> objects;
    ygg::IndexList<::loki::formalism::Literal> initial_literals;
    ygg::IndexList<::loki::formalism::InitialFunctionValue> initial_function_values;
    ::cista::optional<ygg::Index<::loki::formalism::Condition>> goal;
    ::cista::optional<ygg::Index<::loki::formalism::Metric>> metric;
    ygg::IndexList<::loki::formalism::Predicate> predicates;
    ygg::IndexList<::loki::formalism::Axiom> axioms;

    Data() = default;
    Data(::cista::offset::string name_,
         ygg::Index<::loki::formalism::Domain> domain_,
         ygg::IndexList<::loki::formalism::Requirement> requirements_,
         ygg::IndexList<::loki::formalism::Object> objects_,
         ygg::IndexList<::loki::formalism::Literal> initial_literals_,
         ygg::IndexList<::loki::formalism::InitialFunctionValue> initial_function_values_,
         ::cista::optional<ygg::Index<::loki::formalism::Condition>> goal_,
         ::cista::optional<ygg::Index<::loki::formalism::Metric>> metric_,
         ygg::IndexList<::loki::formalism::Predicate> predicates_,
         ygg::IndexList<::loki::formalism::Axiom> axioms_) :
        index(),
        name(std::move(name_)),
        domain(domain_),
        requirements(std::move(requirements_)),
        objects(std::move(objects_)),
        initial_literals(std::move(initial_literals_)),
        initial_function_values(std::move(initial_function_values_)),
        goal(goal_),
        metric(metric_),
        predicates(std::move(predicates_)),
        axioms(std::move(axioms_))
    {
    }
    template<typename C>
    Data(const std::string& name_,
         ::ygg::View<ygg::Index<::loki::formalism::Domain>, C> domain_,
         const std::vector<::ygg::View<ygg::Index<::loki::formalism::Requirement>, C>>& requirements_,
         const std::vector<::ygg::View<ygg::Index<::loki::formalism::Object>, C>>& objects_,
         const std::vector<::ygg::View<ygg::Index<::loki::formalism::Literal>, C>>& initial_literals_,
         const std::vector<::ygg::View<ygg::Index<::loki::formalism::InitialFunctionValue>, C>>& initial_function_values_,
         const std::optional<::ygg::View<ygg::Index<::loki::formalism::Condition>, C>>& goal_,
         const std::optional<::ygg::View<ygg::Index<::loki::formalism::Metric>, C>>& metric_,
         const std::vector<::ygg::View<ygg::Index<::loki::formalism::Predicate>, C>>& predicates_,
         const std::vector<::ygg::View<ygg::Index<::loki::formalism::Axiom>, C>>& axioms_) :
        index(),
        name(name_),
        domain(),
        requirements(),
        objects(),
        initial_literals(),
        initial_function_values(),
        goal(),
        metric(),
        predicates(),
        axioms()
    {
        set(domain_, domain);
        set(requirements_, requirements);
        set(objects_, objects);
        set(initial_literals_, initial_literals);
        set(initial_function_values_, initial_function_values);
        set(goal_, goal);
        set(metric_, metric);
        set(predicates_, predicates);
        set(axioms_, axioms);
    }

    auto cista_members() noexcept
    {
        return std::tie(index, name, domain, requirements, objects, initial_literals, initial_function_values, goal, metric, predicates, axioms);
    }
    auto cista_members() const noexcept
    {
        return std::tie(index, name, domain, requirements, objects, initial_literals, initial_function_values, goal, metric, predicates, axioms);
    }
    auto identifying_members() const noexcept
    {
        return std::tie(name, domain, requirements, objects, initial_literals, initial_function_values, goal, metric, predicates, axioms);
    }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}

#endif
