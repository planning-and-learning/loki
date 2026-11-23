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

#ifndef LOKI_FORMALISM_EFFECT_ONE_OF_DATA_HPP_
#define LOKI_FORMALISM_EFFECT_ONE_OF_DATA_HPP_

#include "loki/formalism/declarations.hpp"
#include "loki/formalism/effects.hpp"

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
struct data<::loki::formalism::Effect<::loki::formalism::OneOfTag>>
{
    urd::key<::loki::formalism::Effect<::loki::formalism::OneOfTag>> index;
    ::loki::formalism::EffectList effects;

    data() = default;

    data(::loki::formalism::EffectList effects_) : index(), effects(std::move(effects_)) {}

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"effects", &data::effects>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&data::effects> {}; }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::effects> {}; }

    [[nodiscard]] urd::key<::loki::formalism::Effect<::loki::formalism::OneOfTag>> key() const noexcept
    { return urd::key<::loki::formalism::Effect<::loki::formalism::OneOfTag>>(index); }
};

}

#endif
