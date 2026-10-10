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

#ifndef LOKI_FORMALISM_ATOM_DATA_HPP_
#define LOKI_FORMALISM_ATOM_DATA_HPP_

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
struct Data<::loki::formalism::Atom>
{
    ygg::Index<::loki::formalism::Atom> index;
    ygg::Index<::loki::formalism::Predicate> predicate;
    ygg::IndexList<::loki::formalism::Term> terms;

    Data() = default;
    Data(ygg::Index<::loki::formalism::Predicate> predicate_, ygg::IndexList<::loki::formalism::Term> terms_ = {}) :
        index(),
        predicate(predicate_),
        terms(std::move(terms_))
    {
    }
    template<typename C>
    Data(::ygg::View<ygg::Index<::loki::formalism::Predicate>, C> predicate_,
         const std::vector<::ygg::View<ygg::Index<::loki::formalism::Term>, C>>& terms_ = {}) :
        index(),
        predicate(),
        terms()
    {
        set(predicate_, predicate);
        set(terms_, terms);
    }

    auto cista_members() noexcept { return std::tie(index, predicate, terms); }
    auto cista_members() const noexcept { return std::tie(index, predicate, terms); }
    auto identifying_members() const noexcept { return std::tie(predicate, terms); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}

#endif
