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

#ifndef LOKI_FORMALISM_FUNCTION_EXPRESSION_IS_VIOLATED_DATA_HPP_
#define LOKI_FORMALISM_FUNCTION_EXPRESSION_IS_VIOLATED_DATA_HPP_

#include "loki/formalism/declarations.hpp"

#include <urd/elements/flat.hpp>
#include <utility>

namespace urd
{

/// The number of violations of the preferences of the name: (is-violated name).
template<>
struct data<::loki::formalism::FunctionExpression<::loki::formalism::IsViolatedTag>>
{
    urd::key<::loki::formalism::FunctionExpression<::loki::formalism::IsViolatedTag>> index;
    urd::flat_string name;

    data() = default;

    explicit data(urd::flat_string name_) : index(), name(std::move(name_)) {}

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) noexcept = default;
    data& operator=(data&& other) noexcept = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"name", &data::name>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&data::name> {}; }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::name> {}; }

    [[nodiscard]] urd::key<::loki::formalism::FunctionExpression<::loki::formalism::IsViolatedTag>> key() const noexcept
    { return urd::key<::loki::formalism::FunctionExpression<::loki::formalism::IsViolatedTag>>(index); }
};

}

#endif
