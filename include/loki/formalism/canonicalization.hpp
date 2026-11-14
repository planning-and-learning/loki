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

#ifndef LOKI_FORMALISM_CANONICALIZATION_HPP_
#define LOKI_FORMALISM_CANONICALIZATION_HPP_

#include "loki/formalism/datas.hpp"
#include "loki/formalism/declarations.hpp"

namespace loki::formalism
{

/// Brings a record into canonical form before it is interned: its lists sorted by how their elements read, and
/// deduplicated where order and repetition carry no meaning. Records without such lists are left as they are.
template<typename T>
inline void canonicalize(Repository&, urd::data<T>&) noexcept
{
}

// Defined in src/formalism/canonicalization.cpp, which alone formats the elements to sort them.
void canonicalize(Repository& repository, urd::data<Type>& data);
void canonicalize(Repository& repository, urd::data<Object>& data);
void canonicalize(Repository& repository, urd::data<Parameter>& data);
void canonicalize(Repository& repository, urd::data<FunctionExpression<MultiTag>>& data);
void canonicalize(Repository& repository, urd::data<Condition<AndTag>>& data);
void canonicalize(Repository& repository, urd::data<Condition<OrTag>>& data);
void canonicalize(Repository& repository, urd::data<Condition<ExistsTag>>& data);
void canonicalize(Repository& repository, urd::data<Condition<ForallTag>>& data);
void canonicalize(Repository& repository, urd::data<Effect<AndTag>>& data);
void canonicalize(Repository& repository, urd::data<Effect<OneOfTag>>& data);
void canonicalize(Repository& repository, urd::data<Effect<ProbabilisticTag>>& data);
void canonicalize(Repository& repository, urd::data<Domain>& data);
void canonicalize(Repository& repository, urd::data<Task>& data);

}  // namespace loki::formalism

#endif
