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
#include <urd/elements/flat.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

template<>
struct data<::loki::formalism::FunctionExpression<::loki::formalism::BinaryTag>>
{
    urd::index<::loki::formalism::FunctionExpression<::loki::formalism::BinaryTag>> index;
    ::loki::formalism::BinaryArithmeticOperator op {};
    ::loki::formalism::FunctionExpressionVariant left;
    ::loki::formalism::FunctionExpressionVariant right;

    data() = default;
    data(::loki::formalism::BinaryArithmeticOperator op_,
         ::loki::formalism::FunctionExpressionVariant left_,
         ::loki::formalism::FunctionExpressionVariant right_) :
        index(),
        op(op_),
        left(std::move(left_)),
        right(std::move(right_))
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::op, &data::left, &data::right> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::op, &data::left, &data::right> {}; }
};

}

#endif
