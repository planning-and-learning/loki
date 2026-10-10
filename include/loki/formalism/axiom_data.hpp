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

#ifndef LOKI_FORMALISM_AXIOM_DATA_HPP_
#define LOKI_FORMALISM_AXIOM_DATA_HPP_

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
struct Data<::loki::formalism::Axiom>
{
    ygg::Index<::loki::formalism::Axiom> index;
    ygg::IndexList<::loki::formalism::Parameter> parameters;
    ygg::uint_t original_arity;
    ygg::Index<::loki::formalism::Literal> head;
    ygg::Index<::loki::formalism::Condition<>> condition;

    Data() = default;
    Data(ygg::IndexList<::loki::formalism::Parameter> parameters_,
         ygg::Index<::loki::formalism::Literal> head_,
         ygg::Index<::loki::formalism::Condition<>> condition_) :
        index(),
        parameters(std::move(parameters_)),
        original_arity(parameters.size()),
        head(head_),
        condition(condition_)
    {
    }
    Data(ygg::IndexList<::loki::formalism::Parameter> parameters_,
         ygg::uint_t original_arity_,
         ygg::Index<::loki::formalism::Literal> head_,
         ygg::Index<::loki::formalism::Condition<>> condition_) :
        index(),
        parameters(std::move(parameters_)),
        original_arity(original_arity_),
        head(head_),
        condition(condition_)
    {
    }
    template<typename C>
    Data(const std::vector<::ygg::View<ygg::Index<::loki::formalism::Parameter>, C>>& parameters_,
         ::ygg::View<ygg::Index<::loki::formalism::Literal>, C> head_,
         ::ygg::View<ygg::Index<::loki::formalism::Condition<>>, C> condition_) :
        index(),
        parameters(),
        original_arity(parameters_.size()),
        head(),
        condition()
    {
        set(parameters_, parameters);
        set(head_, head);
        set(condition_, condition);
    }
    template<typename C>
    Data(const std::vector<::ygg::View<ygg::Index<::loki::formalism::Parameter>, C>>& parameters_,
         ygg::uint_t original_arity_,
         ::ygg::View<ygg::Index<::loki::formalism::Literal>, C> head_,
         ::ygg::View<ygg::Index<::loki::formalism::Condition<>>, C> condition_) :
        index(),
        parameters(),
        original_arity(original_arity_),
        head(),
        condition()
    {
        set(parameters_, parameters);
        set(head_, head);
        set(condition_, condition);
    }

    auto cista_members() noexcept { return std::tie(index, parameters, original_arity, head, condition); }
    auto cista_members() const noexcept { return std::tie(index, parameters, original_arity, head, condition); }
    auto identifying_members() const noexcept { return std::tie(parameters, original_arity, head, condition); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}

#endif
