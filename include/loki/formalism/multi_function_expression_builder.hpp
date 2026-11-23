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

#ifndef LOKI_FORMALISM_MULTI_FUNCTION_EXPRESSION_BUILDER_HPP_
#define LOKI_FORMALISM_MULTI_FUNCTION_EXPRESSION_BUILDER_HPP_

#include "loki/formalism/multi_function_expression_data.hpp"

#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>

namespace urd
{

/// The builder of FunctionExpression<MultiTag>: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<::loki::formalism::FunctionExpression<::loki::formalism::MultiTag>, C>
{
    urd::key<::loki::formalism::FunctionExpression<::loki::formalism::MultiTag>> index;
    urd::built_t<::loki::formalism::MultiArithmeticOperator, C> operation {};
    urd::built_t<::loki::formalism::FunctionExpressionList, C> args;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"operation", &builder::operation>, urd::field<"args", &builder::args>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::operation, &builder::args> {}; }
};

}

#endif
