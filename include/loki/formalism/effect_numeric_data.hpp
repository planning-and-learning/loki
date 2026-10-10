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

#ifndef LOKI_FORMALISM_EFFECT_NUMERIC_DATA_HPP_
#define LOKI_FORMALISM_EFFECT_NUMERIC_DATA_HPP_

#include "loki/formalism/declarations.hpp"
#include "loki/formalism/effect_numeric_index.hpp"
#include "loki/formalism/function_expression_index.hpp"
#include "loki/formalism/function_term_index.hpp"

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
struct Data<::loki::formalism::EffectNumeric>
{
    ygg::Index<::loki::formalism::EffectNumeric> index;
    ::loki::formalism::NumericEffectOperator op {};
    ygg::Index<::loki::formalism::FunctionTerm> function;
    ygg::Index<::loki::formalism::FunctionExpression> expression;

    Data() = default;
    Data(::loki::formalism::NumericEffectOperator op_,
         ygg::Index<::loki::formalism::FunctionTerm> function_,
         ygg::Index<::loki::formalism::FunctionExpression> expression_) :
        index(),
        op(op_),
        function(function_),
        expression(expression_)
    {
    }
    template<typename C>
    Data(::loki::formalism::NumericEffectOperator op_,
         ::ygg::View<ygg::Index<::loki::formalism::FunctionTerm>, C> function_,
         ::ygg::View<ygg::Index<::loki::formalism::FunctionExpression>, C> expression_) :
        index(),
        op(op_),
        function(),
        expression()
    {
        set(function_, function);
        set(expression_, expression);
    }

    auto cista_members() noexcept { return std::tie(index, op, function, expression); }
    auto cista_members() const noexcept { return std::tie(index, op, function, expression); }
    auto identifying_members() const noexcept { return std::tie(op, function, expression); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}

#endif
