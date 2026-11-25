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

#include "translator/infer_requirements.hpp"

#include "mappings.hpp"

#include <concepts>
#include <optional>
#include <string_view>
#include <urd/elements/children.hpp>
#include <urd/repr/maps/unordered_flat_map.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/repr/values/std_set.hpp>
#include <urd/repr/values/unordered_flat_set.hpp>
#include <urd/store.hpp>
#include <utility>
#include <variant>

namespace loki::semantic::detail
{
namespace
{

using enum formalism::RequirementKind;
using namespace formalism;

/// What a construct requires by its kind alone.
template<typename T>
inline constexpr auto required = std::optional<RequirementKind> {};
template<>
inline constexpr auto required<Condition<OrTag>> = std::optional(DisjunctivePreconditions);
template<>
inline constexpr auto required<Condition<ImplyTag>> = std::optional(DisjunctivePreconditions);
template<>
inline constexpr auto required<Condition<NotTag>> = std::optional(NegativePreconditions);
template<>
inline constexpr auto required<Condition<ExistsTag>> = std::optional(ExistentialPreconditions);
template<>
inline constexpr auto required<Condition<ForallTag>> = std::optional(UniversalPreconditions);
template<>
inline constexpr auto required<Condition<NumericConstraintTag>> = std::optional(NumericFluents);
template<>
inline constexpr auto required<Effect<OneOfTag>> = std::optional(NonDeterministic);
template<>
inline constexpr auto required<Condition<PreferenceTag>> = std::optional(Preferences);
template<>
inline constexpr auto required<Constraint<PreferenceTag>> = std::optional(Preferences);
template<>
inline constexpr auto required<FunctionExpression<IsViolatedTag>> = std::optional(Preferences);
template<>
inline constexpr auto required<Constraint<ForallTag>> = std::optional(UniversalPreconditions);
template<>
inline constexpr auto required<Effect<ForallTag>> = std::optional(UniversalPreconditions);
template<>
inline constexpr auto required<Effect<WhenTag>> = std::optional(ConditionalEffects);
template<>
inline constexpr auto required<Effect<ProbabilisticTag>> = std::optional(ProbabilisticEffects);

/// Mirrors the requirement checks of the strict parser (AstBuilder and SemanticChecks): each construct adds what parsing it requires
/// or marks used, so that the result declares every requirement the reparse demands and none that it would reject as unused.
struct RequirementCollector
{
    urd::Store<urd::set<RequirementKind>, urd::sorted, urd::stable_refs> requirements;

    /// Whether (reward) is PPDDL's reward fluent: the source declared :rewards; whether (total-time) is the makespan: the
    /// source declared :durative-actions.
    bool rewards = false;
    bool durative = false;

    static bool is_total_cost(FunctionSkeletonView function) { return std::string_view(function.get_name()) == "total-cost"; }

    [[nodiscard]] bool is_reward(FunctionSkeletonView function) const { return rewards && std::string_view(function.get_name()) == "reward"; }

    /// What a function requires: :action-costs for (total-cost), :rewards for PPDDL's (reward), else :numeric-fluents.
    [[nodiscard]] bool is_total_time(FunctionSkeletonView function) const
    { return durative && std::string_view(function.get_name()) == "total-time" && function.get_parameters().empty(); }

    [[nodiscard]] RequirementKind function_requirement(FunctionSkeletonView function) const
    { return is_total_cost(function) ? ActionCosts : is_reward(function) ? Rewards : is_total_time(function) ? DurativeActions : NumericFluents; }

    /// A construct: what its kind requires, then what its children require.
    template<typename T>
    void visit(EntityView<T> node)
    {
        if (required<T>)
            requirements.insert(*required<T>);
        urd::for_each_child<SymbolRepositoryTypes, T>(node, [&](auto child) { visit(child); });
    }

    /// Whether the formula visited is directly an operand of a PDDL 3 operator, where a goal description stands.
    bool pddl3_operand = false;

