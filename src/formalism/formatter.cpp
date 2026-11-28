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

// How each record of loki's formalism is written: in PDDL, as a domain and a problem file write it (the formatters are
// declared beside each record's view).

#include "loki/formalism/conditions.hpp"
#include "loki/formalism/constraints.hpp"
#include "loki/formalism/effects.hpp"
#include "loki/formalism/function_expressions.hpp"
#include "loki/formalism/repository.hpp"
#include "loki/formalism/term.hpp"
#include "loki/formalism/views.hpp"

#include <concepts>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>
#include <urd/repr/maps/std_map.hpp>
#include <urd/repr/maps/unordered_flat_map.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/repr/values/std_set.hpp>
#include <urd/repr/values/unordered_flat_set.hpp>
#include <urd/store.hpp>
#include <variant>
#include <yggdrasil/formatting/cista_formatters.hpp>
#include <yggdrasil/formatting/formatter.hpp>
#include <yggdrasil/io/iostream.hpp>

namespace lf = loki::formalism;

namespace loki::formalism
{
namespace
{

bool is_builtin_type(TypeView type)
{
    const auto name = std::string_view(type.name().data(), type.name().size());
    return name == "object" || name == "number";
}

/// A range as " e1 e2 ...", a space before each element; an empty range writes nothing.
template<typename Out, typename Range>
Out spaced(Out out, const Range& range)
{
    for (auto element : range)
        out = fmt::format_to(out, " {}", element);
    return out;
}

/// " - type", or " - (either t1 t2 ...)": several types need an either to read back.
template<typename Out, typename Types>
Out type_annotation(Out out, const Types& types)
{
    if (types.size() == 1)
        return fmt::format_to(out, " - {}", types[0]);
    return fmt::format_to(spaced(fmt::format_to(out, " - (either"), types), ")");
}

/// Whether the condition is a timed condition or a conjunction of them (the condition of a conditional effect of a
/// durative action that reads another time point than its effect's).
bool is_timed(ConditionView condition)
{
    if (std::holds_alternative<EntityView<Condition<TimedTag>>>(condition))
        return true;
    const auto* conjunction = std::get_if<EntityView<Condition<AndTag>>>(&condition);
    if (!conjunction || conjunction->conditions().empty())
        return false;
    for (auto conjunct : conjunction->conditions())
        if (!std::holds_alternative<EntityView<Condition<TimedTag>>>(conjunct))
            return false;
    return true;
}

/// An effect at a time point in PDDL 2.1's form: the time specifier on each simple effect, conjunctions, universal
/// effects and conditional effects outside it, and an untimed condition of a conditional effect at the effect's time.
template<typename Out>
Out write_timed_effect(Out out, TimeSpecifier time, EffectView effect)
{
    return visit(
        [&]<typename Node>(const Node& node) -> Out
        {
            if constexpr (std::same_as<Node, EntityView<Effect<AndTag>>>)
            {
                out = fmt::format_to(out, "(and");
                for (auto part : node.effects())
                    out = write_timed_effect(fmt::format_to(out, " "), time, part);
                return fmt::format_to(out, ")");
            }
            else if constexpr (std::same_as<Node, EntityView<Effect<ForallTag>>>)
                return fmt::format_to(write_timed_effect(fmt::format_to(out, "(forall ({}) ", fmt::join(node.parameters(), " ")), time, node.effect()), ")");
            else if constexpr (std::same_as<Node, EntityView<Effect<WhenTag>>>)
            {
                out = is_timed(node.condition()) ? fmt::format_to(out, "(when {} ", node.condition())
                                                     : fmt::format_to(out, "(when ({} {}) ", time, node.condition());
                return fmt::format_to(write_timed_effect(out, time, node.effect()), ")");
            }
            else
                return fmt::format_to(out, "({} {})", time, node);
        },
        effect);
}

/// A probabilistic initial element (PPDDL) as :init writes it: function values as (= f n), not as assignments.
template<typename Out>
Out write_initial_effect(Out out, EffectView effect)
{
    return visit(
        [&]<typename Node>(const Node& node) -> Out
        {
            if constexpr (std::same_as<Node, EntityView<Effect<ProbabilisticTag>>>)
            {
                out = fmt::format_to(out, "(probabilistic");
                for (auto alternative : node.alternatives())
                    out = write_initial_effect(fmt::format_to(out, " {} ", alternative.probability()), alternative.effect());
                return fmt::format_to(out, ")");
            }
            else if constexpr (std::same_as<Node, EntityView<Effect<AndTag>>>)
            {
                out = fmt::format_to(out, "(and");
                for (auto part : node.effects())
                    out = write_initial_effect(fmt::format_to(out, " "), part);
                return fmt::format_to(out, ")");
            }
            else if constexpr (std::same_as<Node, EntityView<Effect<NumericTag>>>)
                return fmt::format_to(out, "(= {} {})", node.function(), node.expression());
            else
                return fmt::format_to(out, "{}", node);
        },
        effect);
}

/// A section's objects (:constants or :objects), each with its types.
template<typename Objects>
void write_objects(std::ostream& os, std::string_view section, const Objects& objects)
{
    auto text = fmt::memory_buffer {};
    auto out = fmt::format_to(std::back_inserter(text), "({} ", section);
    auto first = true;
    for (auto object : objects)
    {
        out = fmt::format_to(out, "{}{}", first ? "" : " ", object);
        first = false;
        if (!object.types().empty())
            out = type_annotation(out, object.types());
    }
    fmt::format_to(out, ")");
    os << '\n' << ygg::print_indent << fmt::to_string(text);
}

/// A section's types (:types), each with its bases, the builtin ones left out where skip_builtin.
template<typename Types>
void write_types(std::ostream& os, const Types& types, bool skip_builtin)
{
    auto text = fmt::memory_buffer {};
    auto out = std::back_inserter(text);
    auto wrote_header = false;
    for (auto type : types)
    {
        if (skip_builtin && is_builtin_type(type))
            continue;
        if (!wrote_header)
            out = fmt::format_to(out, "(:types");
        wrote_header = true;
        out = fmt::format_to(out, " {}", type);
        if (!type.bases().empty())
            out = type_annotation(out, type.bases());
    }
    if (wrote_header)
    {
        fmt::format_to(out, ")");
        os << '\n' << ygg::print_indent << fmt::to_string(text);
    }
}

/// A section that lists its elements one per line, one level deeper (:predicates, :functions).
template<typename Range>
void write_lines(std::ostream& os, std::string_view section, const Range& range)
{
    if (range.empty())
        return;
    os << '\n' << ygg::print_indent << '(' << section;
    {
        ygg::IndentScope lines(os);
        for (auto element : range)
            os << '\n' << ygg::print_indent << fmt::format("{}", element);
    }
    os << ')';
}

/// A section that lists the requirements (:requirements).
template<typename Range>
void write_requirements(std::ostream& os, const Range& requirements)
{
    if (requirements.empty())
        return;
    auto text = fmt::memory_buffer {};
    fmt::format_to(spaced(fmt::format_to(std::back_inserter(text), "(:requirements"), requirements), ")");
    os << '\n' << ygg::print_indent << fmt::to_string(text);
}

}  // namespace
}  // namespace loki::formalism

