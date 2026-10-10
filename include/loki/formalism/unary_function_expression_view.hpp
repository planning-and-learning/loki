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

#ifndef LOKI_FORMALISM_UNARY_FUNCTION_EXPRESSION_VIEW_HPP_
#define LOKI_FORMALISM_UNARY_FUNCTION_EXPRESSION_VIEW_HPP_

#include "loki/formalism/unary_function_expression_data.hpp"

#include <yggdrasil/containers/optional.hpp>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<ygg::formalism::SymbolContextFor<::loki::formalism::FunctionExpression<::loki::formalism::UnaryTag>> C>
class View<ygg::Index<::loki::formalism::FunctionExpression<::loki::formalism::UnaryTag>>, C> :
    public ygg::IndexViewBase<::loki::formalism::FunctionExpression<::loki::formalism::UnaryTag>, C>
{
public:
    using ygg::IndexViewBase<::loki::formalism::FunctionExpression<::loki::formalism::UnaryTag>, C>::IndexViewBase;

    auto get_operator() const noexcept { return this->get_data().op; }
    auto get_expression() const noexcept { return ygg::make_view(this->get_data().expression, this->get_context()); }
};

}

#endif