    /// A formula over the state trajectory: beyond PDDL 3 (:trajectory-formulas) a goal description outside an operand
    /// of a PDDL 3 operator, anything else inside one, disjunction, negation, existentials and the trajectory operators.
    template<typename Tag>
        requires boost::mp11::mp_contains<ConstraintTags, Tag>::value
    void visit(EntityView<Constraint<Tag>> node)
    {
        const auto operand = std::exchange(pddl3_operand, false);
        if constexpr (std::same_as<Tag, StateTag>)
        {
            if (!operand)
                requirements.insert(TrajectoryFormulas);
            visit(node.get_condition());
        }
        else
        {
            if (operand)
                requirements.insert(TrajectoryFormulas);
            if constexpr (std::same_as<Tag, OrTag> || std::same_as<Tag, NotTag>)
                requirements.insert(TrajectoryFormulas);
            if constexpr (std::same_as<Tag, OrTag>)
                requirements.insert(DisjunctivePreconditions);
            if constexpr (std::same_as<Tag, ExistsTag>)
            {
                requirements.insert(TrajectoryFormulas);
                requirements.insert(ExistentialPreconditions);
            }
            if constexpr (std::same_as<Tag, ModalTag>)
            {
                const auto trajectory = is_trajectory_operator(node.get_operation());
                requirements.insert(trajectory ? TrajectoryFormulas : Constraints);
                for (const auto child : node.get_constraints())
                {
                    pddl3_operand = !trajectory;
                    visit(child);
                }
                pddl3_operand = false;
            }
            else
            {
                if (required<Constraint<Tag>>)
                    requirements.insert(*required<Constraint<Tag>>);
                urd::for_each_child<SymbolRepositoryTypes, Constraint<Tag>>(node, [&](auto child) { visit(child); });
            }
        }
        pddl3_operand = operand;
    }

    template<typename... Vs>
    void visit(const std::variant<Vs...>& node)
    {
        std::visit([&](auto alternative) { visit(alternative); }, node);
    }

    void visit(ParameterView parameter)
    {
        if (!parameter.get_types().empty())
            requirements.insert(Typing);
    }

    void visit(ObjectView object)
    {
        if (!object.get_types().empty())
            requirements.insert(Typing);
    }

    void visit(TypeView type)
    {
        const auto name = std::string_view(type.get_name());
        if (name != "object" && name != "number")
            requirements.insert(Typing);
    }

    /// An atom's predicate and terms require nothing beyond the atom: their declarations are visited on their own.
    void visit(AtomView atom)
    {
        if (std::string_view(atom.get_predicate().get_name()) == "=")
            requirements.insert(Equality);
    }

    void visit(FunctionSkeletonView function)
    {
        requirements.insert(function_requirement(function));
        for (const auto parameter : function.get_parameters())
            visit(parameter);
    }

    void visit(FunctionTermView term) { requirements.insert(function_requirement(term.get_function())); }

    void visit(EntityView<Condition<LiteralTag>> node)
    {
        if (!node.get_literal().get_polarity())
            requirements.insert(NegativePreconditions);
        visit(node.get_literal());
    }

    void visit(EntityView<Effect<NumericTag>> node)
    {
        // Bare :action-costs permits only writes of the form (increase (total-cost) ...).
        const auto increases_total_cost = node.get_operation() == NumericEffectOperator::Increase && is_total_cost(node.get_function().get_function());
        requirements.insert(increases_total_cost ? ActionCosts : is_reward(node.get_function().get_function()) ? Rewards : NumericFluents);
        visit(node.get_function());
        visit(node.get_expression());
    }

