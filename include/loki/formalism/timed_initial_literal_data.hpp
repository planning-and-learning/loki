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

#ifndef LOKI_FORMALISM_TIMED_INITIAL_LITERAL_DATA_HPP_
#define LOKI_FORMALISM_TIMED_INITIAL_LITERAL_DATA_HPP_

#include "loki/formalism/declarations.hpp"

#include <urd/elements/flat.hpp>
#include <utility>

namespace urd
{

/// A literal that becomes true at a time point of a task: (at time literal).
template<>
struct data<::loki::formalism::TimedInitialLiteral>
{
    urd::key<::loki::formalism::TimedInitialLiteral> index;
    double time = 0.0;
    urd::index<::loki::formalism::Literal> literal;

    data() = default;

    data(double time_, urd::index<::loki::formalism::Literal> literal_) : index(), time(time_), literal(literal_) {}

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"time", &data::time>, urd::field<"literal", &data::literal>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&data::time, &data::literal> {}; }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::time, &data::literal> {}; }

    [[nodiscard]] urd::key<::loki::formalism::TimedInitialLiteral> key() const noexcept { return urd::key<::loki::formalism::TimedInitialLiteral>(index); }
};

}

#endif
