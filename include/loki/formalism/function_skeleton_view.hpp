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

#ifndef LOKI_FORMALISM_FUNCTION_SKELETON_VIEW_HPP_
#define LOKI_FORMALISM_FUNCTION_SKELETON_VIEW_HPP_

#include "loki/formalism/function_skeleton_data.hpp"

#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<typename V>
struct view_mixin<::loki::formalism::FunctionSkeleton, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    const auto& get_name() const noexcept { return self().data().name; }
    auto get_parameters() const noexcept { return self().template get<&urd::record_of<V>::parameters>(); }
    auto get_arity() const noexcept { return self().data().parameters.size(); }
    auto get_type() const noexcept { return self().template get<&urd::record_of<V>::type>(); }
};
}  // namespace urd

#endif
