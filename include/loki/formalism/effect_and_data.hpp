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

#ifndef LOKI_FORMALISM_EFFECT_AND_DATA_HPP_
#define LOKI_FORMALISM_EFFECT_AND_DATA_HPP_

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
struct Data<::loki::formalism::Effect<::loki::formalism::AndTag>>
{
    ygg::Index<::loki::formalism::Effect<::loki::formalism::AndTag>> index;
    ygg::IndexList<::loki::formalism::Effect<>> effects;

    Data() = default;
    Data(ygg::IndexList<::loki::formalism::Effect<>> effects_) : index(), effects(std::move(effects_)) {}
    template<typename C>
    Data(const std::vector<::ygg::View<ygg::Index<::loki::formalism::Effect<>>, C>>& effects_) : index(), effects()
    {
        set(effects_, effects);
    }

    auto cista_members() noexcept { return std::tie(index, effects); }
    auto cista_members() const noexcept { return std::tie(index, effects); }
    auto identifying_members() const noexcept { return std::tie(effects); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}

#endif