    void visit(AxiomView axiom)
    {
        requirements.insert(DerivedPredicates);
        for (const auto parameter : axiom.get_parameters())
            visit(parameter);
        visit(axiom.get_condition());
        // The printer writes the parameters that are not in the head as an exists around the condition.
        auto head_variables = urd::Store<urd::set<VariableView>> {};
        for (const auto term : axiom.get_head().get_atom().get_terms())
            if (const auto* variable = std::get_if<VariableView>(&term))
                head_variables.insert(*variable);
        for (const auto parameter : axiom.get_parameters())
            if (!head_variables.contains(parameter.get_variable()))
                requirements.insert(ExistentialPreconditions);
    }

    /// Whether the expression reads the makespan (total-time).
    [[nodiscard]] bool has_total_time(FunctionExpressionView expression) const
    {
        return std::visit(
            [&]<typename Node>(const Node& part) -> bool
            {
                if constexpr (std::same_as<Node, FunctionTermView>)
                    return is_total_time(part.get_function());
                else if constexpr (std::same_as<Node, EntityView<FunctionExpression<UnaryTag>>>)
                    return has_total_time(part.get_expression());
                else if constexpr (std::same_as<Node, EntityView<FunctionExpression<BinaryTag>>>)
                    return has_total_time(part.get_left()) || has_total_time(part.get_right());
                else if constexpr (std::same_as<Node, EntityView<FunctionExpression<MultiTag>>>)
                {
                    for (const auto argument : part.get_args())
                        if (has_total_time(argument))
                            return true;
                    return false;
                }
                else
                    return false;
            },
            expression);
    }

    /// A metric uses numeric fluents even if it is a constant; (maximize (reward)) only PPDDL's :rewards; one over the
    /// makespan and violated preferences alone only :durative-actions and :preferences.
    void visit(MetricView metric)
    {
        const auto expression = metric.get_expression();
        const auto* term = std::get_if<FunctionTermView>(&expression);
        const auto reward = term && is_reward(term->get_function());
        // Over violated preferences alone (PDDL 3), the metric needs :preferences only, which is-violated adds.
        auto violations = false;
        auto functions = false;
        const auto scan = [&](const auto& self, FunctionExpressionView node) -> void
        {
            std::visit(
                [&]<typename Node>(const Node& part)
                {
                    if constexpr (std::same_as<Node, EntityView<FunctionExpression<IsViolatedTag>>>)
                        violations = true;
                    else if constexpr (std::same_as<Node, FunctionTermView>)
                        functions = functions || !is_total_time(part.get_function());
                    else if constexpr (std::same_as<Node, EntityView<FunctionExpression<DurationTag>>>)
                        functions = true;
                    else if constexpr (std::same_as<Node, EntityView<FunctionExpression<UnaryTag>>>)
                        self(self, part.get_expression());
                    else if constexpr (std::same_as<Node, EntityView<FunctionExpression<BinaryTag>>>)
                    {
                        self(self, part.get_left());
                        self(self, part.get_right());
                    }
                    else if constexpr (std::same_as<Node, EntityView<FunctionExpression<MultiTag>>>)
                    {
                        for (const auto argument : part.get_args())
                            self(self, argument);
                    }
                },
                node);
        };
        scan(scan, expression);
        if (!reward && functions)
            requirements.insert(NumericFluents);
        if (!reward && !functions && !violations && !has_total_time(expression))
            requirements.insert(NumericFluents);
        visit(metric.get_expression());
    }

    void visit(DurativeActionView action)
    {
        requirements.insert(DurativeActions);
        for (const auto parameter : action.get_parameters())
            visit(parameter);
        if (const auto duration = action.get_duration())
            visit_duration(duration.value());
        if (const auto condition = action.get_condition())
            visit(condition.value());
        if (const auto effect = action.get_effect())
            visit(effect.value());
    }

