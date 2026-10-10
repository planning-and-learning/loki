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
#include "loki/formalism/formatter.hpp"
#include "loki/formalism/views.hpp"

#include <yggdrasil/formalism/relation_repository.hpp>
#include <yggdrasil/formalism/repository.hpp>
#include <yggdrasil/formalism/symbol_repository.hpp>
#include <yggdrasil/semantics/canonicalization.hpp>

namespace loki::formalism
{

template<typename T>
inline void canonicalize(Repository&, ygg::Data<T>&) noexcept
{
}

inline void canonicalize(Repository& repository, ygg::Data<Type>& data) { ygg::canonicalize(repository, data.bases); }
inline void canonicalize(Repository& repository, ygg::Data<Object>& data) { ygg::canonicalize(repository, data.types); }
inline void canonicalize(Repository& repository, ygg::Data<Parameter>& data) { ygg::canonicalize(repository, data.types); }
inline void canonicalize(Repository& repository, ygg::Data<FunctionExpression<MultiTag>>& data) { ygg::canonicalize<false>(repository, data.args); }
inline void canonicalize(Repository& repository, ygg::Data<Condition<AndTag>>& data) { ygg::canonicalize(repository, data.conditions); }
inline void canonicalize(Repository& repository, ygg::Data<Condition<OrTag>>& data) { ygg::canonicalize(repository, data.conditions); }
inline void canonicalize(Repository& repository, ygg::Data<Condition<ExistsTag>>& data) { ygg::canonicalize(repository, data.parameters); }
inline void canonicalize(Repository& repository, ygg::Data<Condition<ForallTag>>& data) { ygg::canonicalize(repository, data.parameters); }
inline void canonicalize(Repository& repository, ygg::Data<Effect<AndTag>>& data) { ygg::canonicalize(repository, data.effects); }
inline void canonicalize(Repository& repository, ygg::Data<Effect<OneOfTag>>& data) { ygg::canonicalize(repository, data.effects); }
inline void canonicalize(Repository& repository, ygg::Data<Effect<ProbabilisticTag>>& data) { ygg::canonicalize(repository, data.alternatives); }

inline void canonicalize(Repository& repository, ygg::Data<Domain>& data)
{
    ygg::canonicalize(repository, data.requirements);
    ygg::canonicalize(repository, data.types);
    ygg::canonicalize(repository, data.constants);
    ygg::canonicalize(repository, data.predicates);
    ygg::canonicalize(repository, data.functions);
    ygg::canonicalize(repository, data.actions);
    ygg::canonicalize(repository, data.axioms);
}

inline void canonicalize(Repository& repository, ygg::Data<Task>& data)
{
    ygg::canonicalize(repository, data.requirements);
    ygg::canonicalize(repository, data.objects);
    ygg::canonicalize(repository, data.initial_literals);
    ygg::canonicalize(repository, data.initial_function_values);
    ygg::canonicalize(repository, data.predicates);
    ygg::canonicalize(repository, data.axioms);
}

// Context-aware is_canonical: the safety-check counterpart of canonicalize(Repository&, Data<T>&).
// Types without sortable list members are canonical by construction (generic overload). The remaining
// types verify each list member with detail::is_canonical_list instead of blindly returning true.
template<typename T>
inline bool is_canonical(const Repository&, const ygg::Data<T>&) noexcept
{
    return true;
}

inline bool is_canonical(const Repository& repository, const ygg::Data<Type>& data) { return ygg::is_canonical(repository, data.bases); }
inline bool is_canonical(const Repository& repository, const ygg::Data<Object>& data) { return ygg::is_canonical(repository, data.types); }
inline bool is_canonical(const Repository& repository, const ygg::Data<Parameter>& data) { return ygg::is_canonical(repository, data.types); }
inline bool is_canonical(const Repository& repository, const ygg::Data<FunctionExpression<MultiTag>>& data)
{
    return ygg::is_canonical<false>(repository, data.args);
}
inline bool is_canonical(const Repository& repository, const ygg::Data<Condition<AndTag>>& data) { return ygg::is_canonical(repository, data.conditions); }
inline bool is_canonical(const Repository& repository, const ygg::Data<Condition<OrTag>>& data) { return ygg::is_canonical(repository, data.conditions); }
inline bool is_canonical(const Repository& repository, const ygg::Data<Condition<ExistsTag>>& data) { return ygg::is_canonical(repository, data.parameters); }
inline bool is_canonical(const Repository& repository, const ygg::Data<Condition<ForallTag>>& data) { return ygg::is_canonical(repository, data.parameters); }
inline bool is_canonical(const Repository& repository, const ygg::Data<Effect<AndTag>>& data) { return ygg::is_canonical(repository, data.effects); }
inline bool is_canonical(const Repository& repository, const ygg::Data<Effect<OneOfTag>>& data) { return ygg::is_canonical(repository, data.effects); }
inline bool is_canonical(const Repository& repository, const ygg::Data<Effect<ProbabilisticTag>>& data) { return ygg::is_canonical(repository, data.alternatives); }

inline bool is_canonical(const Repository& repository, const ygg::Data<Domain>& data)
{
    return ygg::is_canonical(repository, data.requirements) && ygg::is_canonical(repository, data.types) && ygg::is_canonical(repository, data.constants)
           && ygg::is_canonical(repository, data.predicates) && ygg::is_canonical(repository, data.functions) && ygg::is_canonical(repository, data.actions)
           && ygg::is_canonical(repository, data.axioms);
}

inline bool is_canonical(const Repository& repository, const ygg::Data<Task>& data)
{
    return ygg::is_canonical(repository, data.requirements) && ygg::is_canonical(repository, data.objects)
           && ygg::is_canonical(repository, data.initial_literals) && ygg::is_canonical(repository, data.initial_function_values)
           && ygg::is_canonical(repository, data.predicates) && ygg::is_canonical(repository, data.axioms);
}

}  // namespace loki::formalism

#endif