using Out = fmt::format_context::iterator;

Out fmt::formatter<lf::RequirementView>::format(const lf::RequirementView& value, fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "{}", value.kind()); }

Out fmt::formatter<lf::TypeView>::format(const lf::TypeView& value, fmt::format_context& ctx) const { return fmt::format_to(ctx.out(), "{}", value.name()); }

Out fmt::formatter<lf::ObjectView>::format(const lf::ObjectView& value, fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "{}", value.name()); }

Out fmt::formatter<lf::VariableView>::format(const lf::VariableView& value, fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "{}", value.name()); }

Out fmt::formatter<lf::ParameterView>::format(const lf::ParameterView& value, fmt::format_context& ctx) const
{
    auto out = fmt::format_to(ctx.out(), "{}", value.variable());
    return value.types().empty() ? out : lf::type_annotation(out, value.types());
}

Out fmt::formatter<lf::PredicateView>::format(const lf::PredicateView& value, fmt::format_context& ctx) const
{ return fmt::format_to(lf::spaced(fmt::format_to(ctx.out(), "({}", value.name()), value.parameters()), ")"); }

Out fmt::formatter<lf::FunctionSkeletonView>::format(const lf::FunctionSkeletonView& value, fmt::format_context& ctx) const
{
    auto out = fmt::format_to(lf::spaced(fmt::format_to(ctx.out(), "({}", value.name()), value.parameters()), ")");
    // number is the default function type; writing it out would demand :typing of a domain without types.
    if (std::string_view(value.type().name()) == "number")
        return out;
    return fmt::format_to(out, " - {}", value.type());
}

