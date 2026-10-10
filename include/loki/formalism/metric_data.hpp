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

#ifndef LOKI_FORMALISM_METRIC_DATA_HPP_
#define LOKI_FORMALISM_METRIC_DATA_HPP_

#include "loki/formalism/enums.hpp"
#include "loki/formalism/function_expression_index.hpp"
#include "loki/formalism/metric_index.hpp"

#include <tuple>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<::loki::formalism::Metric>
{
    ygg::Index<::loki::formalism::Metric> index;
    ::loki::formalism::OptimizationDirection optimization_direction = ::loki::formalism::OptimizationDirection::Minimize;
    ygg::Index<::loki::formalism::FunctionExpression> expression;

    Data() = default;
    Data(::loki::formalism::OptimizationDirection optimization_direction_, ygg::Index<::loki::formalism::FunctionExpression> expression_) :
        index(),
        optimization_direction(optimization_direction_),
        expression(expression_)
    {
    }
    template<typename C>
    Data(::loki::formalism::OptimizationDirection optimization_direction_, ::ygg::View<ygg::Index<::loki::formalism::FunctionExpression>, C> expression_) :
        index(),
        optimization_direction(optimization_direction_),
        expression()
    {
        set(expression_, expression);
    }

    auto cista_members() noexcept { return std::tie(index, optimization_direction, expression); }
    auto cista_members() const noexcept { return std::tie(index, optimization_direction, expression); }
    auto identifying_members() const noexcept { return std::tie(optimization_direction, expression); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}

#endif
