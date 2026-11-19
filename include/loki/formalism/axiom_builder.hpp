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

#ifndef LOKI_FORMALISM_AXIOM_BUILDER_HPP_
#define LOKI_FORMALISM_AXIOM_BUILDER_HPP_

#include "loki/formalism/axiom_data.hpp"

#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>

namespace urd
{

/// The builder of Axiom: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<::loki::formalism::Axiom, C>
{
    urd::index<::loki::formalism::Axiom> index;
    urd::built_t<urd::flat_vector<urd::index<::loki::formalism::Parameter>>, C> parameters;
    urd::built_t<ygg::uint_t, C> original_arity = 0;
    urd::built_t<urd::index<::loki::formalism::Literal>, C> head;
    urd::built_t<::loki::formalism::ConditionVariant, C> condition;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"parameters", &builder::parameters>,
                           urd::field<"original_arity", &builder::original_arity>,
                           urd::field<"head", &builder::head>,
                           urd::field<"condition", &builder::condition>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::parameters, &builder::original_arity, &builder::head, &builder::condition> {}; }
};

}

#endif
