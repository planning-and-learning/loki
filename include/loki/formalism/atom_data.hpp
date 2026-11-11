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
#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

template<>
struct data<::loki::formalism::Atom>
{
    urd::index<::loki::formalism::Atom> index;
    urd::index<::loki::formalism::Predicate> predicate;
    ::loki::formalism::TermList terms;

    data() = default;
    data(urd::index<::loki::formalism::Predicate> predicate_, ::loki::formalism::TermList terms_ = {}) :
        index(),
        predicate(predicate_),
        terms(std::move(terms_))
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::predicate, &data::terms> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::predicate, &data::terms> {}; }
};

}

#endif
