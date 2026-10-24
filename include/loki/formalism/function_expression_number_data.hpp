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

#ifndef LOKI_FORMALISM_FUNCTION_EXPRESSION_NUMBER_DATA_HPP_
#define LOKI_FORMALISM_FUNCTION_EXPRESSION_NUMBER_DATA_HPP_

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
struct Data<::loki::formalism::FunctionExpression<::loki::formalism::NumberTag>>
{
    ygg::Index<::loki::formalism::FunctionExpression<::loki::formalism::NumberTag>> index;
    double value = 0.0;

    Data() = default;
    Data(double value_) : index(), value(value_) {}

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::value> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::value> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

}

#endif
