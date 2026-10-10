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

#ifndef LOKI_SEMANTIC_BUILDER_HPP_
#define LOKI_SEMANTIC_BUILDER_HPP_

#include "loki/ast/ast_fwd.hpp"
#include "loki/formalism/repository.hpp"
#include "loki/formalism/condition_data.hpp"
#include "loki/formalism/declarations.hpp"
#include "loki/formalism/effect_data.hpp"
#include "loki/formalism/function_expression_data.hpp"
#include "loki/semantic/options.hpp"

#include <cista/containers/optional.h>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include <yggdrasil/containers/associative_containers.hpp>
#include <yggdrasil/core/types.hpp>

namespace loki::semantic
{

struct DiagnosticContext;
struct DomainContext;
struct ParseContext;
struct SemanticChecks;

// Translates a syntactic AST into (un-canonicalized) formalism entities inside the
// supplied repository. Storage lifecycle and canonicalization belong to the caller.
class AstBuilder
{
public:
    AstBuilder(const ParserOptions& options,
               const DiagnosticContext& diagnostics,
               formalism::Repository& repository,
               DomainContext& domain_context,
               ParseContext& parse_context);

    formalism::DomainView build_domain(const ast::Domain& domain);
    formalism::TaskView build_task(const ast::Task& task);

private:
    class VariableScope
    {
    public:
        explicit VariableScope(ParseContext& context);
        ~VariableScope();

        VariableScope(const VariableScope&) = delete;
        VariableScope& operator=(const VariableScope&) = delete;

    private:
        ParseContext& m_context;
    };

    const ParserOptions& m_options;
    const DiagnosticContext& m_diagnostics;
    formalism::Builder m_builder;
    formalism::Repository& m_repository;
    DomainContext& m_domain_context;
    ParseContext& m_parse_context;

    formalism::Repository& repo() noexcept;
    SemanticChecks checks() const;

    template<typename T>
    static void append_indices(const std::vector<formalism::EntityView<T>>& views, ygg::IndexList<T>& indices);

    template<typename T>
    static cista::optional<ygg::Index<T>> to_optional_index(const std::optional<formalism::EntityView<T>>& view);

    // Symbol lookup with auto-declaration in non-strict mode.
    formalism::PredicateView predicate(const ast::Identifier& identifier, size_t arity);
    formalism::PredicateView equality_predicate(const ast::Identifier& identifier, size_t arity);
    formalism::FunctionSkeletonView function(const ast::Identifier& identifier, size_t arity);

    // Domain and task sections.
    std::vector<formalism::RequirementView> parse_requirements(const std::vector<ast::Requirement>& nodes);
    std::vector<formalism::TypeView> parse_types(const std::vector<ast::TypedGroup>& groups);
    std::vector<formalism::TypeView> parse_type_expression(const ast::TypeExpression& type);
    std::vector<formalism::TypeView> parse_type_expression_node(const ast::TypeReference& node);
    std::vector<formalism::TypeView> parse_type_expression_node(const ast::EitherType& node);
    std::vector<formalism::ObjectView> parse_objects(const std::vector<ast::TypedGroup>& groups,
                                                     ygg::UnorderedMap<std::string, formalism::ObjectView>& table,
                                                     ygg::UnorderedSet<std::string>& declared_objects);
    std::vector<formalism::ParameterView> parse_parameters(const std::vector<ast::TypedGroup>& groups);
    std::vector<formalism::PredicateView> parse_predicates(const std::vector<ast::PredicateDeclaration>& nodes);
    std::vector<formalism::FunctionSkeletonView> parse_functions(const std::vector<ast::FunctionDeclaration>& nodes);

    // Terms, atoms, and literals.
    formalism::TermView parse_term(const ast::Term& node);
    formalism::AtomView parse_atom(const ast::Atom& node);
    formalism::LiteralView parse_literal(const ast::Literal& node);

