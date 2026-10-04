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

#include <yggdrasil/containers/optional.hpp>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/detail/view.hpp>

namespace ygg
{

template<ygg::formalism::SymbolContextFor<::loki::formalism::Task> C>
class View<ygg::Index<::loki::formalism::Task>, C> : public ygg::formalism::detail::View<ygg::Index<::loki::formalism::Task>, C>
{
public:
    View(ygg::Index<::loki::formalism::Task> handle, const C& context) noexcept :
        ygg::formalism::detail::View<ygg::Index<::loki::formalism::Task>, C>(handle, context)
    {
    }

    const auto& get_name() const noexcept { return this->get_data().name; }
    auto get_domain() const noexcept { return ygg::make_view(this->get_data().domain, this->get_context()); }
    auto get_requirements() const noexcept { return ygg::make_view(this->get_data().requirements, this->get_context()); }
    auto get_objects() const noexcept { return ygg::make_view(this->get_data().objects, this->get_context()); }
    auto get_initial_literals() const noexcept { return ygg::make_view(this->get_data().initial_literals, this->get_context()); }
    auto get_initial_function_values() const noexcept { return ygg::make_view(this->get_data().initial_function_values, this->get_context()); }
    auto get_goal() const noexcept { return ygg::make_view(this->get_data().goal, this->get_context()); }
    auto get_metric() const noexcept { return ygg::make_view(this->get_data().metric, this->get_context()); }
    auto get_predicates() const noexcept { return ygg::make_view(this->get_data().predicates, this->get_context()); }
    auto get_axioms() const noexcept { return ygg::make_view(this->get_data().axioms, this->get_context()); }
};

}

#endif
