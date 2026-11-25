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

#ifndef LOKI_FORMALISM_CONSTRAINT_STATE_DATA_HPP_
#define LOKI_FORMALISM_CONSTRAINT_STATE_DATA_HPP_

#include "loki/formalism/conditions.hpp"
#include "loki/formalism/declarations.hpp"

#include <urd/elements/flat.hpp>
#include <utility>

namespace urd
{

/// A goal description as a formula over the state trajectory: it holds at the position it is read at.
template<>
struct data<::loki::formalism::Constraint<::loki::formalism::StateTag>>
{
    urd::key<::loki::formalism::Constraint<::loki::formalism::StateTag>> index;
    ::loki::formalism::ConditionVariant condition;

    data() = default;

    explicit data(::loki::formalism::ConditionVariant condition_) : index(), condition(std::move(condition_)) {}

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"condition", &data::condition>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&data::condition> {}; }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::condition> {}; }

    [[nodiscard]] urd::key<::loki::formalism::Constraint<::loki::formalism::StateTag>> key() const noexcept
    { return urd::key<::loki::formalism::Constraint<::loki::formalism::StateTag>>(index); }
};

}

#endif
