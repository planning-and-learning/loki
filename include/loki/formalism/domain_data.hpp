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
    urd::index<::loki::formalism::Domain> index;
    urd::flat_string name;
    urd::flat_vector<urd::index<::loki::formalism::Requirement>> requirements;
    urd::flat_vector<urd::index<::loki::formalism::Type>> types;
    urd::flat_vector<urd::index<::loki::formalism::Object>> constants;
    urd::flat_vector<urd::index<::loki::formalism::Predicate>> predicates;
    urd::flat_vector<urd::index<::loki::formalism::FunctionSkeleton>> functions;
    urd::flat_vector<urd::index<::loki::formalism::Action>> actions;
    urd::flat_vector<urd::index<::loki::formalism::Axiom>> axioms;

    data() = default;
    data(urd::flat_string name_,
         urd::flat_vector<urd::index<::loki::formalism::Requirement>> requirements_,
         urd::flat_vector<urd::index<::loki::formalism::Type>> types_,
         urd::flat_vector<urd::index<::loki::formalism::Object>> constants_,
         urd::flat_vector<urd::index<::loki::formalism::Predicate>> predicates_,
         urd::flat_vector<urd::index<::loki::formalism::FunctionSkeleton>> functions_,
         urd::flat_vector<urd::index<::loki::formalism::Action>> actions_,
         urd::flat_vector<urd::index<::loki::formalism::Axiom>> axioms_) :
        index(),
        name(std::move(name_)),
        requirements(std::move(requirements_)),
        types(std::move(types_)),
        constants(std::move(constants_)),
        predicates(std::move(predicates_)),
        functions(std::move(functions_)),
        actions(std::move(actions_)),
        axioms(std::move(axioms_))
    {
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
                            &data::axioms> {};
    }
    static constexpr auto identifying_members() noexcept
    {
        return urd::
            Members<&data::name, &data::requirements, &data::types, &data::constants, &data::predicates, &data::functions, &data::actions, &data::axioms> {};
    }
};

}

#endif