Out fmt::formatter<lf::AtomView>::format(const lf::AtomView& value, fmt::format_context& ctx) const
{ return fmt::format_to(lf::spaced(fmt::format_to(ctx.out(), "({}", value.predicate().name()), value.terms()), ")"); }

Out fmt::formatter<lf::LiteralView>::format(const lf::LiteralView& value, fmt::format_context& ctx) const
{
    if (value.polarity())
        return fmt::format_to(ctx.out(), "{}", value.atom());
    return fmt::format_to(ctx.out(), "(not {})", value.atom());
}

Out fmt::formatter<lf::FunctionTermView>::format(const lf::FunctionTermView& value, fmt::format_context& ctx) const
{ return fmt::format_to(lf::spaced(fmt::format_to(ctx.out(), "({}", value.function().name()), value.terms()), ")"); }

Out fmt::formatter<lf::EntityView<lf::FunctionExpression<lf::NumberTag>>>::format(const lf::EntityView<lf::FunctionExpression<lf::NumberTag>>& value,
                                                                                   fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "{}", value.value()); }

Out fmt::formatter<lf::EntityView<lf::FunctionExpression<lf::UnaryTag>>>::format(const lf::EntityView<lf::FunctionExpression<lf::UnaryTag>>& value,
                                                                                  fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "(- {})", value.expression()); }

Out fmt::formatter<lf::EntityView<lf::FunctionExpression<lf::BinaryTag>>>::format(const lf::EntityView<lf::FunctionExpression<lf::BinaryTag>>& value,
                                                                                   fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "({} {} {})", value.operation(), value.left(), value.right()); }

Out fmt::formatter<lf::EntityView<lf::FunctionExpression<lf::MultiTag>>>::format(const lf::EntityView<lf::FunctionExpression<lf::MultiTag>>& value,
                                                                                  fmt::format_context& ctx) const
{ return fmt::format_to(lf::spaced(fmt::format_to(ctx.out(), "({}", value.operation()), value.args()), ")"); }

Out fmt::formatter<lf::EntityView<lf::FunctionExpression<lf::DurationTag>>>::format(const lf::EntityView<lf::FunctionExpression<lf::DurationTag>>&,
                                                                                     fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "?duration"); }

Out fmt::formatter<lf::EntityView<lf::FunctionExpression<lf::IsViolatedTag>>>::format(const lf::EntityView<lf::FunctionExpression<lf::IsViolatedTag>>& value,
                                                                                       fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "(is-violated {})", value.name()); }

Out fmt::formatter<lf::EntityView<lf::Condition<lf::LiteralTag>>>::format(const lf::EntityView<lf::Condition<lf::LiteralTag>>& value,
                                                                           fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "{}", value.literal()); }

Out fmt::formatter<lf::EntityView<lf::Condition<lf::AndTag>>>::format(const lf::EntityView<lf::Condition<lf::AndTag>>& value, fmt::format_context& ctx) const
{ return fmt::format_to(lf::spaced(fmt::format_to(ctx.out(), "(and"), value.conditions()), ")"); }

Out fmt::formatter<lf::EntityView<lf::Condition<lf::OrTag>>>::format(const lf::EntityView<lf::Condition<lf::OrTag>>& value, fmt::format_context& ctx) const
{ return fmt::format_to(lf::spaced(fmt::format_to(ctx.out(), "(or"), value.conditions()), ")"); }

Out fmt::formatter<lf::EntityView<lf::Condition<lf::NotTag>>>::format(const lf::EntityView<lf::Condition<lf::NotTag>>& value, fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "(not {})", value.condition()); }

Out fmt::formatter<lf::EntityView<lf::Condition<lf::ImplyTag>>>::format(const lf::EntityView<lf::Condition<lf::ImplyTag>>& value,
                                                                        fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "(imply {} {})", value.left(), value.right()); }

Out fmt::formatter<lf::EntityView<lf::Condition<lf::ExistsTag>>>::format(const lf::EntityView<lf::Condition<lf::ExistsTag>>& value,
                                                                         fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "(exists ({}) {})", fmt::join(value.parameters(), " "), value.condition()); }

