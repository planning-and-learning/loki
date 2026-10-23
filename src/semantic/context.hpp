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
#include <vector>
#include <yggdrasil/containers/associative_containers.hpp>
#include <yggdrasil/semantics/equal_to.hpp>
#include <yggdrasil/semantics/hash.hpp>

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

    formalism::TypeView object_type;
    formalism::TypeView number_type;
    ygg::UnorderedMap<std::string, formalism::TypeView> types;
    // Sorted alternatives of an (either ...) type -> name of its union type.
    std::map<std::vector<std::string>, std::string> union_type_names;
    ygg::UnorderedMap<std::string, formalism::ObjectView> objects;
    ygg::UnorderedMap<std::string, formalism::PredicateView> predicates;
    ygg::UnorderedMap<std::string, formalism::FunctionSkeletonView> functions;
    ygg::UnorderedSet<std::string> declared_types;
    ygg::UnorderedSet<std::string> declared_objects;
    ygg::UnorderedSet<std::string> declared_predicates;
    ygg::UnorderedSet<std::string> declared_functions;
    ygg::UnorderedSet<formalism::RequirementKind> requirement_kinds;
    bool action_costs = false;
    bool numeric_fluents = false;
};

// Transient state of a single domain or task parse.
struct ParseContext
{
    ygg::UnorderedSet<formalism::RequirementKind> active_requirements;
    ygg::UnorderedSet<formalism::RequirementKind> used_requirements;
    bool active_action_costs = false;
    // Set by :fluents/:numeric-fluents only; bare :action-costs permits reads but restricts
    // numeric writes to (increase (total-cost) ...).
    bool active_numeric_fluents = false;
    ygg::UnorderedMap<formalism::VariableView, std::vector<formalism::TypeView>> variable_types;
    std::vector<ygg::UnorderedMap<std::string, formalism::VariableView>> variable_scopes;
    ygg::UnorderedSet<std::string> declared_objects;
    ygg::UnorderedMap<std::string, formalism::ObjectView> task_objects;
    // The sorted alternatives of the (either ...) types of variables: those known before the build, and those it found missing.
    std::set<std::vector<std::string>> union_types;
    std::set<std::vector<std::string>> missing_union_types;
};

void remember_requirement(ParseContext& parse_context, formalism::RequirementKind kind);

formalism::TypeView intern_type(DomainContext& domain_context,
                                formalism::Builder& builder,
                                formalism::Repository& repository,
                                const std::string& name,
                                const std::vector<formalism::TypeView>& bases);

// Repopulates the symbol tables from domain_context.domain after canonicalization.
void rebuild_domain_symbols(DomainContext& domain_context, formalism::Repository& repository);

}  // namespace loki::semantic

#endif
