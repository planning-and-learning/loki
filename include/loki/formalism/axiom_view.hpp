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

#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<typename V>
struct view_mixin<::loki::formalism::Axiom, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_parameters() const noexcept { return self().template member<&urd::record_of<V>::parameters>(); }

    auto get_original_arity() const noexcept { return self().template member<&urd::record_of<V>::original_arity>(); }

    auto get_head() const noexcept { return self().template member<&urd::record_of<V>::head>(); }

    auto get_condition() const noexcept { return self().template member<&urd::record_of<V>::condition>(); }

    auto get_arity() const noexcept { return get_parameters().size(); }

    static constexpr auto functions() noexcept { return urd::Functions<urd::function<"get_arity", &view_mixin::get_arity>> {}; }
};
}  // namespace urd

#endif
