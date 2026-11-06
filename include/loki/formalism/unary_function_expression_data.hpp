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

#ifndef LOKI_FORMALISM_UNARY_FUNCTION_EXPRESSION_DATA_HPP_
#define LOKI_FORMALISM_UNARY_FUNCTION_EXPRESSION_DATA_HPP_

#include "loki/formalism/declarations.hpp"

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
struct data<::loki::formalism::FunctionExpression<::loki::formalism::UnaryTag>>
{
    urd::index<::loki::formalism::FunctionExpression<::loki::formalism::UnaryTag>> index;
    ::loki::formalism::UnaryArithmeticOperator op {};
    ::loki::formalism::FunctionExpressionVariant expression;

    data() = default;
    data(::loki::formalism::UnaryArithmeticOperator op_, ::loki::formalism::FunctionExpressionVariant expression_) :
        index(),
        op(op_),
        expression(expression_)
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::op, &data::expression> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::op, &data::expression> {}; }
};

}

#endif
