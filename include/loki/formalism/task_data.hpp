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

#include "loki/formalism/declarations.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

#include <cista/containers/optional.h>
#include <cista/containers/string.h>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>
#include <urd/elements/flat.hpp>

namespace urd
{

template<>
struct data<::loki::formalism::Task>
{
    urd::index<::loki::formalism::Task> index;
    ::cista::offset::string name;
    urd::index<::loki::formalism::Domain> domain;
    urd::flat_vector<urd::index<::loki::formalism::Requirement>> requirements;
    urd::flat_vector<urd::index<::loki::formalism::Type>> types;
    urd::flat_vector<urd::index<::loki::formalism::Predicate>> predicates;
    urd::flat_vector<urd::index<::loki::formalism::Object>> objects;
    urd::flat_vector<urd::index<::loki::formalism::Literal>> initial_literals;
    urd::flat_vector<urd::index<::loki::formalism::InitialFunctionValue>> initial_function_values;
    ::cista::optional<::loki::formalism::ConditionVariant> goal;
    ::cista::optional<urd::index<::loki::formalism::Metric>> metric;
    urd::flat_vector<urd::index<::loki::formalism::Axiom>> axioms;

    data() = default;
    data(::cista::offset::string name_,
         urd::index<::loki::formalism::Domain> domain_,
         urd::flat_vector<urd::index<::loki::formalism::Requirement>> requirements_,
         urd::flat_vector<urd::index<::loki::formalism::Type>> types_,
         urd::flat_vector<urd::index<::loki::formalism::Predicate>> predicates_,
         urd::flat_vector<urd::index<::loki::formalism::Object>> objects_,
         urd::flat_vector<urd::index<::loki::formalism::Literal>> initial_literals_,
         urd::flat_vector<urd::index<::loki::formalism::InitialFunctionValue>> initial_function_values_,
         ::cista::optional<::loki::formalism::ConditionVariant> goal_,
         ::cista::optional<urd::index<::loki::formalism::Metric>> metric_,
         urd::flat_vector<urd::index<::loki::formalism::Axiom>> axioms_) :
        index(),
        name(std::move(name_)),
        domain(domain_),
        requirements(std::move(requirements_)),
        types(std::move(types_)),
        predicates(std::move(predicates_)),
        objects(std::move(objects_)),
        initial_literals(std::move(initial_literals_)),
        initial_function_values(std::move(initial_function_values_)),
        goal(std::move(goal_)),
        metric(std::move(metric_)),
        axioms(std::move(axioms_))
    {
    }

    static constexpr auto cista_members() noexcept
    {
        return urd::Members<&data::index,
                            &data::name,
                            &data::domain,
                            &data::requirements,
                            &data::types,
                            &data::predicates,
                            &data::objects,
                            &data::initial_literals,
                            &data::initial_function_values,
                            &data::goal,
                            &data::metric,
                            &data::axioms> {};
    }
    static constexpr auto identifying_members() noexcept
    {
        return urd::Members<&data::name,
                            &data::domain,
                            &data::requirements,
                            &data::types,
                            &data::predicates,
                            &data::objects,
                            &data::initial_literals,
                            &data::initial_function_values,
                            &data::goal,
                            &data::metric,
                            &data::axioms> {};
    }
};

}

#endif
