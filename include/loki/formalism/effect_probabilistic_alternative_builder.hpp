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

#ifndef LOKI_FORMALISM_EFFECT_PROBABILISTIC_ALTERNATIVE_BUILDER_HPP_
#define LOKI_FORMALISM_EFFECT_PROBABILISTIC_ALTERNATIVE_BUILDER_HPP_

#include "loki/formalism/effect_probabilistic_alternative_data.hpp"

#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>

namespace urd
{

/// The builder of EffectProbabilisticAlternative: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<::loki::formalism::EffectProbabilisticAlternative, C>
{
    urd::index<::loki::formalism::EffectProbabilisticAlternative> index;
    urd::built_t<double, C> probability = 0.0;
    urd::built_t<::loki::formalism::EffectVariant, C> effect;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"probability", &builder::probability>, urd::field<"effect", &builder::effect>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::probability, &builder::effect> {}; }
};

}

#endif
