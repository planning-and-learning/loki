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
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<::loki::formalism::Action>
{
    ygg::Index<::loki::formalism::Action> index;
    ::cista::offset::string name;
    ::cista::offset::string original_name;
    ygg::IndexList<::loki::formalism::Parameter> parameters;
    ygg::uint_t original_arity;
    ::cista::optional<ygg::Index<::loki::formalism::Condition<>>> precondition;
    ::cista::optional<ygg::Index<::loki::formalism::Effect<>>> effect;

    Data() = default;
    Data(::cista::offset::string name_,
         ygg::IndexList<::loki::formalism::Parameter> parameters_,
         ::cista::optional<ygg::Index<::loki::formalism::Condition<>>> precondition_,
         ::cista::optional<ygg::Index<::loki::formalism::Effect<>>> effect_) :
        index(),
        name(std::move(name_)),
        original_name(name),
        parameters(std::move(parameters_)),
        original_arity(parameters.size()),
        precondition(precondition_),
        effect(effect_)
    {
    }
    Data(::cista::offset::string name_,
         ygg::IndexList<::loki::formalism::Parameter> parameters_,
         ygg::uint_t original_arity_,
         ::cista::optional<ygg::Index<::loki::formalism::Condition<>>> precondition_,
         ::cista::optional<ygg::Index<::loki::formalism::Effect<>>> effect_) :
        index(),
        name(std::move(name_)),
        original_name(name),
        parameters(std::move(parameters_)),
        original_arity(original_arity_),
        precondition(precondition_),
        effect(effect_)
    {
    }
    Data(::cista::offset::string name_,
         ::cista::offset::string original_name_,
         ygg::IndexList<::loki::formalism::Parameter> parameters_,
         ygg::uint_t original_arity_,
         ::cista::optional<ygg::Index<::loki::formalism::Condition<>>> precondition_,
         ::cista::optional<ygg::Index<::loki::formalism::Effect<>>> effect_) :
        index(),
        name(std::move(name_)),
        original_name(std::move(original_name_)),
        parameters(std::move(parameters_)),
        original_arity(original_arity_),
        precondition(precondition_),
        effect(effect_)
    {
    }
    template<typename C>
    Data(const std::string& name_,
         const std::vector<::ygg::View<ygg::Index<::loki::formalism::Parameter>, C>>& parameters_,
         const std::optional<::ygg::View<ygg::Index<::loki::formalism::Condition<>>, C>>& precondition_,
         const std::optional<::ygg::View<ygg::Index<::loki::formalism::Effect<>>, C>>& effect_) :
        Data(name_, name_, parameters_, parameters_.size(), precondition_, effect_)
    {
    }
    template<typename C>
    Data(const std::string& name_,
         const std::string& original_name_,
         const std::vector<::ygg::View<ygg::Index<::loki::formalism::Parameter>, C>>& parameters_,
         ygg::uint_t original_arity_,
         const std::optional<::ygg::View<ygg::Index<::loki::formalism::Condition<>>, C>>& precondition_,
         const std::optional<::ygg::View<ygg::Index<::loki::formalism::Effect<>>, C>>& effect_) :
        index(),
        name(name_),
        original_name(original_name_),
        parameters(),
        original_arity(original_arity_),
        precondition(),
        effect()
    {
        set(parameters_, parameters);
        set(precondition_, precondition);
        set(effect_, effect);
    }

    auto cista_members() noexcept { return std::tie(index, name, original_name, parameters, original_arity, precondition, effect); }
    auto cista_members() const noexcept { return std::tie(index, name, original_name, parameters, original_arity, precondition, effect); }
    auto identifying_members() const noexcept { return std::tie(original_name, parameters, original_arity, precondition, effect); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}

#endif
