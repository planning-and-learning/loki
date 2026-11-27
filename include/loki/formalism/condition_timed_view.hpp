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

#ifndef LOKI_FORMALISM_CONDITION_TIMED_VIEW_HPP_
#define LOKI_FORMALISM_CONDITION_TIMED_VIEW_HPP_

#include "loki/formalism/condition_timed_data.hpp"
#include "loki/formalism/declarations.hpp"

#include <fmt/base.h>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>
#include <yggdrasil/formatting/without_format_spec.hpp>

namespace urd
{
template<typename V>
struct view_mixin<::loki::formalism::Condition<::loki::formalism::TimedTag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_time() const noexcept { return self().template member<&urd::record_of<V>::time>(); }

    auto get_condition() const noexcept { return self().template member<&urd::record_of<V>::condition>(); }
};
}  // namespace urd

/// Written in PDDL (src/formalism/formatter.cpp).
template<>
struct fmt::formatter<loki::formalism::EntityView<loki::formalism::Condition<loki::formalism::TimedTag>>, char> : ygg::without_format_spec
{
    fmt::format_context::iterator format(const loki::formalism::EntityView<loki::formalism::Condition<loki::formalism::TimedTag>>& value, fmt::format_context& ctx) const;
};

#endif
