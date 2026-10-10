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

#ifndef LOKI_FORMALISM_BINARY_FUNCTION_EXPRESSION_DATA_HPP_
#define LOKI_FORMALISM_BINARY_FUNCTION_EXPRESSION_DATA_HPP_

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
struct Data<::loki::formalism::FunctionExpression<::loki::formalism::BinaryTag>>
{
    ygg::Index<::loki::formalism::FunctionExpression<::loki::formalism::BinaryTag>> index;
    ::loki::formalism::BinaryArithmeticOperator op {};
    ygg::Index<::loki::formalism::FunctionExpression<>> left;
    ygg::Index<::loki::formalism::FunctionExpression<>> right;

    Data() = default;
    Data(::loki::formalism::BinaryArithmeticOperator op_,
         ygg::Index<::loki::formalism::FunctionExpression<>> left_,
         ygg::Index<::loki::formalism::FunctionExpression<>> right_) :
        index(),
        op(op_),
        left(left_),
        right(right_)
    {
    }
    template<typename C>
    Data(::loki::formalism::BinaryArithmeticOperator op_,
         ::ygg::View<ygg::Index<::loki::formalism::FunctionExpression<>>, C> left_,
         ::ygg::View<ygg::Index<::loki::formalism::FunctionExpression<>>, C> right_) :
        index(),
        op(op_),
        left(),
        right()
    {
        set(left_, left);
        set(right_, right);
    }

    auto cista_members() noexcept { return std::tie(index, op, left, right); }
    auto cista_members() const noexcept { return std::tie(index, op, left, right); }
    auto identifying_members() const noexcept { return std::tie(op, left, right); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}

#endif
