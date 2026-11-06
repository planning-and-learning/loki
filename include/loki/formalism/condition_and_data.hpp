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

#ifndef LOKI_FORMALISM_CONDITION_AND_DATA_HPP_
#define LOKI_FORMALISM_CONDITION_AND_DATA_HPP_

#include "loki/formalism/declarations.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

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
struct data<::loki::formalism::Condition<::loki::formalism::AndTag>>
{
    urd::index<::loki::formalism::Condition<::loki::formalism::AndTag>> index;
    ::loki::formalism::ConditionList conditions;

    data() = default;
    data(::loki::formalism::ConditionList conditions_) : index(), conditions(std::move(conditions_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::conditions> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::conditions> {}; }
};

}

#endif
