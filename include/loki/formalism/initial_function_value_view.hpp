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

#ifndef LOKI_FORMALISM_INITIAL_FUNCTION_VALUE_VIEW_HPP_
#define LOKI_FORMALISM_INITIAL_FUNCTION_VALUE_VIEW_HPP_

#include "loki/formalism/initial_function_value_data.hpp"

#include <yggdrasil/containers/optional.hpp>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/detail/view.hpp>

namespace ygg
{

template<ygg::formalism::SymbolContextFor<::loki::formalism::InitialFunctionValue> C>
class View<ygg::Index<::loki::formalism::InitialFunctionValue>, C> : public ygg::formalism::detail::View<ygg::Index<::loki::formalism::InitialFunctionValue>, C>
{
public:
    View(ygg::Index<::loki::formalism::InitialFunctionValue> handle, const C& context) noexcept :
        ygg::formalism::detail::View<ygg::Index<::loki::formalism::InitialFunctionValue>, C>(handle, context)
    {
    }

    auto get_function() const noexcept { return ygg::make_view(this->get_data().function, this->get_context()); }
    auto get_value() const noexcept { return ygg::make_view(this->get_data().value, this->get_context()); }
};

}

#endif
