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

#ifndef LOKI_FORMALISM_CONDITION_IMPLY_DATA_HPP_
#define LOKI_FORMALISM_CONDITION_IMPLY_DATA_HPP_

#include "loki/formalism/condition_imply_index.hpp"
#include "loki/formalism/condition_index.hpp"

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
struct Data<::loki::formalism::ConditionImply>
{
    ygg::Index<::loki::formalism::ConditionImply> index;
    ygg::Index<::loki::formalism::Condition> left;
    ygg::Index<::loki::formalism::Condition> right;

    Data() = default;
    Data(ygg::Index<::loki::formalism::Condition> left_, ygg::Index<::loki::formalism::Condition> right_) : index(), left(left_), right(right_) {}
    template<typename C>
    Data(::ygg::View<ygg::Index<::loki::formalism::Condition>, C> left_, ::ygg::View<ygg::Index<::loki::formalism::Condition>, C> right_) :
        index(),
        left(),
        right()
    {
        set(left_, left);
        set(right_, right);
    }

    auto cista_members() noexcept { return std::tie(index, left, right); }
    auto cista_members() const noexcept { return std::tie(index, left, right); }
    auto identifying_members() const noexcept { return std::tie(left, right); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}

#endif
