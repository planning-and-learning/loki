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

#ifndef LOKI_FORMALISM_DOMAIN_DATA_HPP_
#define LOKI_FORMALISM_DOMAIN_DATA_HPP_

#include "loki/formalism/constraints.hpp"
#include "loki/formalism/declarations.hpp"

#include <cista/containers/string.h>
#include <optional>
#include <string>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

template<>
struct data<::loki::formalism::Domain>
{
    urd::key<::loki::formalism::Domain> index;
    urd::flat_string name;
    urd::flat_vector<urd::index<::loki::formalism::Requirement>> requirements;
    urd::flat_vector<urd::index<::loki::formalism::Type>> types;
    urd::flat_vector<urd::index<::loki::formalism::Object>> constants;
    urd::flat_vector<urd::index<::loki::formalism::Predicate>> predicates;
    urd::flat_vector<urd::index<::loki::formalism::FunctionSkeleton>> functions;
    urd::flat_vector<urd::index<::loki::formalism::Action>> actions;
    urd::flat_vector<urd::index<::loki::formalism::DurativeAction>> durative_actions;
    urd::flat_vector<urd::index<::loki::formalism::Axiom>> axioms;
    urd::flat_optional<::loki::formalism::ConstraintVariant> constraints;

    data() = default;

    data(urd::flat_string name_,
         urd::flat_vector<urd::index<::loki::formalism::Requirement>> requirements_,
         urd::flat_vector<urd::index<::loki::formalism::Type>> types_,
         urd::flat_vector<urd::index<::loki::formalism::Object>> constants_,
         urd::flat_vector<urd::index<::loki::formalism::Predicate>> predicates_,
         urd::flat_vector<urd::index<::loki::formalism::FunctionSkeleton>> functions_,
         urd::flat_vector<urd::index<::loki::formalism::Action>> actions_,
         urd::flat_vector<urd::index<::loki::formalism::DurativeAction>> durative_actions_,
         urd::flat_vector<urd::index<::loki::formalism::Axiom>> axioms_,
         urd::flat_optional<::loki::formalism::ConstraintVariant> constraints_) :
        index(),
        name(std::move(name_)),
        requirements(std::move(requirements_)),
        types(std::move(types_)),
        constants(std::move(constants_)),
        predicates(std::move(predicates_)),
        functions(std::move(functions_)),
        actions(std::move(actions_)),
        durative_actions(std::move(durative_actions_)),
        axioms(std::move(axioms_)),
        constraints(std::move(constraints_))
    {
    }

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) noexcept = default;
    data& operator=(data&& other) noexcept = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"name", &data::name>,
                           urd::field<"requirements", &data::requirements>,
                           urd::field<"types", &data::types>,
                           urd::field<"constants", &data::constants>,
                           urd::field<"predicates", &data::predicates>,
                           urd::field<"functions", &data::functions>,
                           urd::field<"actions", &data::actions>,
                           urd::field<"durative_actions", &data::durative_actions>,
                           urd::field<"axioms", &data::axioms>,
                           urd::field<"constraints", &data::constraints>> {};
    }

    static constexpr auto identifying_members() noexcept
    {
        return urd::Members<&data::name,
                            &data::requirements,
                            &data::types,
                            &data::constants,
                            &data::predicates,
                            &data::functions,
                            &data::actions,
                            &data::durative_actions,
                            &data::axioms,
                            &data::constraints> {};
    }

    static constexpr auto cista_members() noexcept
    {
        return urd::Members<&data::index,
                            &data::name,
                            &data::requirements,
                            &data::types,
                            &data::constants,
                            &data::predicates,
                            &data::functions,
                            &data::actions,
                            &data::durative_actions,
                            &data::axioms,
                            &data::constraints> {};
    }

    [[nodiscard]] urd::key<::loki::formalism::Domain> key() const noexcept { return urd::key<::loki::formalism::Domain>(index); }
};

}

#endif
