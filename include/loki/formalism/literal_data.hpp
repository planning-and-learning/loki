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

#ifndef LOKI_FORMALISM_LITERAL_DATA_HPP_
#define LOKI_FORMALISM_LITERAL_DATA_HPP_

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
struct Data<::loki::formalism::Literal>
{
    ygg::Index<::loki::formalism::Literal> index;
    ygg::Index<::loki::formalism::Atom> atom;
    bool m_polarity = true;

    Data() = default;
    Data(ygg::Index<::loki::formalism::Atom> atom_, bool polarity_ = true) : index(), atom(atom_), m_polarity(polarity_) {}
    template<typename C>
    Data(::ygg::View<ygg::Index<::loki::formalism::Atom>, C> atom_, bool polarity_ = true) : index(), atom(), m_polarity(polarity_)
    { set(atom_, atom); }

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::atom, &Data::m_polarity> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::atom, &Data::m_polarity> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

}

#endif
