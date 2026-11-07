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

#include "loki/semantic/translator/infer_requirements.hpp"

#include "mappings.hpp"

#include <optional>
#include <string_view>
#include <urd/elements/children.hpp>
#include <variant>
#include <urd/repr/maps/unordered_flat_map.hpp>
#include <urd/repr/values/std_set.hpp>
#include <urd/repr/values/unordered_flat_set.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

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
inline constexpr auto required<Effect<ForallTag>> = std::optional(UniversalPreconditions);
template<>
inline constexpr auto required<Effect<WhenTag>> = std::optional(ConditionalEffects);
template<>
inline constexpr auto required<Effect<ProbabilisticTag>> = std::optional(ProbabilisticEffects);
template<>
inline constexpr auto required<Metric> = std::optional(NumericFluents);

/// Mirrors the requirement checks of the strict parser (AstBuilder and SemanticChecks): each construct adds what parsing it requires
/// or marks used, so that the result declares every requirement the reparse demands and none that it would reject as unused.
struct RequirementCollector
{
    urd::Store<urd::set<RequirementKind>, urd::sorted, urd::stable_refs> requirements;

    static bool is_total_cost(FunctionSkeletonView function) { return std::string_view(function.get_name()) == "total-cost"; }

    /// A construct: what its kind requires, then what its children require.
    template<typename T>
    void visit(EntityView<T> node)
    {
        if (required<T>)
            requirements.insert(*required<T>);
        urd::for_each_child<SymbolRepositoryTypes, T>(node.data(), node.context(), [&](auto child) { visit(child); });
    }

    template<typename... Vs>
    void visit(const std::variant<Vs...>& node)
    { std::visit([&](auto alternative) { visit(alternative); }, node); }

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
        requirements.insert(is_total_cost(function) ? ActionCosts : NumericFluents);
        for (const auto parameter : function.get_parameters())
            visit(parameter);
    }

    void visit(FunctionTermView term) { requirements.insert(is_total_cost(term.get_function()) ? ActionCosts : NumericFluents); }

    void visit(EntityView<Condition<LiteralTag>> node)
    {
        if (!node.get_literal().get_polarity())
            requirements.insert(NegativePreconditions);
        visit(node.get_literal());
    }

    void visit(EntityView<Effect<NumericTag>> node)
    {
        // Bare :action-costs permits only writes of the form (increase (total-cost) ...).
        const auto increases_total_cost = node.get_operator() == NumericEffectOperator::Increase && is_total_cost(node.get_function().get_function());
        requirements.insert(increases_total_cost ? ActionCosts : NumericFluents);
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

urd::Store<urd::sequence<RequirementKind>> infer_requirements(DomainView domain)
{
    auto collector = RequirementCollector {};
    // The strict parser demands :strips of every domain.
    collector.requirements.insert(Strips);
    collector.visit_all(domain.get_types());
    collector.visit_all(domain.get_constants());
    collector.predicates(domain.get_predicates());
    collector.visit_all(domain.get_functions());
    collector.visit_all(domain.get_actions());
    collector.visit_all(domain.get_axioms());
    return { collector.requirements.begin(), collector.requirements.end() };
}

urd::Store<urd::sequence<RequirementKind>> infer_requirements(TaskView task, DomainView domain)
{
    auto collector = RequirementCollector {};
    collector.visit_all(task.get_types());
    collector.predicates(task.get_predicates());
    collector.visit_all(task.get_objects());
    collector.visit_all(task.get_initial_literals());
    collector.visit_all(task.get_initial_function_values());
    if (const auto goal = task.get_goal())
        collector.visit(goal.value());
    if (const auto metric = task.get_metric())
        collector.visit(metric.value());
    collector.visit_all(task.get_axioms());

    // What a domain requirement provides needs no declaration in the task, and declaring it again is redundant.
    collector.requirements.erase(Strips);
    for (const auto requirement : domain.get_requirements())
        for (const auto capability : requirement_capabilities(requirement.get_kind()))
            collector.requirements.erase(capability);
    return { collector.requirements.begin(), collector.requirements.end() };
}

}  // namespace loki::semantic::detail
