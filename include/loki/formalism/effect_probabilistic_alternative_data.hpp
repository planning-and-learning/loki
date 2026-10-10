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

#ifndef LOKI_FORMALISM_EFFECT_PROBABILISTIC_ALTERNATIVE_DATA_HPP_
#define LOKI_FORMALISM_EFFECT_PROBABILISTIC_ALTERNATIVE_DATA_HPP_

#include "loki/formalism/effect_index.hpp"
#include "loki/formalism/effect_probabilistic_alternative_index.hpp"

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
struct Data<::loki::formalism::EffectProbabilisticAlternative>
{
    ygg::Index<::loki::formalism::EffectProbabilisticAlternative> index;
    double probability = 0.0;
    ygg::Index<::loki::formalism::Effect> effect;

    Data() = default;
    Data(double probability_, ygg::Index<::loki::formalism::Effect> effect_) : index(), probability(probability_), effect(effect_) {}
    template<typename C>
    Data(double probability_, ::ygg::View<ygg::Index<::loki::formalism::Effect>, C> effect_) : index(), probability(probability_), effect()
    {
        set(effect_, effect);
    }

    auto cista_members() noexcept { return std::tie(index, probability, effect); }
    auto cista_members() const noexcept { return std::tie(index, probability, effect); }
    auto identifying_members() const noexcept { return std::tie(probability, effect); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}

#endif