Out fmt::formatter<lf::EntityView<lf::Condition<lf::ForallTag>>>::format(const lf::EntityView<lf::Condition<lf::ForallTag>>& value,
                                                                         fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "(forall ({}) {})", fmt::join(value.parameters(), " "), value.condition()); }

Out fmt::formatter<lf::EntityView<lf::Condition<lf::NumericConstraintTag>>>::format(const lf::EntityView<lf::Condition<lf::NumericConstraintTag>>& value,
                                                                                    fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "({} {} {})", value.comparator(), value.left(), value.right()); }

Out fmt::formatter<lf::EntityView<lf::Condition<lf::TimedTag>>>::format(const lf::EntityView<lf::Condition<lf::TimedTag>>& value,
                                                                        fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "({} {})", value.time(), value.condition()); }

Out fmt::formatter<lf::EntityView<lf::Condition<lf::PreferenceTag>>>::format(const lf::EntityView<lf::Condition<lf::PreferenceTag>>& value,
                                                                             fmt::format_context& ctx) const
{
    if (value.name().empty())
        return fmt::format_to(ctx.out(), "(preference {})", value.condition());
    return fmt::format_to(ctx.out(), "(preference {} {})", value.name(), value.condition());
}

Out fmt::formatter<lf::EntityView<lf::Effect<lf::LiteralTag>>>::format(const lf::EntityView<lf::Effect<lf::LiteralTag>>& value, fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "{}", value.literal()); }

Out fmt::formatter<lf::EntityView<lf::Effect<lf::AndTag>>>::format(const lf::EntityView<lf::Effect<lf::AndTag>>& value, fmt::format_context& ctx) const
{ return fmt::format_to(lf::spaced(fmt::format_to(ctx.out(), "(and"), value.effects()), ")"); }

Out fmt::formatter<lf::EntityView<lf::Effect<lf::NumericTag>>>::format(const lf::EntityView<lf::Effect<lf::NumericTag>>& value, fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "({} {} {})", value.operation(), value.function(), value.expression()); }

Out fmt::formatter<lf::EntityView<lf::Effect<lf::ForallTag>>>::format(const lf::EntityView<lf::Effect<lf::ForallTag>>& value, fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "(forall ({}) {})", fmt::join(value.parameters(), " "), value.effect()); }

Out fmt::formatter<lf::EntityView<lf::Effect<lf::WhenTag>>>::format(const lf::EntityView<lf::Effect<lf::WhenTag>>& value, fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "(when {} {})", value.condition(), value.effect()); }

Out fmt::formatter<lf::EntityView<lf::Effect<lf::OneOfTag>>>::format(const lf::EntityView<lf::Effect<lf::OneOfTag>>& value, fmt::format_context& ctx) const
{ return fmt::format_to(lf::spaced(fmt::format_to(ctx.out(), "(oneof"), value.effects()), ")"); }

Out fmt::formatter<lf::EffectProbabilisticAlternativeView>::format(const lf::EffectProbabilisticAlternativeView& value, fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "{} {}", value.probability(), value.effect()); }

Out fmt::formatter<lf::EntityView<lf::Effect<lf::ProbabilisticTag>>>::format(const lf::EntityView<lf::Effect<lf::ProbabilisticTag>>& value,
                                                                             fmt::format_context& ctx) const
{ return fmt::format_to(lf::spaced(fmt::format_to(ctx.out(), "(probabilistic"), value.alternatives()), ")"); }

Out fmt::formatter<lf::EntityView<lf::Effect<lf::TimedTag>>>::format(const lf::EntityView<lf::Effect<lf::TimedTag>>& value, fmt::format_context& ctx) const
{ return lf::write_timed_effect(ctx.out(), value.time(), value.effect()); }

Out fmt::formatter<lf::EntityView<lf::Constraint<lf::StateTag>>>::format(const lf::EntityView<lf::Constraint<lf::StateTag>>& value,
                                                                         fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "{}", value.condition()); }

Out fmt::formatter<lf::EntityView<lf::Constraint<lf::AndTag>>>::format(const lf::EntityView<lf::Constraint<lf::AndTag>>& value, fmt::format_context& ctx) const
{ return fmt::format_to(lf::spaced(fmt::format_to(ctx.out(), "(and"), value.constraints()), ")"); }

