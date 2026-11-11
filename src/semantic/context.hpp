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

#ifndef LOKI_SEMANTIC_CONTEXT_HPP_
#define LOKI_SEMANTIC_CONTEXT_HPP_

#include "loki/formalism/domain_view.hpp"
#include "loki/formalism/function_skeleton_view.hpp"
#include "loki/formalism/object_view.hpp"
#include "loki/formalism/predicate_view.hpp"
#include "loki/formalism/repository.hpp"
#include "loki/formalism/type_view.hpp"
#include "loki/formalism/variable_view.hpp"

#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <urd/config.hpp>
#include <urd/elements/equal_to.hpp>
#include <urd/elements/hash.hpp>
#include <urd/repr/maps/std_map.hpp>
#include <urd/repr/maps/unordered_flat_map.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/repr/values/std_set.hpp>
#include <urd/repr/values/unordered_flat_set.hpp>
#include <urd/repr/wrappers/scoped.hpp>
#include <urd/store.hpp>
#include <vector>

namespace loki::semantic
{

namespace detail
{
struct TranslationStorage;
}

// Symbols and storage of the currently parsed domain; survives across parse_task calls.
struct DomainContext
{
    // Interns the "object" and "number" base types into the storage's repository.
    explicit DomainContext(std::shared_ptr<detail::TranslationStorage> storage);

    std::shared_ptr<detail::TranslationStorage> storage;
    std::optional<formalism::DomainView> domain;
    std::string domain_name;

    /// The domain built from the domain file; a problem is built only after it (asserted in Debug).
    [[nodiscard]] formalism::DomainView get_domain() const noexcept
    {
        URD_ASSUME(domain.has_value());
        return *domain;
    }

    formalism::TypeView object_type;
    formalism::TypeView number_type;
    urd::Store<urd::map<std::string, formalism::TypeView>> types;
    // Sorted alternatives of an (either ...) type -> name of its union type.
    urd::Store<urd::map<urd::Store<urd::sequence<std::string>>, std::string>, urd::sorted, urd::stable_refs> union_type_names;
    urd::Store<urd::map<std::string, formalism::ObjectView>> objects;
    urd::Store<urd::map<std::string, formalism::PredicateView>> predicates;
    urd::Store<urd::map<std::string, formalism::FunctionSkeletonView>> functions;
    urd::Store<urd::set<std::string>> declared_types;
    urd::Store<urd::set<std::string>> declared_objects;
    urd::Store<urd::set<std::string>> declared_predicates;
    urd::Store<urd::set<std::string>> declared_functions;
    urd::Store<urd::set<formalism::RequirementKind>> requirement_kinds;
    bool action_costs = false;
    bool numeric_fluents = false;
};

// Transient state of a single domain or task parse.
struct ParseContext
{
    urd::Store<urd::set<formalism::RequirementKind>> active_requirements;
    urd::Store<urd::set<formalism::RequirementKind>> used_requirements;
    bool active_action_costs = false;
    // Set by :fluents/:numeric-fluents only; bare :action-costs permits reads but restricts
    // numeric writes to (increase (total-cost) ...).
    bool active_numeric_fluents = false;
    urd::Store<urd::map<formalism::VariableView, urd::Store<urd::sequence<formalism::TypeView>>>> variable_types;
    /// The variables in scope by name: a scope per quantifier or parameter list, inner ones shadowing outer ones.
    urd::Store<urd::map<std::string, formalism::VariableView>, urd::scoped> variable_scopes;
    urd::Store<urd::set<std::string>> declared_objects;
    urd::Store<urd::map<std::string, formalism::ObjectView>> task_objects;
    // The sorted alternatives of the (either ...) types of variables: those known before the build, and those it found missing.
    urd::Store<urd::set<urd::Store<urd::sequence<std::string>>>, urd::sorted, urd::stable_refs> union_types;
    urd::Store<urd::set<urd::Store<urd::sequence<std::string>>>, urd::sorted, urd::stable_refs> missing_union_types;
};

void remember_requirement(ParseContext& parse_context, formalism::RequirementKind kind);

formalism::TypeView intern_type(DomainContext& domain_context,
                                formalism::Builder& builder,
                                formalism::Repository& repository,
                                const std::string& name,
                                const urd::Store<urd::sequence<formalism::TypeView>>& bases);

// Repopulates the symbol tables from domain_context.domain after canonicalization.
void rebuild_domain_symbols(DomainContext& domain_context, formalism::Repository& repository);

}  // namespace loki::semantic

#endif
