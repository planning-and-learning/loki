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

#ifndef LOKI_FORMALISM_CHOICES_HPP_
#define LOKI_FORMALISM_CHOICES_HPP_

#include "loki/formalism/effects.hpp"
#include "loki/formalism/repository.hpp"
#include "loki/formalism/views.hpp"

#include <variant>

namespace loki::formalism
{

/// Whether a oneof or probabilistic effect occurs in the effect.
///
/// A choice is drawn once per application of the effect that holds it, independently of every other choice
/// (PPDDL 1.0, NPDDL): two equal choices are two draws, and a rewrite that duplicates or merges an effect holding
/// a choice changes the semantics, where for a deterministic effect it would not.
inline bool contains_choice(EffectView effect)
{
    return std::visit(
        [](const auto& node) -> bool
        {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::same_as<Node, EntityView<Effect<OneOfTag>>> || std::same_as<Node, EntityView<Effect<ProbabilisticTag>>>)
                return true;
            else if constexpr (std::same_as<Node, EntityView<Effect<AndTag>>>)
            {
                for (auto child : node.effects())
                    if (contains_choice(child))
                        return true;
                return false;
            }
            else if constexpr (std::same_as<Node, EntityView<Effect<ForallTag>>> || std::same_as<Node, EntityView<Effect<WhenTag>>>
                               || std::same_as<Node, EntityView<Effect<TimedTag>>>)
                return contains_choice(node.effect());
            else
                return false;
        },
        effect);
}

}  // namespace loki::formalism

#endif
