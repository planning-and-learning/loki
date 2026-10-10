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

#ifndef LOKI_FORMALISM_CONDITION_OR_DATA_HPP_
#define LOKI_FORMALISM_CONDITION_OR_DATA_HPP_

#include "loki/formalism/declarations.hpp"

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
struct Data<::loki::formalism::Condition<::loki::formalism::OrTag>>
{
    ygg::Index<::loki::formalism::Condition<::loki::formalism::OrTag>> index;
    ygg::IndexList<::loki::formalism::Condition<>> conditions;

    Data() = default;
    Data(ygg::IndexList<::loki::formalism::Condition<>> conditions_) : index(), conditions(std::move(conditions_)) {}
    template<typename C>
    Data(const std::vector<::ygg::View<ygg::Index<::loki::formalism::Condition<>>, C>>& conditions_) : index(), conditions()
    {
        set(conditions_, conditions);
    }

    auto cista_members() noexcept { return std::tie(index, conditions); }
    auto cista_members() const noexcept { return std::tie(index, conditions); }
    auto identifying_members() const noexcept { return std::tie(conditions); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}

#endif
