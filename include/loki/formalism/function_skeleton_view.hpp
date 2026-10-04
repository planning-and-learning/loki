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

#ifndef LOKI_FORMALISM_FUNCTION_SKELETON_VIEW_HPP_
#define LOKI_FORMALISM_FUNCTION_SKELETON_VIEW_HPP_

#include "loki/formalism/function_skeleton_data.hpp"

#include <yggdrasil/containers/optional.hpp>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/detail/view.hpp>

namespace ygg
{

template<ygg::formalism::SymbolContextFor<::loki::formalism::FunctionSkeleton> C>
class View<ygg::Index<::loki::formalism::FunctionSkeleton>, C> : public ygg::formalism::detail::View<ygg::Index<::loki::formalism::FunctionSkeleton>, C>
{
public:
    View(ygg::Index<::loki::formalism::FunctionSkeleton> handle, const C& context) noexcept :
        ygg::formalism::detail::View<ygg::Index<::loki::formalism::FunctionSkeleton>, C>(handle, context)
    {
    }

    const auto& get_name() const noexcept { return this->get_data().name; }
    auto get_parameters() const noexcept { return ygg::make_view(this->get_data().parameters, this->get_context()); }
    auto get_arity() const noexcept { return this->get_data().parameters.size(); }
    auto get_type() const noexcept { return ygg::make_view(this->get_data().type, this->get_context()); }
};

}

#endif
