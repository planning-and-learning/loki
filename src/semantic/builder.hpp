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
#include "loki/formalism/conditions.hpp"
#include "loki/formalism/constraints.hpp"
#include "loki/formalism/declarations.hpp"
#include "loki/formalism/effects.hpp"
#include "loki/formalism/function_expressions.hpp"
#include "loki/formalism/repository.hpp"
#include "loki/formalism/term.hpp"
#include "loki/semantic/options.hpp"

#include <cstddef>
#include <optional>
#include <set>
#include <string>
#include <urd/elements/flat.hpp>
#include <urd/repr/maps/std_map.hpp>
#include <urd/repr/maps/unordered_flat_map.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/repr/values/std_set.hpp>
#include <urd/repr/values/unordered_flat_set.hpp>
#include <urd/store.hpp>
#include <vector>

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
        VariableScope(VariableScope&&) = delete;
        VariableScope& operator=(VariableScope&&) = delete;

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
    [[nodiscard]] SemanticChecks checks() const;

    // Symbol lookup with auto-declaration in non-strict mode.
    formalism::PredicateView predicate(const ast::Identifier& identifier, size_t arity);
    formalism::PredicateView equality_predicate(const ast::Identifier& identifier, size_t arity);
    formalism::FunctionSkeletonView function(const ast::Identifier& identifier, size_t arity);

    // Domain and task sections.
    urd::Store<urd::sequence<formalism::RequirementView>> parse_requirements(const urd::Store<urd::sequence<ast::Requirement>>& nodes);
    urd::Store<urd::sequence<formalism::TypeView>> parse_type_hierarchy(const urd::Store<urd::sequence<ast::TypedGroup>>& groups);
    std::optional<formalism::TypeView> union_type(const urd::Store<urd::sequence<std::string>>& alternatives, const ast::Identifier& identifier);
    urd::Store<urd::sequence<formalism::TypeView>> parse_type_expression(const ast::TypeExpression& type);
    urd::Store<urd::sequence<formalism::TypeView>> parse_type_expression_node(const ast::TypeReference& node);
    urd::Store<urd::sequence<formalism::TypeView>> parse_type_expression_node(const ast::EitherType& node);
    urd::Store<urd::sequence<formalism::ObjectView>> parse_objects(const urd::Store<urd::sequence<ast::TypedGroup>>& groups,
                                                                   urd::Store<urd::map<std::string, formalism::ObjectView>>& table,
                                                                   urd::Store<urd::set<std::string>>& declared_objects);
    urd::Store<urd::sequence<formalism::ParameterView>> parse_parameters(const urd::Store<urd::sequence<ast::TypedGroup>>& groups);
    urd::Store<urd::sequence<formalism::PredicateView>> parse_predicates(const urd::Store<urd::sequence<ast::PredicateDeclaration>>& nodes);
    urd::Store<urd::sequence<formalism::FunctionSkeletonView>> parse_functions(const urd::Store<urd::sequence<ast::FunctionDeclaration>>& nodes);

    // Terms, atoms, and literals.
    formalism::TermView parse_term(const ast::Term& node);
    formalism::AtomView parse_atom(const ast::Atom& node);
    formalism::LiteralView parse_literal(const ast::Literal& node);

    // Conditions.
    formalism::ConditionView parse_condition(const ast::Condition& condition);
    formalism::ConditionView parse_condition_node(const ast::ConditionLiteral& node);
    formalism::ConditionView parse_condition_node(const ast::ConditionAnd& node);
    formalism::ConditionView parse_condition_node(const ast::ConditionOr& node);
    formalism::ConditionView parse_condition_node(const ast::ConditionNot& node);
    formalism::ConditionView parse_condition_node(const ast::ConditionImply& node);
    formalism::ConditionView parse_condition_node(const ast::ConditionExists& node);
    formalism::ConditionView parse_condition_node(const ast::ConditionForall& node);
    formalism::ConditionView parse_condition_node(const ast::ConditionNumericConstraint& node);
    formalism::ConditionView parse_condition_node(const ast::ConditionTimed& node);
    formalism::ConditionView parse_condition_node(const ast::ConditionPreference& node);

    // Numeric expressions.
    formalism::FunctionTermView parse_function_term(const ast::FunctionTerm& node);
    formalism::FunctionExpressionView parse_function_expression(const ast::FunctionExpression& expression);
    formalism::FunctionExpressionView parse_function_expression_node(const ast::FunctionExpressionNumber& node);
    formalism::FunctionExpressionView parse_function_expression_node(const ast::FunctionExpressionFunction& node);
    formalism::FunctionExpressionView parse_function_expression_node(const ast::FunctionExpressionUnary& node);
    formalism::FunctionExpressionView parse_function_expression_node(const ast::FunctionExpressionBinary& node);
    formalism::FunctionExpressionView parse_function_expression_node(const ast::FunctionExpressionMulti& node);
    formalism::FunctionExpressionView parse_function_expression_node(const ast::FunctionExpressionDuration& node);
    formalism::FunctionExpressionView parse_function_expression_node(const ast::FunctionExpressionIsViolated& node);

    // PDDL 3 constraints.
    formalism::ConstraintView parse_constraint(const ast::Constraint& constraint);
    formalism::ConstraintView parse_constraint_node(const ast::ConstraintAnd& node);
    formalism::ConstraintView parse_constraint_node(const ast::ConstraintForall& node);
    formalism::ConstraintView parse_constraint_node(const ast::ConstraintPreference& node);
    formalism::ConstraintView parse_constraint_node(const ast::ConstraintModal& node);
    formalism::ConstraintView parse_constraint_node(const ast::ConstraintOr& node);
    formalism::ConstraintView parse_constraint_node(const ast::ConstraintNot& node);
    formalism::ConstraintView parse_constraint_node(const ast::ConstraintImply& node);
    formalism::ConstraintView parse_constraint_node(const ast::ConstraintExists& node);
    formalism::ConstraintView parse_constraint_node(const ast::ConstraintState& node);
    formalism::ConstraintView negated(formalism::ConstraintView constraint);
    /// The task's goal description, or none when its goal is a formula over the state trajectory, then conjoined with constraints.
    std::optional<formalism::ConditionView> parse_goal(const ast::Constraint& goal, std::optional<formalism::ConstraintView>& constraints);
    /// The constraints of the sections as one: their conjunction if several. None if there are none.
    std::optional<formalism::ConstraintView> parse_constraints(const urd::Store<urd::sequence<ast::Constraint>>& constraints);
    /// The name of a preference, remembered for is-violated; empty if anonymous.
    template<typename Node>
    std::string preference_name(const Node& node);

    // Effects and top-level constructs.
    formalism::EffectView parse_effect(const ast::Effect& effect);
    formalism::EffectView parse_effect_node(const ast::EffectLiteral& node);
    formalism::EffectView parse_effect_node(const ast::EffectAnd& node);
    formalism::EffectView parse_effect_node(const ast::EffectNumeric& node);
    formalism::EffectView parse_effect_node(const ast::EffectForall& node);
    formalism::EffectView parse_effect_node(const ast::EffectWhen& node);
    formalism::EffectView parse_effect_node(const ast::EffectOneOf& node);
    formalism::EffectView parse_effect_node(const ast::EffectProbabilistic& node);
    formalism::EffectView parse_effect_node(const ast::EffectTimed& node);
    formalism::ActionView parse_action(const ast::Action& node);
    formalism::DurativeActionView parse_durative_action(const ast::DurativeAction& node);
    /// A durative action's condition as a conjunction of one timed condition per time point (at start, over all, at
    /// end), each of an untimed condition: (forall (?x) (at t c)) is (at t (forall (?x) c)). None if empty.
    std::optional<formalism::ConditionView> group_timed_condition(formalism::ConditionView condition, const ast::DurativeAction& node);
    /// A durative action's effect as a conjunction of one timed effect per time point (at start, at end), each of an
    /// untimed effect; a conditional effect whose condition is timed at its effect's time point only reads it untimed.
    std::optional<formalism::EffectView> group_timed_effect(formalism::EffectView effect, const ast::DurativeAction& node);
    formalism::AxiomView parse_axiom(const ast::Axiom& node);

    /// The elements of a task's initial state, by kind.
    struct InitialElements
    {
        urd::Store<urd::sequence<formalism::LiteralView>> literals;
        urd::Store<urd::sequence<formalism::InitialFunctionValueView>> values;
        urd::Store<urd::sequence<formalism::TimedInitialLiteralView>> timed_literals;
        /// PPDDL's probabilistic initial elements: probabilistic effects on the initial state.
        urd::Store<urd::sequence<formalism::EntityView<formalism::Effect<formalism::ProbabilisticTag>>>> effects;
    };

    void parse_initial_element(const ast::Literal& literal, InitialElements& elements);
    void parse_initial_element(const ast::InitialFunctionValue& value, InitialElements& elements);
    void parse_initial_element(const ast::TimedInitialLiteral& value, InitialElements& elements);
    void parse_initial_element(const ast::InitialProbabilistic& value, InitialElements& elements);
    formalism::MetricView parse_metric(const ast::Metric& node);
    /// What a metric over the makespan (total-time) or violated preferences alone requires instead of numeric fluents;
    /// none if it reads another function.
    [[nodiscard]] std::optional<formalism::RequirementKind> implicit_metric_requirement(const ast::FunctionExpression& expression) const;

    // Action-costs completion.
    formalism::FunctionSkeletonView total_cost_function();
    /// The number function of the name, declared once: (total-cost) of :action-costs, (reward) of :rewards.
    formalism::FunctionSkeletonView number_function(const std::string& name);
    formalism::FunctionTermView total_cost_term();
    formalism::ActionView add_unit_cost(formalism::ActionView view);
    bool has_total_cost_initial_value(const urd::Store<urd::sequence<formalism::InitialFunctionValueView>>& values);
    void complete_action_costs(const ast::Task& task,
                               urd::Store<urd::sequence<formalism::InitialFunctionValueView>>& initial_function_values,
                               std::optional<formalism::MetricView>& metric);
};

}  // namespace loki::semantic

#endif
