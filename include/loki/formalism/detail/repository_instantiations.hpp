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

#ifndef LOKI_FORMALISM_DETAIL_REPOSITORY_INSTANTIATIONS_HPP_
#define LOKI_FORMALISM_DETAIL_REPOSITORY_INSTANTIATIONS_HPP_

/// Extern declarations of intern for every symbol type of the repository, instantiated in the library
/// (src/formalism/repository.cpp). Include only from sources that intern many of these types; see rule 9 of the
/// template rules (https://planning-and-learning.github.io/brokkr/template_rules.html).

#include "loki/formalism/declarations.hpp"

#include <yggdrasil/formalism/intern_instantiations.hpp>

/// The instantiations (PREFIX extern: declarations).
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage): expands to explicit template instantiations at namespace scope, which no function can produce
#define LOKI_FORMALISM_INTERNS(PREFIX) YGG_FORMALISM_INTERNS(PREFIX, 36, false, loki::formalism::SymbolRepositoryTypes)

LOKI_FORMALISM_INTERNS(extern)

#endif
