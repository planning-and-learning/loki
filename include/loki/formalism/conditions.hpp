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

#ifndef LOKI_FORMALISM_CONDITIONS_HPP_
#define LOKI_FORMALISM_CONDITIONS_HPP_

#include "loki/formalism/declarations.hpp"

#include <boost/mp11.hpp>
#include <urd/elements/flat.hpp>
#include <urd/elements/view.hpp>
#include <variant>

// Conditions: one of the family's records, stored as it is, and read through the repository.

namespace loki::formalism
{

/// A condition: one of the family's records. Stored as it is, not interned.
using ConditionVariant = urd::index_variant<boost::mp11::mp_transform<Condition, ConditionTags>>;
using ConditionList = urd::flat_vector<ConditionVariant>;
/// A condition read through the repository: the view of the record it holds.
using ConditionView = boost::mp11::mp_apply<std::variant, boost::mp11::mp_transform<EntityView, boost::mp11::mp_transform<Condition, ConditionTags>>>;
using ConditionListView = urd::resolved_range<ConditionList, Repository>;

}  // namespace loki::formalism

#endif
