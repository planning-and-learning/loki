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

#include "loki/formalism/declarations.hpp"
#include "loki/formalism/enums.hpp"

#include <tuple>
#include <utility>
#include <urd/elements/flat.hpp>

namespace urd
{

template<>
struct data<::loki::formalism::Metric>
{
    urd::index<::loki::formalism::Metric> index;
    ::loki::formalism::OptimizationDirection optimization_direction = ::loki::formalism::OptimizationDirection::Minimize;
    ::loki::formalism::FunctionExpressionVariant expression;

    data() = default;
    data(::loki::formalism::OptimizationDirection optimization_direction_, ::loki::formalism::FunctionExpressionVariant expression_) :
        index(),
        optimization_direction(optimization_direction_),
        expression(expression_)
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::optimization_direction, &data::expression> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::optimization_direction, &data::expression> {}; }
};

}

#endif
