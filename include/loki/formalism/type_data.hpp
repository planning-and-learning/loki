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

#ifndef LOKI_FORMALISM_TYPE_DATA_HPP_
#define LOKI_FORMALISM_TYPE_DATA_HPP_

#include "loki/formalism/declarations.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

#include <cista/containers/string.h>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>
#include <urd/elements/flat.hpp>

namespace urd
{

template<>
struct data<::loki::formalism::Type>
{
    urd::index<::loki::formalism::Type> index;
    urd::flat_string name;
    urd::flat_vector<urd::index<::loki::formalism::Type>> bases;

    data() = default;
    data(urd::flat_string name_, urd::flat_vector<urd::index<::loki::formalism::Type>> bases_ = {}) : index(), name(std::move(name_)), bases(std::move(bases_)) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::name, &data::bases> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::name, &data::bases> {}; }
};

}

#endif
