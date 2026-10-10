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

#ifndef LOKI_FORMALISM_DOMAIN_VIEW_HPP_
#define LOKI_FORMALISM_DOMAIN_VIEW_HPP_

#include "loki/formalism/domain_data.hpp"

#include <yggdrasil/containers/optional.hpp>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<ygg::formalism::SymbolContextFor<::loki::formalism::Domain> C>
class View<ygg::Index<::loki::formalism::Domain>, C> : public ygg::IndexViewBase<::loki::formalism::Domain, C>
{
public:
    using ygg::IndexViewBase<::loki::formalism::Domain, C>::IndexViewBase;

    const auto& get_name() const noexcept { return this->get_data().name; }
    auto get_requirements() const noexcept { return ygg::make_view(this->get_data().requirements, this->get_context()); }
    auto get_types() const noexcept { return ygg::make_view(this->get_data().types, this->get_context()); }
    auto get_constants() const noexcept { return ygg::make_view(this->get_data().constants, this->get_context()); }
    auto get_predicates() const noexcept { return ygg::make_view(this->get_data().predicates, this->get_context()); }
    auto get_functions() const noexcept { return ygg::make_view(this->get_data().functions, this->get_context()); }
    auto get_actions() const noexcept { return ygg::make_view(this->get_data().actions, this->get_context()); }
    auto get_axioms() const noexcept { return ygg::make_view(this->get_data().axioms, this->get_context()); }
};

}

#endif
