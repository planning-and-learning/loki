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

#ifndef LOKI_FORMALISM_TERM_DATA_HPP_
#define LOKI_FORMALISM_TERM_DATA_HPP_

#include "loki/formalism/declarations.hpp"

#include <cista/containers/variant.h>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<::loki::formalism::Term>
{
    using Variant = ::cista::offset::variant<ygg::Index<::loki::formalism::Variable>, ygg::Index<::loki::formalism::Object>>;
    template<typename C>
    using ViewVariant = ::ygg::ViewVariant<Variant, C>;

    ygg::Index<::loki::formalism::Term> index;
    Variant variant;

    Data() = default;
    explicit Data(Variant variant_) : index(), variant(std::move(variant_)) {}
    template<typename C>
    explicit Data(const ViewVariant<C>& variant_) :
        index(),
        variant(std::visit([](const auto& view) -> Variant { return Variant(view.get_index()); }, variant_))
    {
    }

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::variant> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::variant> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

}

#endif
