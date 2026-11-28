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

#ifndef LOKI_FORMALISM_PREFERENCES_HPP_
#define LOKI_FORMALISM_PREFERENCES_HPP_

#include "loki/formalism/conditions.hpp"
#include "loki/formalism/repository.hpp"
#include "loki/formalism/views.hpp"

#include <variant>

namespace loki::formalism
{

/// Whether a preference (PDDL 3) occurs in the condition.
///
/// A preference counts its violations: a rewrite that turns a condition holding one into a derived predicate, or that
/// duplicates it, changes what is-violated reads, where for a hard condition it would not.
inline bool contains_preference(ConditionView condition)
{
    return std::visit(
        [](const auto& node) -> bool
        {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::same_as<Node, EntityView<Condition<PreferenceTag>>>)
                return true;
            else if constexpr (std::same_as<Node, EntityView<Condition<AndTag>>> || std::same_as<Node, EntityView<Condition<OrTag>>>)
            {
                for (auto child : node.conditions())
                    if (contains_preference(child))
                        return true;
                return false;
            }
            else if constexpr (std::same_as<Node, EntityView<Condition<ImplyTag>>>)
                return contains_preference(node.left()) || contains_preference(node.right());
            else if constexpr (std::same_as<Node, EntityView<Condition<NotTag>>> || std::same_as<Node, EntityView<Condition<ExistsTag>>>
                               || std::same_as<Node, EntityView<Condition<ForallTag>>> || std::same_as<Node, EntityView<Condition<TimedTag>>>)
                return contains_preference(node.condition());
            else
                return false;
        },
        condition);
}

}  // namespace loki::formalism

#endif
