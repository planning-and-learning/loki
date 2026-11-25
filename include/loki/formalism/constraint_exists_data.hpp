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

#ifndef LOKI_FORMALISM_CONSTRAINT_EXISTS_DATA_HPP_
#define LOKI_FORMALISM_CONSTRAINT_EXISTS_DATA_HPP_

#include "loki/formalism/constraints.hpp"
#include "loki/formalism/declarations.hpp"

#include <urd/elements/flat.hpp>
#include <utility>

namespace urd
{

/// A formula over the state trajectory for some binding of its parameters (:trajectory-formulas).
template<>
struct data<::loki::formalism::Constraint<::loki::formalism::ExistsTag>>
{
    urd::key<::loki::formalism::Constraint<::loki::formalism::ExistsTag>> index;
    urd::flat_vector<urd::index<::loki::formalism::Parameter>> parameters;
    ::loki::formalism::ConstraintVariant constraint;

    data() = default;

    data(urd::flat_vector<urd::index<::loki::formalism::Parameter>> parameters_, ::loki::formalism::ConstraintVariant constraint_) :
        index(),
        parameters(std::move(parameters_)),
        constraint(std::move(constraint_))
    {
    }

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"parameters", &data::parameters>, urd::field<"constraint", &data::constraint>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&data::parameters, &data::constraint> {}; }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::parameters, &data::constraint> {}; }

    [[nodiscard]] urd::key<::loki::formalism::Constraint<::loki::formalism::ExistsTag>> key() const noexcept
    { return urd::key<::loki::formalism::Constraint<::loki::formalism::ExistsTag>>(index); }
};

}

#endif
