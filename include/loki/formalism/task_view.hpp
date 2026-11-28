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

#ifndef LOKI_FORMALISM_TASK_VIEW_HPP_
#define LOKI_FORMALISM_TASK_VIEW_HPP_

#include "loki/formalism/declarations.hpp"
#include "loki/formalism/task_data.hpp"

#include <fmt/base.h>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>
#include <yggdrasil/formatting/without_format_spec.hpp>

namespace urd
{
template<typename V>
struct view_mixin<::loki::formalism::Task, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    const auto& name() const noexcept { return self().template member<&urd::record_of<V>::name>(); }

    auto domain() const noexcept { return self().template member<&urd::record_of<V>::domain>(); }

    auto requirements() const noexcept { return self().template member<&urd::record_of<V>::requirements>(); }

    auto types() const noexcept { return self().template member<&urd::record_of<V>::types>(); }

    auto predicates() const noexcept { return self().template member<&urd::record_of<V>::predicates>(); }

    auto objects() const noexcept { return self().template member<&urd::record_of<V>::objects>(); }

    auto initial_literals() const noexcept { return self().template member<&urd::record_of<V>::initial_literals>(); }

    auto initial_function_values() const noexcept { return self().template member<&urd::record_of<V>::initial_function_values>(); }

    auto timed_initial_literals() const noexcept { return self().template member<&urd::record_of<V>::timed_initial_literals>(); }

    auto initial_effects() const noexcept { return self().template member<&urd::record_of<V>::initial_effects>(); }

    auto goal() const noexcept { return self().template member<&urd::record_of<V>::goal>(); }

    auto goal_reward() const noexcept { return self().template member<&urd::record_of<V>::goal_reward>(); }

    auto metric() const noexcept { return self().template member<&urd::record_of<V>::metric>(); }

    auto axioms() const noexcept { return self().template member<&urd::record_of<V>::axioms>(); }

    auto constraints() const noexcept { return self().template member<&urd::record_of<V>::constraints>(); }
};
}  // namespace urd

/// Written in PDDL (src/formalism/formatter.cpp).
template<>
struct fmt::formatter<loki::formalism::TaskView, char> : ygg::without_format_spec
{
    fmt::format_context::iterator format(const loki::formalism::TaskView& value, fmt::format_context& ctx) const;
};

#endif
