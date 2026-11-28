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

#ifndef LOKI_FORMALISM_AXIOM_VIEW_HPP_
#define LOKI_FORMALISM_AXIOM_VIEW_HPP_

#include "loki/formalism/axiom_data.hpp"
#include "loki/formalism/declarations.hpp"

#include <fmt/base.h>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>
#include <yggdrasil/formatting/without_format_spec.hpp>

namespace urd
{
template<typename V>
struct view_mixin<::loki::formalism::Axiom, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto parameters() const noexcept { return self().template member<&urd::record_of<V>::parameters>(); }

    auto original_arity() const noexcept { return self().template member<&urd::record_of<V>::original_arity>(); }

    auto head() const noexcept { return self().template member<&urd::record_of<V>::head>(); }

    auto condition() const noexcept { return self().template member<&urd::record_of<V>::condition>(); }

    auto arity() const noexcept { return parameters().size(); }

    static constexpr auto functions() noexcept { return urd::Functions<urd::function<"arity", &view_mixin::arity>> {}; }
};
}  // namespace urd

/// Written in PDDL (src/formalism/formatter.cpp).
template<>
struct fmt::formatter<loki::formalism::AxiomView, char> : ygg::without_format_spec
{
    fmt::format_context::iterator format(const loki::formalism::AxiomView& value, fmt::format_context& ctx) const;
};

#endif
