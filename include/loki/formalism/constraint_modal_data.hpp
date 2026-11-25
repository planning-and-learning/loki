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

#ifndef LOKI_FORMALISM_CONSTRAINT_MODAL_DATA_HPP_
#define LOKI_FORMALISM_CONSTRAINT_MODAL_DATA_HPP_

#include "loki/formalism/constraints.hpp"
#include "loki/formalism/declarations.hpp"

#include <urd/elements/flat.hpp>
#include <utility>

namespace urd
{

/// A modal formula over the state trajectory: its operator, its numbers (PDDL 3's time bounds, or a metric operator's interval),
/// and its operands in order.
template<>
struct data<::loki::formalism::Constraint<::loki::formalism::ModalTag>>
{
    urd::key<::loki::formalism::Constraint<::loki::formalism::ModalTag>> index;
    ::loki::formalism::ConstraintOperator operation {};
    urd::flat_vector<double> numbers;
    ::loki::formalism::ConstraintList constraints;

    data() = default;

    data(::loki::formalism::ConstraintOperator operation_, urd::flat_vector<double> numbers_, ::loki::formalism::ConstraintList constraints_) :
        index(),
        operation(std::move(operation_)),
        numbers(std::move(numbers_)),
        constraints(std::move(constraints_))
    {
    }

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"operation", &data::operation>, urd::field<"numbers", &data::numbers>, urd::field<"constraints", &data::constraints>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&data::operation, &data::numbers, &data::constraints> {}; }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::operation, &data::numbers, &data::constraints> {}; }

    [[nodiscard]] urd::key<::loki::formalism::Constraint<::loki::formalism::ModalTag>> key() const noexcept
    { return urd::key<::loki::formalism::Constraint<::loki::formalism::ModalTag>>(index); }
};

}

#endif