    /// A duration constraint: a conjunction of (timed) comparisons of ?duration, which require numeric fluents only by the
    /// function terms they read, and :duration-inequalities unless they are equalities.
    void visit_duration(ConditionView duration)
    {
        std::visit(
            [&]<typename Node>(const Node& node)
            {
                if constexpr (std::same_as<Node, EntityView<Condition<AndTag>>>)
                {
                    for (const auto child : node.get_conditions())
                        visit_duration(child);
                }
                else if constexpr (std::same_as<Node, EntityView<Condition<TimedTag>>>)
                {
                    visit_duration(node.get_condition());
                }
                else if constexpr (std::same_as<Node, EntityView<Condition<NumericConstraintTag>>>)
                {
                    if (node.get_comparator() != BinaryComparator::Eq)
                        requirements.insert(DurationInequalities);
                    visit(node.get_left());
                    visit(node.get_right());
                }
                else
                {
                    visit(node);
                }
            },
            duration);
    }

    void visit(TimedInitialLiteralView literal)
    {
        requirements.insert(TimedInitialLiterals);
        visit(literal.get_literal());
    }

    /// Declared predicates: what their parameters require.
    void predicates(EntityListView<Predicate> predicates)
    {
        for (const auto predicate : predicates)
            for (const auto parameter : predicate.get_parameters())
                visit(parameter);
    }

    template<typename Range>
    void visit_all(const Range& nodes)
    {
        for (const auto node : nodes)
            visit(node);
    }
};

}  // namespace

/// Whether the requirements provide a capability.
bool declares(EntityListView<Requirement> requirements, RequirementKind capability)
{
    for (const auto requirement : requirements)
        for (const auto provided : requirement_capabilities(requirement.get_kind()))
            if (provided == capability)
                return true;
    return false;
}

/// Whether the requirements provide PPDDL's (reward).
bool declares_rewards(EntityListView<Requirement> requirements) { return declares(requirements, Rewards); }

urd::Store<urd::sequence<RequirementKind>> infer_requirements(DomainView domain)
{
    auto collector = RequirementCollector {};
    collector.rewards = declares_rewards(domain.get_requirements());
    collector.durative = declares(domain.get_requirements(), DurativeActions);
    // The strict parser demands :strips of every domain.
    collector.requirements.insert(Strips);
    collector.visit_all(domain.get_types());
    collector.visit_all(domain.get_constants());
    collector.predicates(domain.get_predicates());
    collector.visit_all(domain.get_functions());
    collector.visit_all(domain.get_actions());
    collector.visit_all(domain.get_durative_actions());
    collector.visit_all(domain.get_axioms());
    if (const auto constraints = domain.get_constraints())
        collector.visit(constraints.value());
    return { collector.requirements.begin(), collector.requirements.end() };
}

urd::Store<urd::sequence<RequirementKind>> infer_requirements(TaskView task, DomainView domain)
{
    auto collector = RequirementCollector {};
    collector.rewards = declares_rewards(domain.get_requirements()) || declares_rewards(task.get_requirements());
    collector.durative = declares(domain.get_requirements(), DurativeActions);
    if (task.get_goal_reward())
        collector.requirements.insert(Rewards);
    collector.visit_all(task.get_types());
    collector.predicates(task.get_predicates());
    collector.visit_all(task.get_objects());
    collector.visit_all(task.get_initial_literals());
    collector.visit_all(task.get_initial_function_values());
    collector.visit_all(task.get_timed_initial_literals());
    collector.visit_all(task.get_initial_effects());
    if (const auto goal = task.get_goal())
        collector.visit(goal.value());
    if (const auto metric = task.get_metric())
        collector.visit(metric.value());
    collector.visit_all(task.get_axioms());
    if (const auto constraints = task.get_constraints())
        collector.visit(constraints.value());

    // What a domain requirement provides needs no declaration in the task, and declaring it again is redundant.
    collector.requirements.erase(Strips);
    for (const auto requirement : domain.get_requirements())
        for (const auto capability : requirement_capabilities(requirement.get_kind()))
            collector.requirements.erase(capability);
    return { collector.requirements.begin(), collector.requirements.end() };
}

}  // namespace loki::semantic::detail
