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

#ifndef LOKI_FORMALISM_DOMAIN_BUILDER_HPP_
#define LOKI_FORMALISM_DOMAIN_BUILDER_HPP_

#include "loki/formalism/domain_data.hpp"

#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>

namespace urd
{

/// The builder of Domain: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<::loki::formalism::Domain, C>
{
    urd::key<::loki::formalism::Domain> index;
    urd::built_t<urd::flat_string, C> name;
    urd::built_t<urd::flat_vector<urd::index<::loki::formalism::Requirement>>, C> requirements;
    urd::built_t<urd::flat_vector<urd::index<::loki::formalism::Type>>, C> types;
    urd::built_t<urd::flat_vector<urd::index<::loki::formalism::Object>>, C> constants;
    urd::built_t<urd::flat_vector<urd::index<::loki::formalism::Predicate>>, C> predicates;
    urd::built_t<urd::flat_vector<urd::index<::loki::formalism::FunctionSkeleton>>, C> functions;
    urd::built_t<urd::flat_vector<urd::index<::loki::formalism::Action>>, C> actions;
    urd::built_t<urd::flat_vector<urd::index<::loki::formalism::DurativeAction>>, C> durative_actions;
    urd::built_t<urd::flat_vector<urd::index<::loki::formalism::Axiom>>, C> axioms;
    urd::built_t<urd::flat_optional<::loki::formalism::ConstraintVariant>, C> constraints;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"name", &builder::name>,
                           urd::field<"requirements", &builder::requirements>,
                           urd::field<"types", &builder::types>,
                           urd::field<"constants", &builder::constants>,
                           urd::field<"predicates", &builder::predicates>,
                           urd::field<"functions", &builder::functions>,
                           urd::field<"actions", &builder::actions>,
                           urd::field<"durative_actions", &builder::durative_actions>,
                           urd::field<"axioms", &builder::axioms>,
                           urd::field<"constraints", &builder::constraints>> {};
    }

    static constexpr auto identifying_members() noexcept
    {
        return urd::Members<&builder::name,
                            &builder::requirements,
                            &builder::types,
                            &builder::constants,
                            &builder::predicates,
                            &builder::functions,
                            &builder::actions,
                            &builder::durative_actions,
                            &builder::axioms,
                            &builder::constraints> {};
    }
};

}

#endif
