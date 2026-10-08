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

#ifndef LOKI_FORMALISM_TERM_VIEW_HPP_
#define LOKI_FORMALISM_TERM_VIEW_HPP_

#include "loki/formalism/term_data.hpp"

#include <yggdrasil/containers/optional.hpp>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/detail/view.hpp>

namespace ygg
{

template<ygg::formalism::SymbolContextFor<::loki::formalism::Term> C>
class View<ygg::Index<::loki::formalism::Term>, C> : public ygg::formalism::detail::View<ygg::Index<::loki::formalism::Term>, C>
{
public:
    View(ygg::Index<::loki::formalism::Term> handle, const C& context) noexcept :
        ygg::formalism::detail::View<ygg::Index<::loki::formalism::Term>, C>(handle, context)
    {
    }

    auto get_variant() const noexcept { return ygg::make_view(this->get_data().variant, this->get_context()); }
};

}

#endif
