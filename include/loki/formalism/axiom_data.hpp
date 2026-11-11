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

#ifndef LOKI_FORMALISM_AXIOM_DATA_HPP_
#define LOKI_FORMALISM_AXIOM_DATA_HPP_

#include "loki/formalism/declarations.hpp"

#include <optional>
#include <string>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/core/config.hpp>

namespace urd
{

template<>
struct data<::loki::formalism::Axiom>
{
    urd::index<::loki::formalism::Axiom> index;
    urd::flat_vector<urd::index<::loki::formalism::Parameter>> parameters;
    ygg::uint_t original_arity = 0;
    urd::index<::loki::formalism::Literal> head;
    ::loki::formalism::ConditionVariant condition;

    data() = default;
    data(urd::flat_vector<urd::index<::loki::formalism::Parameter>> parameters_,
         urd::index<::loki::formalism::Literal> head_,
         ::loki::formalism::ConditionVariant condition_) :
        index(),
        parameters(std::move(parameters_)),
        original_arity(parameters.size()),
        head(head_),
        condition(std::move(condition_))
    {
    }
    data(urd::flat_vector<urd::index<::loki::formalism::Parameter>> parameters_,
         ygg::uint_t original_arity_,
         urd::index<::loki::formalism::Literal> head_,
         ::loki::formalism::ConditionVariant condition_) :
        index(),
        parameters(std::move(parameters_)),
        original_arity(original_arity_),
        head(head_),
        condition(std::move(condition_))
    {
    }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::index, &data::parameters, &data::original_arity, &data::head, &data::condition> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::parameters, &data::original_arity, &data::head, &data::condition> {}; }
};

}

#endif
