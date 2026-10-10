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

#ifndef LOKI_FORMALISM_PARAMETER_DATA_HPP_
#define LOKI_FORMALISM_PARAMETER_DATA_HPP_

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
struct Data<::loki::formalism::Parameter>
{
    ygg::Index<::loki::formalism::Parameter> index;
    ygg::Index<::loki::formalism::Variable> variable;
    ygg::IndexList<::loki::formalism::Type> types;

    Data() = default;
    Data(ygg::Index<::loki::formalism::Variable> variable_, ygg::IndexList<::loki::formalism::Type> types_ = {}) :
        index(),
        variable(variable_),
        types(std::move(types_))
    {
    }
    template<typename C>
    Data(::ygg::View<ygg::Index<::loki::formalism::Variable>, C> variable_,
         const std::vector<::ygg::View<ygg::Index<::loki::formalism::Type>, C>>& types_ = {}) :
        index(),
        variable(),
        types()
    {
        set(variable_, variable);
        set(types_, types);
    }

    auto cista_members() noexcept { return std::tie(index, variable, types); }
    auto cista_members() const noexcept { return std::tie(index, variable, types); }
    auto identifying_members() const noexcept { return std::tie(variable, types); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}

#endif
