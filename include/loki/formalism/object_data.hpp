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

#ifndef LOKI_FORMALISM_OBJECT_DATA_HPP_
#define LOKI_FORMALISM_OBJECT_DATA_HPP_

#include "loki/formalism/object_index.hpp"
#include "loki/formalism/type_index.hpp"

#include <cista/containers/string.h>
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
struct Data<::loki::formalism::Object>
{
    ygg::Index<::loki::formalism::Object> index;
    ::cista::offset::string name;
    ygg::IndexList<::loki::formalism::Type> types;

    Data() = default;
    Data(::cista::offset::string name_, ygg::IndexList<::loki::formalism::Type> types_ = {}) : index(), name(std::move(name_)), types(std::move(types_)) {}
    template<typename C>
    Data(const std::string& name_, const std::vector<::ygg::View<ygg::Index<::loki::formalism::Type>, C>>& types_ = {}) : index(), name(name_), types()
    {
        set(types_, types);
    }

    auto cista_members() noexcept { return std::tie(index, name, types); }
    auto cista_members() const noexcept { return std::tie(index, name, types); }
    auto identifying_members() const noexcept { return std::tie(name, types); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}

#endif
