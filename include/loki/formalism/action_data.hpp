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

#ifndef LOKI_FORMALISM_ACTION_DATA_HPP_
#define LOKI_FORMALISM_ACTION_DATA_HPP_

#include "loki/formalism/declarations.hpp"

#include <cista/containers/optional.h>
#include <cista/containers/string.h>
#include <optional>
#include <string>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/core/config.hpp>

namespace urd
{

template<>
struct data<::loki::formalism::Action>
{
    urd::index<::loki::formalism::Action> index;
    urd::flat_string name;
    urd::flat_string original_name;
    urd::flat_vector<urd::index<::loki::formalism::Parameter>> parameters;
    ygg::uint_t original_arity = 0;
    urd::flat_optional<::loki::formalism::ConditionVariant> precondition;
    urd::flat_optional<::loki::formalism::EffectVariant> effect;

    data() = default;
    data(urd::flat_string name_,
         urd::flat_vector<urd::index<::loki::formalism::Parameter>> parameters_,
         urd::flat_optional<::loki::formalism::ConditionVariant> precondition_,
         urd::flat_optional<::loki::formalism::EffectVariant> effect_) :
        index(),
        name(std::move(name_)),
        original_name(name),
        parameters(std::move(parameters_)),
        original_arity(parameters.size()),
        precondition(std::move(precondition_)),
        effect(std::move(effect_))
    {
    }
    data(urd::flat_string name_,
         urd::flat_vector<urd::index<::loki::formalism::Parameter>> parameters_,
         ygg::uint_t original_arity_,
         urd::flat_optional<::loki::formalism::ConditionVariant> precondition_,
         urd::flat_optional<::loki::formalism::EffectVariant> effect_) :
        index(),
        name(std::move(name_)),
        original_name(name),
        parameters(std::move(parameters_)),
        original_arity(original_arity_),
        precondition(std::move(precondition_)),
        effect(std::move(effect_))
    {
    }
    data(urd::flat_string name_,
         urd::flat_string original_name_,
         urd::flat_vector<urd::index<::loki::formalism::Parameter>> parameters_,
         ygg::uint_t original_arity_,
         urd::flat_optional<::loki::formalism::ConditionVariant> precondition_,
         urd::flat_optional<::loki::formalism::EffectVariant> effect_) :
        index(),
        name(std::move(name_)),
        original_name(std::move(original_name_)),
        parameters(std::move(parameters_)),
        original_arity(original_arity_),
        precondition(std::move(precondition_)),
        effect(std::move(effect_))
    {
    }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::index, &data::name, &data::original_name, &data::parameters, &data::original_arity, &data::precondition, &data::effect> {}; }
    static constexpr auto identifying_members() noexcept
    { return urd::Members<&data::original_name, &data::parameters, &data::original_arity, &data::precondition, &data::effect> {}; }
};

}

#endif
