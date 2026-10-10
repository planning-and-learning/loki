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
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<::loki::formalism::FunctionExpression<::loki::formalism::MultiTag>>
{
    ygg::Index<::loki::formalism::FunctionExpression<::loki::formalism::MultiTag>> index;
    ::loki::formalism::MultiArithmeticOperator op {};
    ygg::IndexList<::loki::formalism::FunctionExpression<>> args;

    Data() = default;
    Data(::loki::formalism::MultiArithmeticOperator op_, ygg::IndexList<::loki::formalism::FunctionExpression<>> args_) : index(), op(op_), args(std::move(args_))
    {
    }
    template<typename C>
    Data(::loki::formalism::MultiArithmeticOperator op_, const std::vector<::ygg::View<ygg::Index<::loki::formalism::FunctionExpression<>>, C>>& args_) :
        index(),
        op(op_),
        args()
    {
        set(args_, args);
    }

    auto cista_members() noexcept { return std::tie(index, op, args); }
    auto cista_members() const noexcept { return std::tie(index, op, args); }
    auto identifying_members() const noexcept { return std::tie(op, args); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}

#endif
