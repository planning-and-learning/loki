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

#ifndef LOKI_FORMALISM_TASK_BUILDER_HPP_
#define LOKI_FORMALISM_TASK_BUILDER_HPP_

#include "loki/formalism/task_data.hpp"

#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>

namespace urd
{

/// The builder of Task: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<::loki::formalism::Task, C>
{
    urd::key<::loki::formalism::Task> index;
    urd::built_t<urd::flat_string, C> name;
    urd::built_t<urd::index<::loki::formalism::Domain>, C> domain;
    urd::built_t<urd::flat_vector<urd::index<::loki::formalism::Requirement>>, C> requirements;
    urd::built_t<urd::flat_vector<urd::index<::loki::formalism::Type>>, C> types;
    urd::built_t<urd::flat_vector<urd::index<::loki::formalism::Predicate>>, C> predicates;
    urd::built_t<urd::flat_vector<urd::index<::loki::formalism::Object>>, C> objects;
    urd::built_t<urd::flat_vector<urd::index<::loki::formalism::Literal>>, C> initial_literals;
    urd::built_t<urd::flat_vector<urd::index<::loki::formalism::InitialFunctionValue>>, C> initial_function_values;
    urd::built_t<urd::flat_vector<urd::index<::loki::formalism::TimedInitialLiteral>>, C> timed_initial_literals;
    urd::built_t<urd::flat_vector<urd::index<::loki::formalism::Effect<::loki::formalism::ProbabilisticTag>>>, C> initial_effects;
    urd::built_t<urd::flat_optional<::loki::formalism::ConditionVariant>, C> goal;
    urd::built_t<urd::flat_optional<double>, C> goal_reward;
    urd::built_t<urd::flat_optional<urd::index<::loki::formalism::Metric>>, C> metric;
    urd::built_t<urd::flat_vector<urd::index<::loki::formalism::Axiom>>, C> axioms;
    urd::built_t<urd::flat_optional<::loki::formalism::ConstraintVariant>, C> constraints;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"name", &builder::name>,
                           urd::field<"domain", &builder::domain>,
                           urd::field<"requirements", &builder::requirements>,
                           urd::field<"types", &builder::types>,
                           urd::field<"predicates", &builder::predicates>,
                           urd::field<"objects", &builder::objects>,
                           urd::field<"initial_literals", &builder::initial_literals>,
                           urd::field<"initial_function_values", &builder::initial_function_values>,
                           urd::field<"timed_initial_literals", &builder::timed_initial_literals>,
                           urd::field<"initial_effects", &builder::initial_effects>,
                           urd::field<"goal", &builder::goal>,
                           urd::field<"goal_reward", &builder::goal_reward>,
                           urd::field<"metric", &builder::metric>,
                           urd::field<"axioms", &builder::axioms>,
                           urd::field<"constraints", &builder::constraints>> {};
    }

    static constexpr auto identifying_members() noexcept
    {
        return urd::Members<&builder::name,
                            &builder::domain,
                            &builder::requirements,
                            &builder::types,
                            &builder::predicates,
                            &builder::objects,
                            &builder::initial_literals,
                            &builder::initial_function_values,
                            &builder::timed_initial_literals,
                            &builder::initial_effects,
                            &builder::goal,
                            &builder::goal_reward,
                            &builder::metric,
                            &builder::axioms,
                            &builder::constraints> {};
    }
};

}

#endif