Out fmt::formatter<lf::EntityView<lf::Constraint<lf::OrTag>>>::format(const lf::EntityView<lf::Constraint<lf::OrTag>>& value, fmt::format_context& ctx) const
{ return fmt::format_to(lf::spaced(fmt::format_to(ctx.out(), "(or"), value.constraints()), ")"); }

Out fmt::formatter<lf::EntityView<lf::Constraint<lf::NotTag>>>::format(const lf::EntityView<lf::Constraint<lf::NotTag>>& value, fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "(not {})", value.constraint()); }

Out fmt::formatter<lf::EntityView<lf::Constraint<lf::ExistsTag>>>::format(const lf::EntityView<lf::Constraint<lf::ExistsTag>>& value,
                                                                          fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "(exists ({}) {})", fmt::join(value.parameters(), " "), value.constraint()); }

Out fmt::formatter<lf::EntityView<lf::Constraint<lf::ForallTag>>>::format(const lf::EntityView<lf::Constraint<lf::ForallTag>>& value,
                                                                          fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "(forall ({}) {})", fmt::join(value.parameters(), " "), value.constraint()); }

Out fmt::formatter<lf::EntityView<lf::Constraint<lf::PreferenceTag>>>::format(const lf::EntityView<lf::Constraint<lf::PreferenceTag>>& value,
                                                                              fmt::format_context& ctx) const
{
    if (value.name().empty())
        return fmt::format_to(ctx.out(), "(preference {})", value.constraint());
    return fmt::format_to(ctx.out(), "(preference {} {})", value.name(), value.constraint());
}

Out fmt::formatter<lf::EntityView<lf::Constraint<lf::ModalTag>>>::format(const lf::EntityView<lf::Constraint<lf::ModalTag>>& value,
                                                                         fmt::format_context& ctx) const
{
    auto out = fmt::format_to(ctx.out(), "({}", value.operation());
    const auto numbers = value.numbers();
    // PDDL 3's time bounds stand before the operands, a metric operator's interval as (interval lo hi).
    if (lf::is_trajectory_operator(value.operation()) && !numbers.empty())
        out = fmt::format_to(out, " (interval {} {})", numbers[0], numbers[1]);
    else
        for (const auto number : numbers)
            out = fmt::format_to(out, " {}", number);
    return fmt::format_to(lf::spaced(out, value.constraints()), ")");
}

Out fmt::formatter<lf::ActionView>::format(const lf::ActionView& value, fmt::format_context& ctx) const
{
    auto out = fmt::format_to(ctx.out(), "(:action {} :parameters ({})", value.name(), fmt::join(value.parameters(), " "));
    if (value.precondition())
        out = fmt::format_to(out, " :precondition {}", value.precondition().value());
    if (value.effect())
        out = fmt::format_to(out, " :effect {}", value.effect().value());
    return fmt::format_to(out, ")");
}

Out fmt::formatter<lf::DurativeActionView>::format(const lf::DurativeActionView& value, fmt::format_context& ctx) const
{
    auto out = fmt::format_to(ctx.out(), "(:durative-action {} :parameters ({}) :duration ", value.name(), fmt::join(value.parameters(), " "));
    out = value.duration() ? fmt::format_to(out, "{}", value.duration().value()) : fmt::format_to(out, "()");
    if (value.condition())
        out = fmt::format_to(out, " :condition {}", value.condition().value());
    if (value.effect())
        out = fmt::format_to(out, " :effect {}", value.effect().value());
    return fmt::format_to(out, ")");
}

