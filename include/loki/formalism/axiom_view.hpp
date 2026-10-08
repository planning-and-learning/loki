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

#ifndef LOKI_FORMALISM_AXIOM_VIEW_HPP_
#define LOKI_FORMALISM_AXIOM_VIEW_HPP_

#include "loki/formalism/axiom_data.hpp"

#include <yggdrasil/containers/optional.hpp>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/detail/view.hpp>

namespace ygg
{

template<ygg::formalism::SymbolContextFor<::loki::formalism::Axiom> C>
class View<ygg::Index<::loki::formalism::Axiom>, C> : public ygg::formalism::detail::View<ygg::Index<::loki::formalism::Axiom>, C>
{
public:
    View(ygg::Index<::loki::formalism::Axiom> handle, const C& context) noexcept :
        ygg::formalism::detail::View<ygg::Index<::loki::formalism::Axiom>, C>(handle, context)
    {
    }

    auto get_parameters() const noexcept { return ygg::make_view(this->get_data().parameters, this->get_context()); }
    auto get_arity() const noexcept { return this->get_data().parameters.size(); }
    auto get_original_arity() const noexcept { return this->get_data().original_arity; }
    auto get_head() const noexcept { return ygg::make_view(this->get_data().head, this->get_context()); }
    auto get_condition() const noexcept { return ygg::make_view(this->get_data().condition, this->get_context()); }
};

}

#endif
