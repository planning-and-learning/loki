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

#ifndef LOKI_FORMALISM_TERM_HPP_
#define LOKI_FORMALISM_TERM_HPP_

#include "loki/formalism/declarations.hpp"

#include <boost/mp11.hpp>
#include <urd/elements/flat.hpp>
#include <urd/elements/view.hpp>
#include <variant>

// Terms: a variable or an object, stored as it is, and read through the repository.

namespace loki::formalism
{

/// A term: a variable or an object. Stored as it is, not interned.
using Term = urd::flat_variant<urd::index<Variable>, urd::index<Object>>;
using TermList = urd::flat_vector<Term>;
/// A term read through the repository: its variable's or its object's view.
using TermView = std::variant<VariableView, ObjectView>;
using TermListView = urd::resolved_range<TermList, Repository>;

}  // namespace loki::formalism

#endif