    // Conditions.
    formalism::ConditionView parse_condition(const ast::Condition& condition);
    formalism::ConditionView wrap_condition(ygg::Data<formalism::Condition<>>::Variant value);
    formalism::ConditionView parse_condition_node(const ast::ConditionLiteral& node);
    formalism::ConditionView parse_condition_node(const ast::ConditionAnd& node);
    formalism::ConditionView parse_condition_node(const ast::ConditionOr& node);
    formalism::ConditionView parse_condition_node(const ast::ConditionNot& node);
    formalism::ConditionView parse_condition_node(const ast::ConditionImply& node);
    formalism::ConditionView parse_condition_node(const ast::ConditionExists& node);
    formalism::ConditionView parse_condition_node(const ast::ConditionForall& node);
    formalism::ConditionView parse_condition_node(const ast::ConditionNumericConstraint& node);

    // Numeric expressions.
    formalism::FunctionTermView parse_function_term(const ast::FunctionTerm& node);
    formalism::FunctionExpressionView parse_function_expression(const ast::FunctionExpression& expression);
    formalism::FunctionExpressionView wrap_function_expression(ygg::Data<formalism::FunctionExpression<>>::Variant value);
    formalism::FunctionExpressionView parse_function_expression_node(const ast::FunctionExpressionNumber& node);
    formalism::FunctionExpressionView parse_function_expression_node(const ast::FunctionExpressionFunction& node);
    formalism::FunctionExpressionView parse_function_expression_node(const ast::FunctionExpressionUnary& node);
    formalism::FunctionExpressionView parse_function_expression_node(const ast::FunctionExpressionBinary& node);
    formalism::FunctionExpressionView parse_function_expression_node(const ast::FunctionExpressionMulti& node);

    // Effects and top-level constructs.
    formalism::EffectView parse_effect(const ast::Effect& effect);
    formalism::EffectView wrap_effect(ygg::Data<formalism::Effect<>>::Variant value);
    formalism::EffectView parse_effect_node(const ast::EffectLiteral& node);
    formalism::EffectView parse_effect_node(const ast::EffectAnd& node);
    formalism::EffectView parse_effect_node(const ast::EffectNumeric& node);
    formalism::EffectView parse_effect_node(const ast::EffectForall& node);
    formalism::EffectView parse_effect_node(const ast::EffectWhen& node);
    formalism::EffectView parse_effect_node(const ast::EffectOneOf& node);
    formalism::EffectView parse_effect_node(const ast::EffectProbabilistic& node);
    formalism::ActionView parse_action(const ast::Action& node);
    formalism::AxiomView parse_axiom(const ast::Axiom& node);
    void parse_initial_element(const ast::Literal& literal, std::vector<formalism::LiteralView>& literals, std::vector<formalism::InitialFunctionValueView>&);
    void parse_initial_element(const ast::InitialFunctionValue& value,
                               std::vector<formalism::LiteralView>&,
                               std::vector<formalism::InitialFunctionValueView>& values);
    formalism::MetricView parse_metric(const ast::Metric& node);

    // Action-costs completion.
    formalism::FunctionSkeletonView total_cost_function();
    formalism::FunctionTermView total_cost_term();
    bool writes_total_cost(formalism::EffectView effect);
    formalism::ActionView add_unit_cost(formalism::ActionView action);
    bool has_total_cost_initial_value(const std::vector<formalism::InitialFunctionValueView>& values);
    void complete_action_costs(const ast::Task& task,
                               std::vector<formalism::InitialFunctionValueView>& initial_function_values,
                               std::optional<formalism::MetricView>& metric);
};

template<typename T>
void AstBuilder::append_indices(const std::vector<formalism::EntityView<T>>& views, ygg::IndexList<T>& indices)
{
    indices.reserve(indices.size() + views.size());
    for (const auto view : views)
        indices.push_back(view.get_index());
}

template<typename T>
cista::optional<ygg::Index<T>> AstBuilder::to_optional_index(const std::optional<formalism::EntityView<T>>& view)
{
    return view ? cista::optional<ygg::Index<T>> { view->get_index() } : cista::optional<ygg::Index<T>> {};
}

}  // namespace loki::semantic

#endif
