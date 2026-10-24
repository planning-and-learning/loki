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

#ifndef LOKI_SEMANTIC_CHECKS_HPP_
#define LOKI_SEMANTIC_CHECKS_HPP_

#include "ast_mappings.hpp"
#include "context.hpp"
#include "diagnostics.hpp"
#include "loki/ast/ast.hpp"
#include "loki/formalism/object_view.hpp"
#include "loki/formalism/parameter_view.hpp"
#include "loki/formalism/type_view.hpp"
#include "loki/formalism/variable_view.hpp"
#include "loki/semantic/errors.hpp"
#include "loki/semantic/options.hpp"
#include "mappings.hpp"

#include <boost/optional.hpp>
#include <cstddef>
#include <string>
#include <vector>
#include <yggdrasil/containers/associative_containers.hpp>

namespace loki::semantic
{

// Symbol lookups that fail with a positioned diagnostic.
inline formalism::VariableView lookup_variable(const ParseContext& parse_context, const DiagnosticContext& diagnostics, const ast::Identifier& identifier)
{
    const auto name = key(identifier.text);
    for (auto it = parse_context.variable_scopes.rbegin(); it != parse_context.variable_scopes.rend(); ++it)
        if (auto found = it->find(name); found != it->end())
            return found->second;
    diagnostics.throw_at(identifier, UndefinedVariableError(identifier.text));
}

inline formalism::ObjectView
lookup_object(const DomainContext& domain_context, const ParseContext& parse_context, const DiagnosticContext& diagnostics, const ast::Identifier& identifier)
{
    const auto name = key(identifier.text);
    if (auto it = parse_context.task_objects.find(name); it != parse_context.task_objects.end())
        return it->second;
    if (auto it = domain_context.objects.find(name); it != domain_context.objects.end())
        return it->second;
    diagnostics.throw_at(identifier, UndefinedObjectError(identifier.text));
}

struct SemanticChecks
{
    const ParserOptions& options;
    const DiagnosticContext& diagnostics;
    const DomainContext& domain_context;
    ParseContext& parse_context;

    void mark_requirement_used(formalism::RequirementKind kind) const { parse_context.used_requirements.insert(kind); }

    bool uses_declared_requirement(const ast::Requirement& node) const
    {
        for (const auto capability : requirement_capabilities(node, diagnostics))
            if (parse_context.used_requirements.contains(capability))
                return true;
        return false;
    }

    void reject_unused_requirements(const std::vector<ast::Requirement>& nodes) const
    {
        if (!options.strict)
            return;
        for (const auto& node : nodes)
        {
            if (node.name.text == "strips")  // baseline: mandatory in strict mode, never audited as unused
                continue;
            if (!uses_declared_requirement(node))
                diagnostics.throw_at(node.name, UnusedRequirementError(node.name.text));
        }
    }

    template<typename Node>
    void require_requirement(formalism::RequirementKind kind, const Node& node) const
    {
        mark_requirement_used(kind);
        // Permissive mode never enforces requirement declarations; strict mode does.
        if (!options.strict)
            return;
        if (!parse_context.active_requirements.contains(kind))
            diagnostics.throw_at(node, MissingRequirementError(requirement_name(kind)));
    }

    void require_typing_if_needed(const boost::optional<ast::TypeExpression>& type, const boost::spirit::x3::position_tagged& node) const
    {
        if (type)
            require_requirement(formalism::RequirementKind::Typing, node);
    }

    template<typename Node>
    void check_argument_types(const std::string& name,
                              formalism::EntityListView<formalism::Parameter> parameters,
                              const std::vector<ast::Term>& terms,
                              const Node& node) const
    {
        if (!options.strict)
            return;
        for (auto i = std::size_t { 0 }; i < terms.size(); ++i)
        {
            const auto actual = term_types(terms[i]);
            if (!types_compatible(actual, parameters[i].get_types()))
                diagnostics.throw_at(node, TypeMismatchError(name));
        }
    }

    template<typename Error, typename Node>
    void ensure_new(ygg::UnorderedSet<std::string>& names, std::string name, const Node& node) const
    {
        if (!names.insert(name).second)
            diagnostics.throw_at(node, Error(name));
    }

    template<typename Node>
    void ensure_arity(const std::string& name, std::size_t expected, std::size_t actual, const Node& node) const
    {
        if (expected != actual)
            diagnostics.throw_at(node, ArityMismatchError(name, expected, actual));
    }

    std::vector<formalism::TypeView> term_types(const ast::Term& term) const
    {
        if (!term.variable)
        {
            auto result = std::vector<formalism::TypeView> {};
            for (auto type : lookup_object(domain_context, parse_context, diagnostics, term.name).get_types())
                result.push_back(type);
            return result;
        }
        auto variable_view = lookup_variable(parse_context, diagnostics, term.name);
        if (auto it = parse_context.variable_types.find(variable_view); it != parse_context.variable_types.end())
        {
            return it->second;
        }
        return std::vector<formalism::TypeView> { domain_context.types.at("object") };
    }

    bool types_compatible(const std::vector<formalism::TypeView>& actual_types, formalism::EntityListView<formalism::Type> expected_types) const
    {
        if (expected_types.empty() || actual_types.empty())
            return true;
        for (auto actual : actual_types)
            for (auto expected : expected_types)
                if (is_subtype(actual, expected))
                    return true;
        return false;
    }

    bool is_subtype(formalism::TypeView actual, formalism::TypeView expected) const
    {
        auto seen = ygg::UnorderedSet<formalism::TypeView> {};
        return is_subtype(actual, expected, seen);
    }

    bool is_subtype(formalism::TypeView actual, formalism::TypeView expected, ygg::UnorderedSet<formalism::TypeView>& seen) const
    {
        if (actual == expected)
            return true;
        if (!seen.insert(actual).second)
            return false;
        for (auto base : actual.get_bases())
            if (is_subtype(base, expected, seen))
                return true;
        return false;
    }
};

}  // namespace loki::semantic

#endif
