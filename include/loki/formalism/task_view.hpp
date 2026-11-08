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

#include "loki/formalism/task_data.hpp"

#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<typename V>
struct view_mixin<::loki::formalism::Task, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    const auto& get_name() const noexcept { return self().data().name; }
    auto get_domain() const noexcept { return self().template member<&urd::record_of<V>::domain>(); }
    auto get_requirements() const noexcept { return self().template member<&urd::record_of<V>::requirements>(); }
    auto get_types() const noexcept { return self().template member<&urd::record_of<V>::types>(); }
    auto get_predicates() const noexcept { return self().template member<&urd::record_of<V>::predicates>(); }
    auto get_objects() const noexcept { return self().template member<&urd::record_of<V>::objects>(); }
    auto get_initial_literals() const noexcept { return self().template member<&urd::record_of<V>::initial_literals>(); }
    auto get_initial_function_values() const noexcept { return self().template member<&urd::record_of<V>::initial_function_values>(); }
    auto get_goal() const noexcept { return self().template member<&urd::record_of<V>::goal>(); }
    auto get_metric() const noexcept { return self().template member<&urd::record_of<V>::metric>(); }
    auto get_axioms() const noexcept { return self().template member<&urd::record_of<V>::axioms>(); }
};
}  // namespace urd

#endif
