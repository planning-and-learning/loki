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

#ifndef LOKI_FORMALISM_ATOM_VIEW_HPP_
#define LOKI_FORMALISM_ATOM_VIEW_HPP_

#include "loki/formalism/atom_data.hpp"

#include <yggdrasil/containers/optional.hpp>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/detail/view.hpp>

namespace ygg
{

template<ygg::formalism::SymbolContextFor<::loki::formalism::Atom> C>
class View<ygg::Index<::loki::formalism::Atom>, C> : public ygg::formalism::detail::View<ygg::Index<::loki::formalism::Atom>, C>
{
public:
    View(ygg::Index<::loki::formalism::Atom> handle, const C& context) noexcept :
        ygg::formalism::detail::View<ygg::Index<::loki::formalism::Atom>, C>(handle, context)
    {
    }

    auto get_predicate() const noexcept { return ygg::make_view(this->get_data().predicate, this->get_context()); }
    auto get_terms() const noexcept { return ygg::make_view(this->get_data().terms, this->get_context()); }
};

}

#endif