Out fmt::formatter<lf::AxiomView>::format(const lf::AxiomView& value, fmt::format_context& ctx) const
{
    auto head_variables = urd::Store<urd::set<lf::VariableView>> {};
    for (auto term : value.head().atom().terms())
        visit(
            [&]<typename T>(const T& node)
            {
                if constexpr (std::same_as<T, lf::VariableView>)
                    head_variables.insert(node);
            },
            term);

    auto existential_parameters = urd::Store<urd::sequence<lf::ParameterView>> {};
    for (auto parameter : value.parameters())
        if (!head_variables.contains(parameter.variable()))
            existential_parameters.push_back(parameter);

    // The head declares the types of its variables, as in (:derived (p ?x - t) ...).
    auto parameters = urd::Store<urd::map<lf::VariableView, lf::ParameterView>> {};
    for (auto parameter : value.parameters())
        parameters.emplace(parameter.variable(), parameter);
    auto out = fmt::format_to(ctx.out(), "(:derived ({}", value.head().atom().predicate().name());
    for (auto term : value.head().atom().terms())
        visit(
            [&]<typename T>(const T& node)
            {
                if constexpr (std::same_as<T, lf::VariableView>)
                    if (const auto it = parameters.find(node); it != parameters.end())
                    {
                        out = fmt::format_to(out, " {}", it->second);
                        return;
                    }
                out = fmt::format_to(out, " {}", node);
            },
            term);
    out = fmt::format_to(out, ") ");
    if (!existential_parameters.empty())
        return fmt::format_to(out, "(exists ({}) {}))", fmt::join(existential_parameters, " "), value.condition());
    return fmt::format_to(out, "{})", value.condition());
}

Out fmt::formatter<lf::MetricView>::format(const lf::MetricView& value, fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "(:metric {} {})", value.optimization_direction(), value.expression()); }

Out fmt::formatter<lf::InitialFunctionValueView>::format(const lf::InitialFunctionValueView& value, fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "(= {} {})", value.function(), value.value()); }

Out fmt::formatter<lf::TimedInitialLiteralView>::format(const lf::TimedInitialLiteralView& value, fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "(at {} {})", value.time(), value.literal()); }

Out fmt::formatter<lf::DomainView>::format(const lf::DomainView& value, fmt::format_context& ctx) const
{
    auto os = std::stringstream {};
    os << fmt::format("(define (domain {})", value.name());
    {
        ygg::IndentScope section(os);
        lf::write_requirements(os, value.requirements());
        lf::write_types(os, value.types(), true);
        if (!value.constants().empty())
            lf::write_objects(os, ":constants", value.constants());
        lf::write_lines(os, ":predicates", value.predicates());
        lf::write_lines(os, ":functions", value.functions());
        for (auto axiom : value.axioms())
            os << '\n' << ygg::print_indent << fmt::format("{}", axiom);
        for (auto action : value.actions())
            os << '\n' << ygg::print_indent << fmt::format("{}", action);
        for (auto action : value.durative_actions())
            os << '\n' << ygg::print_indent << fmt::format("{}", action);
        if (value.constraints())
            os << '\n' << ygg::print_indent << fmt::format("(:constraints {})", value.constraints().value());
    }
    os << "\n)";
    return fmt::format_to(ctx.out(), "{}", os.view());
}

Out fmt::formatter<lf::TaskView>::format(const lf::TaskView& value, fmt::format_context& ctx) const
{
    auto os = std::stringstream {};
    os << fmt::format("(define (problem {})", value.name());
    {
        ygg::IndentScope section(os);
        os << '\n' << ygg::print_indent << fmt::format("(:domain {})", value.domain().name());
        lf::write_requirements(os, value.requirements());
        lf::write_types(os, value.types(), false);
        lf::write_lines(os, ":predicates", value.predicates());
        if (!value.objects().empty())
            lf::write_objects(os, ":objects", value.objects());
        auto init = fmt::memory_buffer {};
        auto out = fmt::format_to(std::back_inserter(init), "(:init");
        out = lf::spaced(out, value.initial_literals());
        out = lf::spaced(out, value.initial_function_values());
        out = lf::spaced(out, value.timed_initial_literals());
        for (auto effect : value.initial_effects())
            out = lf::write_initial_effect(fmt::format_to(out, " "), lf::EffectView(effect));
        fmt::format_to(out, ")");
        os << '\n' << ygg::print_indent << fmt::to_string(init);
        if (value.goal())
            os << '\n' << ygg::print_indent << fmt::format("(:goal {})", value.goal().value());
        if (value.goal_reward())
            os << '\n' << ygg::print_indent << fmt::format("(:goal-reward {})", value.goal_reward().value());
        if (value.constraints())
            os << '\n' << ygg::print_indent << fmt::format("(:constraints {})", value.constraints().value());
        if (value.metric())
            os << '\n' << ygg::print_indent << fmt::format("{}", value.metric().value());
        for (auto axiom : value.axioms())
            os << '\n' << ygg::print_indent << fmt::format("{}", axiom);
    }
    os << "\n)";
    return fmt::format_to(ctx.out(), "{}", os.view());
}
