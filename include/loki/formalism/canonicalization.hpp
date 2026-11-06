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
#include <yggdrasil/formatting/cista_formatters.hpp>
#include <yggdrasil/semantics/canonicalization.hpp>

#include <algorithm>
#include <fmt/format.h>
#include <string>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>

namespace loki::formalism
{

namespace detail
{
/// The text an element reads as through the context: the order canonical lists are sorted by.
template<typename C, typename Element>
    requires urd::resolving<Element> || urd::variant_value<Element>
std::string fmt_key(const C& context, const Element& element)
{ return fmt::format("{}", urd::resolve(element, context)); }

/// A list of elements the context reads (indices, records, compositions of them): sorted by how they read.
template<bool Deduplicate = true, typename C, typename List>
bool is_canonical(const C& context, const List& list)
{
    if (list.size() < 2)
        return true;

    auto previous_key = fmt_key(context, list[0]);
    for (std::size_t i = 1; i < list.size(); ++i)
    {
        auto current_key = fmt_key(context, list[i]);
        if constexpr (Deduplicate)
        {
            if (!(previous_key < current_key))
                return false;
        }
        else if (current_key < previous_key || (current_key == previous_key && list[i] < list[i - 1]))
            return false;
        previous_key = std::move(current_key);
    }
    return true;
}

template<bool Deduplicate = true, typename C, typename List>
void canonicalize(const C& context, List& list)
{
    using Element = typename List::value_type;

    if (list.size() < 2)
        return;

    auto keyed = urd::Store<urd::sequence<std::pair<std::string, Element>>> {};
    keyed.reserve(list.size());
    for (auto& element : list)
        keyed.emplace_back(fmt_key(context, element), std::move(element));

    if (!std::is_sorted(keyed.begin(), keyed.end()))
        std::sort(keyed.begin(), keyed.end());

    auto size = std::size_t { 0 };
    for (std::size_t i = 0; i < keyed.size(); ++i)
    {
        if constexpr (Deduplicate)
            if (i > 0 && keyed[i - 1].first == keyed[i].first)
                continue;
        list[size++] = std::move(keyed[i].second);
    }
    list.resize(size);
}

}  // namespace detail


template<typename T>
inline void canonicalize(Repository&, urd::data<T>&) noexcept
{
}

inline void canonicalize(Repository& repository, urd::data<Type>& data) { detail::canonicalize(repository, data.bases); }
inline void canonicalize(Repository& repository, urd::data<Object>& data) { detail::canonicalize(repository, data.types); }
inline void canonicalize(Repository& repository, urd::data<Parameter>& data) { detail::canonicalize(repository, data.types); }
inline void canonicalize(Repository& repository, urd::data<FunctionExpression<MultiTag>>& data) { detail::canonicalize<false>(repository, data.args); }
inline void canonicalize(Repository& repository, urd::data<Condition<AndTag>>& data) { detail::canonicalize(repository, data.conditions); }
inline void canonicalize(Repository& repository, urd::data<Condition<OrTag>>& data) { detail::canonicalize(repository, data.conditions); }
inline void canonicalize(Repository& repository, urd::data<Condition<ExistsTag>>& data) { detail::canonicalize(repository, data.parameters); }
inline void canonicalize(Repository& repository, urd::data<Condition<ForallTag>>& data) { detail::canonicalize(repository, data.parameters); }
inline void canonicalize(Repository& repository, urd::data<Effect<AndTag>>& data) { detail::canonicalize(repository, data.effects); }
inline void canonicalize(Repository& repository, urd::data<Effect<OneOfTag>>& data) { detail::canonicalize(repository, data.effects); }
inline void canonicalize(Repository& repository, urd::data<Effect<ProbabilisticTag>>& data) { detail::canonicalize(repository, data.alternatives); }

inline void canonicalize(Repository& repository, urd::data<Domain>& data)
{
    detail::canonicalize(repository, data.requirements);
    detail::canonicalize(repository, data.types);
    detail::canonicalize(repository, data.constants);
    detail::canonicalize(repository, data.predicates);
    detail::canonicalize(repository, data.functions);
    detail::canonicalize(repository, data.actions);
    detail::canonicalize(repository, data.axioms);
}

inline void canonicalize(Repository& repository, urd::data<Task>& data)
{
    detail::canonicalize(repository, data.requirements);
    detail::canonicalize(repository, data.objects);
    detail::canonicalize(repository, data.initial_literals);
    detail::canonicalize(repository, data.initial_function_values);
    detail::canonicalize(repository, data.predicates);
    detail::canonicalize(repository, data.axioms);
}

// Context-aware is_canonical: the safety-check counterpart of canonicalize(Repository&, urd::data<T>&).
// Types without sortable list members are canonical by construction (generic overload). The remaining
// types verify each list member with detail::is_canonical instead of blindly returning true.
template<typename T>
inline bool is_canonical(const Repository&, const urd::data<T>&) noexcept
{ return true; }

inline bool is_canonical(const Repository& repository, const urd::data<Type>& data) { return detail::is_canonical(repository, data.bases); }
inline bool is_canonical(const Repository& repository, const urd::data<Object>& data) { return detail::is_canonical(repository, data.types); }
inline bool is_canonical(const Repository& repository, const urd::data<Parameter>& data) { return detail::is_canonical(repository, data.types); }
inline bool is_canonical(const Repository& repository, const urd::data<FunctionExpression<MultiTag>>& data)
{ return detail::is_canonical<false>(repository, data.args); }
inline bool is_canonical(const Repository& repository, const urd::data<Condition<AndTag>>& data) { return detail::is_canonical(repository, data.conditions); }
inline bool is_canonical(const Repository& repository, const urd::data<Condition<OrTag>>& data) { return detail::is_canonical(repository, data.conditions); }
inline bool is_canonical(const Repository& repository, const urd::data<Condition<ExistsTag>>& data) { return detail::is_canonical(repository, data.parameters); }
inline bool is_canonical(const Repository& repository, const urd::data<Condition<ForallTag>>& data) { return detail::is_canonical(repository, data.parameters); }
inline bool is_canonical(const Repository& repository, const urd::data<Effect<AndTag>>& data) { return detail::is_canonical(repository, data.effects); }
inline bool is_canonical(const Repository& repository, const urd::data<Effect<OneOfTag>>& data) { return detail::is_canonical(repository, data.effects); }
inline bool is_canonical(const Repository& repository, const urd::data<Effect<ProbabilisticTag>>& data)
{ return detail::is_canonical(repository, data.alternatives); }

inline bool is_canonical(const Repository& repository, const urd::data<Domain>& data)
{
    return detail::is_canonical(repository, data.requirements) && detail::is_canonical(repository, data.types) && detail::is_canonical(repository, data.constants)
           && detail::is_canonical(repository, data.predicates) && detail::is_canonical(repository, data.functions) && detail::is_canonical(repository, data.actions)
           && detail::is_canonical(repository, data.axioms);
}

inline bool is_canonical(const Repository& repository, const urd::data<Task>& data)
{
    return detail::is_canonical(repository, data.requirements) && detail::is_canonical(repository, data.objects)
           && detail::is_canonical(repository, data.initial_literals) && detail::is_canonical(repository, data.initial_function_values)
           && detail::is_canonical(repository, data.predicates) && detail::is_canonical(repository, data.axioms);
}

}  // namespace loki::formalism

#endif
