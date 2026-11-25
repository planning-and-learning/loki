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

#ifndef LOKI_FORMALISM_DURATIVE_ACTION_DATA_HPP_
#define LOKI_FORMALISM_DURATIVE_ACTION_DATA_HPP_

#include "loki/formalism/conditions.hpp"
#include "loki/formalism/declarations.hpp"
#include "loki/formalism/effects.hpp"

#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <yggdrasil/core/config.hpp>

namespace urd
{

/// A durative action: its duration constraint, its timed conditions and its timed effects.
template<>
struct data<::loki::formalism::DurativeAction>
{
    urd::key<::loki::formalism::DurativeAction> index;
    urd::flat_string name;
    urd::flat_string original_name;
    urd::flat_vector<urd::index<::loki::formalism::Parameter>> parameters;
    ygg::uint_t original_arity = 0;
    urd::flat_optional<::loki::formalism::ConditionVariant> duration;
    urd::flat_optional<::loki::formalism::ConditionVariant> condition;
    urd::flat_optional<::loki::formalism::EffectVariant> effect;

    data() = default;

    data(urd::flat_string name_,
         urd::flat_string original_name_,
         urd::flat_vector<urd::index<::loki::formalism::Parameter>> parameters_,
         ygg::uint_t original_arity_,
         urd::flat_optional<::loki::formalism::ConditionVariant> duration_,
         urd::flat_optional<::loki::formalism::ConditionVariant> condition_,
         urd::flat_optional<::loki::formalism::EffectVariant> effect_) :
        index(),
        name(std::move(name_)),
        original_name(std::move(original_name_)),
        parameters(std::move(parameters_)),
        original_arity(std::move(original_arity_)),
        duration(std::move(duration_)),
        condition(std::move(condition_)),
        effect(std::move(effect_))
    {
    }

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"name", &data::name>,
                           urd::field<"original_name", &data::original_name>,
                           urd::field<"parameters", &data::parameters>,
                           urd::field<"original_arity", &data::original_arity>,
                           urd::field<"duration", &data::duration>,
                           urd::field<"condition", &data::condition>,
                           urd::field<"effect", &data::effect>> {};
    }

    static constexpr auto identifying_members() noexcept
    { return urd::Members<&data::original_name, &data::parameters, &data::original_arity, &data::duration, &data::condition, &data::effect> {}; }

    static constexpr auto cista_members() noexcept
    {
        return urd::Members<&data::index,
                            &data::name,
                            &data::original_name,
                            &data::parameters,
                            &data::original_arity,
                            &data::duration,
                            &data::condition,
                            &data::effect> {};
    }

    [[nodiscard]] urd::key<::loki::formalism::DurativeAction> key() const noexcept { return urd::key<::loki::formalism::DurativeAction>(index); }
};

}

#endif
