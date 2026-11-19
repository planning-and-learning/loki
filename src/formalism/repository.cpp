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

#include "loki/formalism/repository.hpp"

#include "loki/formalism/detail/repository_instantiations.hpp"

#include <yggdrasil/formalism/builder_instantiations.hpp>
#include <yggdrasil/formalism/repository_instantiations.hpp>

LOKI_FORMALISM_INTERNS()

// The repository's members, which the library's other sources only declare (yggdrasil/formalism/repository.hpp).
YGG_FORMALISM_REPOSITORY(, loki::formalism::Repository, 36, 0)
YGG_FORMALISM_BUILDER(, loki::formalism::Builder, 36)
