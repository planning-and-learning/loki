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

#ifndef LOKI_FORMALISM_MULTI_FUNCTION_EXPRESSION_DATA_HPP_
#define LOKI_FORMALISM_MULTI_FUNCTION_EXPRESSION_DATA_HPP_

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

namespace urd
{

template<>
struct data<::loki::formalism::FunctionExpression<::loki::formalism::MultiTag>>
{
    urd::index<::loki::formalism::FunctionExpression<::loki::formalism::MultiTag>> index;
    ::loki::formalism::MultiArithmeticOperator op {};
    ::loki::formalism::FunctionExpressionList args;

    data() = default;
    data(::loki::formalism::MultiArithmeticOperator op_, ::loki::formalism::FunctionExpressionList args_) : index(), op(op_), args(std::move(args_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::op, &data::args> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::op, &data::args> {}; }
};

}

#endif
