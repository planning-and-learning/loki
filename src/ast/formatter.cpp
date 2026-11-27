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

#include "loki/ast/ast.hpp"

#include <boost/variant/apply_visitor.hpp>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <sstream>
#include <string_view>
#include <yggdrasil/io/iostream.hpp>

namespace fmt
{

format_context::iterator formatter<loki::ast::Identifier, char>::format(const loki::ast::Identifier& value, format_context& ctx) const
{ return fmt::format_to(ctx.out(), fmt::runtime("{}"), value.text); }

format_context::iterator formatter<loki::ast::TypeReference, char>::format(const loki::ast::TypeReference& value, format_context& ctx) const
{ return fmt::format_to(ctx.out(), fmt::runtime("{}"), value.name); }

format_context::iterator formatter<loki::ast::EitherType, char>::format(const loki::ast::EitherType& value, format_context& ctx) const
{
    auto out = fmt::format_to(ctx.out(), fmt::runtime("(either"));
    for (const auto& type : value.alternatives)
        out = fmt::format_to(out, fmt::runtime(" {}"), type.get());
    return fmt::format_to(out, fmt::runtime(")"));
}

format_context::iterator formatter<loki::ast::TypeExpression, char>::format(const loki::ast::TypeExpression& value, format_context& ctx) const
{
    auto out = ctx.out();
    boost::apply_visitor([&](const auto& node) { out = fmt::format_to(out, fmt::runtime("{}"), node); }, value);
    return out;
}

format_context::iterator formatter<loki::ast::TypedGroup, char>::format(const loki::ast::TypedGroup& value, format_context& ctx) const
{
    auto out = ctx.out();
    for (std::size_t i = 0; i < value.names.size(); ++i)
        out = fmt::format_to(out, fmt::runtime("{}{}"), i == 0 ? "" : " ", value.names[i]);
    if (value.type)
        out = fmt::format_to(out, fmt::runtime(" - {}"), *value.type);
    return out;
}

format_context::iterator formatter<loki::ast::Requirement, char>::format(const loki::ast::Requirement& value, format_context& ctx) const
{ return fmt::format_to(ctx.out(), fmt::runtime(":{}"), value.name); }

format_context::iterator formatter<loki::ast::Term, char>::format(const loki::ast::Term& value, format_context& ctx) const
{ return fmt::format_to(ctx.out(), fmt::runtime("{}"), value.name); }

format_context::iterator formatter<loki::ast::Atom, char>::format(const loki::ast::Atom& value, format_context& ctx) const
{
    auto out = fmt::format_to(ctx.out(), fmt::runtime("({}"), value.predicate);
    for (const auto& term : value.terms)
        out = fmt::format_to(out, fmt::runtime(" {}"), term);
    return fmt::format_to(out, fmt::runtime(")"));
}

format_context::iterator formatter<loki::ast::Literal, char>::format(const loki::ast::Literal& value, format_context& ctx) const
{
    if (value.positive)
        return fmt::format_to(ctx.out(), fmt::runtime("{}"), value.atom);
    return fmt::format_to(ctx.out(), fmt::runtime("(not {})"), value.atom);
}

format_context::iterator formatter<loki::ast::FunctionTerm, char>::format(const loki::ast::FunctionTerm& value, format_context& ctx) const
{
    auto out = fmt::format_to(ctx.out(), fmt::runtime("({}"), value.function);
    for (const auto& term : value.terms)
        out = fmt::format_to(out, fmt::runtime(" {}"), term);
    return fmt::format_to(out, fmt::runtime(")"));
}

format_context::iterator formatter<loki::ast::FunctionExpressionNumber, char>::format(const loki::ast::FunctionExpressionNumber& value, format_context& ctx) const
{ return fmt::format_to(ctx.out(), fmt::runtime("{}"), value.value); }

format_context::iterator formatter<loki::ast::FunctionExpressionFunction, char>::format(const loki::ast::FunctionExpressionFunction& value, format_context& ctx) const
{ return fmt::format_to(ctx.out(), fmt::runtime("{}"), value.term); }

format_context::iterator formatter<loki::ast::FunctionExpressionUnary, char>::format(const loki::ast::FunctionExpressionUnary& value, format_context& ctx) const
{ return fmt::format_to(ctx.out(), fmt::runtime("({} {})"), value.op, value.expression.get()); }

format_context::iterator formatter<loki::ast::FunctionExpressionBinary, char>::format(const loki::ast::FunctionExpressionBinary& value, format_context& ctx) const
{ return fmt::format_to(ctx.out(), fmt::runtime("({} {} {})"), value.op, value.left.get(), value.right.get()); }

format_context::iterator formatter<loki::ast::FunctionExpressionMulti, char>::format(const loki::ast::FunctionExpressionMulti& value, format_context& ctx) const
{
    auto out = fmt::format_to(ctx.out(), fmt::runtime("({}"), value.op);
    for (const auto& expression : value.expressions)
        out = fmt::format_to(out, fmt::runtime(" {}"), expression.get());
    return fmt::format_to(out, fmt::runtime(")"));
}

format_context::iterator formatter<loki::ast::FunctionExpressionDuration, char>::format(const loki::ast::FunctionExpressionDuration& value, format_context& ctx) const
{
    (void) value;
    return fmt::format_to(ctx.out(), fmt::runtime("?duration"));
}

format_context::iterator formatter<loki::ast::FunctionExpressionIsViolated, char>::format(const loki::ast::FunctionExpressionIsViolated& value, format_context& ctx) const
{ return fmt::format_to(ctx.out(), fmt::runtime("(is-violated {})"), value.name); }

format_context::iterator formatter<loki::ast::FunctionExpression, char>::format(const loki::ast::FunctionExpression& value, format_context& ctx) const
{
    auto out = ctx.out();
    boost::apply_visitor([&](const auto& node) { out = fmt::format_to(out, fmt::runtime("{}"), node); }, value);
    return out;
}

format_context::iterator formatter<loki::ast::ConditionLiteral, char>::format(const loki::ast::ConditionLiteral& value, format_context& ctx) const
{ return fmt::format_to(ctx.out(), fmt::runtime("{}"), value.literal); }

format_context::iterator formatter<loki::ast::ConditionAnd, char>::format(const loki::ast::ConditionAnd& value, format_context& ctx) const
{
    auto out = fmt::format_to(ctx.out(), fmt::runtime("(and"));
    for (const auto& condition : value.conditions)
        out = fmt::format_to(out, fmt::runtime(" {}"), condition.get());
    return fmt::format_to(out, fmt::runtime(")"));
}

format_context::iterator formatter<loki::ast::ConditionOr, char>::format(const loki::ast::ConditionOr& value, format_context& ctx) const
{
    auto out = fmt::format_to(ctx.out(), fmt::runtime("(or"));
    for (const auto& condition : value.conditions)
        out = fmt::format_to(out, fmt::runtime(" {}"), condition.get());
    return fmt::format_to(out, fmt::runtime(")"));
}

format_context::iterator formatter<loki::ast::ConditionNot, char>::format(const loki::ast::ConditionNot& value, format_context& ctx) const
{ return fmt::format_to(ctx.out(), fmt::runtime("(not {})"), value.condition.get()); }

format_context::iterator formatter<loki::ast::ConditionImply, char>::format(const loki::ast::ConditionImply& value, format_context& ctx) const
{ return fmt::format_to(ctx.out(), fmt::runtime("(imply {} {})"), value.left.get(), value.right.get()); }

format_context::iterator formatter<loki::ast::ConditionExists, char>::format(const loki::ast::ConditionExists& value, format_context& ctx) const
{
    auto out = fmt::format_to(ctx.out(), fmt::runtime("(exists ("));
    auto first = true;
    for (const auto& parameter : value.parameters)
    {
        if (!first)
            out = fmt::format_to(out, fmt::runtime(" "));
        first = false;
        out = fmt::format_to(out, fmt::runtime("{}"), parameter);
    }
    return fmt::format_to(out, fmt::runtime(") {})"), value.condition.get());
}

format_context::iterator formatter<loki::ast::ConditionForall, char>::format(const loki::ast::ConditionForall& value, format_context& ctx) const
{
    auto out = fmt::format_to(ctx.out(), fmt::runtime("(forall ("));
    auto first = true;
    for (const auto& parameter : value.parameters)
    {
        if (!first)
            out = fmt::format_to(out, fmt::runtime(" "));
        first = false;
        out = fmt::format_to(out, fmt::runtime("{}"), parameter);
    }
    return fmt::format_to(out, fmt::runtime(") {})"), value.condition.get());
}

format_context::iterator formatter<loki::ast::ConditionNumericConstraint, char>::format(const loki::ast::ConditionNumericConstraint& value, format_context& ctx) const
{ return fmt::format_to(ctx.out(), fmt::runtime("({} {} {})"), value.comparator, value.left.get(), value.right.get()); }

format_context::iterator formatter<loki::ast::ConditionTimed, char>::format(const loki::ast::ConditionTimed& value, format_context& ctx) const
{
    const auto* time = value.time == 's' ? "at start" : value.time == 'e' ? "at end" : "over all";
    return fmt::format_to(ctx.out(), fmt::runtime("({} {})"), time, value.condition.get());
}

format_context::iterator formatter<loki::ast::ConditionPreference, char>::format(const loki::ast::ConditionPreference& value, format_context& ctx) const
{
    if (value.name)
        return fmt::format_to(ctx.out(), fmt::runtime("(preference {} {})"), *value.name, value.condition.get());
    return fmt::format_to(ctx.out(), fmt::runtime("(preference {})"), value.condition.get());
}

format_context::iterator formatter<loki::ast::Condition, char>::format(const loki::ast::Condition& value, format_context& ctx) const
{
    auto out = ctx.out();
    boost::apply_visitor([&](const auto& node) { out = fmt::format_to(out, fmt::runtime("{}"), node); }, value);
    return out;
}

format_context::iterator formatter<loki::ast::EffectLiteral, char>::format(const loki::ast::EffectLiteral& value, format_context& ctx) const
{ return fmt::format_to(ctx.out(), fmt::runtime("{}"), value.literal); }

format_context::iterator formatter<loki::ast::EffectAnd, char>::format(const loki::ast::EffectAnd& value, format_context& ctx) const
{
    auto out = fmt::format_to(ctx.out(), fmt::runtime("(and"));
    for (const auto& effect : value.effects)
        out = fmt::format_to(out, fmt::runtime(" {}"), effect.get());
    return fmt::format_to(out, fmt::runtime(")"));
}

format_context::iterator formatter<loki::ast::EffectNumeric, char>::format(const loki::ast::EffectNumeric& value, format_context& ctx) const
{ return fmt::format_to(ctx.out(), fmt::runtime("({} {} {})"), value.op, value.function, value.expression.get()); }

format_context::iterator formatter<loki::ast::EffectForall, char>::format(const loki::ast::EffectForall& value, format_context& ctx) const
{
    auto out = fmt::format_to(ctx.out(), fmt::runtime("(forall ("));
    auto first = true;
    for (const auto& parameter : value.parameters)
    {
        if (!first)
            out = fmt::format_to(out, fmt::runtime(" "));
        first = false;
        out = fmt::format_to(out, fmt::runtime("{}"), parameter);
    }
    return fmt::format_to(out, fmt::runtime(") {})"), value.effect.get());
}

format_context::iterator formatter<loki::ast::EffectWhen, char>::format(const loki::ast::EffectWhen& value, format_context& ctx) const
{ return fmt::format_to(ctx.out(), fmt::runtime("(when {} {})"), value.condition.get(), value.effect.get()); }

format_context::iterator formatter<loki::ast::EffectOneOf, char>::format(const loki::ast::EffectOneOf& value, format_context& ctx) const
{
    auto out = fmt::format_to(ctx.out(), fmt::runtime("(oneof"));
    for (const auto& effect : value.effects)
        out = fmt::format_to(out, fmt::runtime(" {}"), effect.get());
    return fmt::format_to(out, fmt::runtime(")"));
}

format_context::iterator formatter<loki::ast::ProbabilisticEffectAlternative, char>::format(const loki::ast::ProbabilisticEffectAlternative& value, format_context& ctx) const
{
    if (value.denominator)
        return fmt::format_to(ctx.out(), fmt::runtime("{}/{} {}"), value.probability, *value.denominator, value.effect.get());
    return fmt::format_to(ctx.out(), fmt::runtime("{} {}"), value.probability, value.effect.get());
}

format_context::iterator formatter<loki::ast::EffectProbabilistic, char>::format(const loki::ast::EffectProbabilistic& value, format_context& ctx) const
{
    auto out = fmt::format_to(ctx.out(), fmt::runtime("(probabilistic"));
    for (const auto& alternative : value.alternatives)
        out = fmt::format_to(out, fmt::runtime(" {}"), alternative);
    return fmt::format_to(out, fmt::runtime(")"));
}

format_context::iterator formatter<loki::ast::EffectTimed, char>::format(const loki::ast::EffectTimed& value, format_context& ctx) const
{
    const auto* time = value.time == 's' ? "at start" : value.time == 'e' ? "at end" : "over all";
    return fmt::format_to(ctx.out(), fmt::runtime("({} {})"), time, value.effect.get());
}

format_context::iterator formatter<loki::ast::Effect, char>::format(const loki::ast::Effect& value, format_context& ctx) const
{
    auto out = ctx.out();
    boost::apply_visitor([&](const auto& node) { out = fmt::format_to(out, fmt::runtime("{}"), node); }, value);
    return out;
}

format_context::iterator formatter<loki::ast::PredicateDeclaration, char>::format(const loki::ast::PredicateDeclaration& value, format_context& ctx) const
{
    auto out = fmt::format_to(ctx.out(), fmt::runtime("({}"), value.name);
    for (const auto& parameter : value.parameters)
        out = fmt::format_to(out, fmt::runtime(" {}"), parameter);
    return fmt::format_to(out, fmt::runtime(")"));
}

format_context::iterator formatter<loki::ast::FunctionDeclaration, char>::format(const loki::ast::FunctionDeclaration& value, format_context& ctx) const
{
    auto out = fmt::format_to(ctx.out(), fmt::runtime("({}"), value.name);
    for (const auto& parameter : value.parameters)
        out = fmt::format_to(out, fmt::runtime(" {}"), parameter);
    out = fmt::format_to(out, fmt::runtime(")"));
    if (value.type)
        out = fmt::format_to(out, fmt::runtime(" - {}"), *value.type);
    return out;
}

format_context::iterator formatter<loki::ast::Action, char>::format(const loki::ast::Action& value, format_context& ctx) const
{
    auto out = fmt::format_to(ctx.out(), fmt::runtime("(:action {} :parameters ("), value.name);
    auto first = true;
    for (const auto& parameter : value.parameters)
    {
        if (!first)
            out = fmt::format_to(out, fmt::runtime(" "));
        first = false;
        out = fmt::format_to(out, fmt::runtime("{}"), parameter);
    }
    out = fmt::format_to(out, fmt::runtime(")"));
    if (value.precondition)
        out = fmt::format_to(out, fmt::runtime(" :precondition {}"), *value.precondition);
    if (value.effect)
        out = fmt::format_to(out, fmt::runtime(" :effect {}"), *value.effect);
    return fmt::format_to(out, fmt::runtime(")"));
}

format_context::iterator formatter<loki::ast::ConstraintAnd, char>::format(const loki::ast::ConstraintAnd& value, format_context& ctx) const
{
    auto out = fmt::format_to(ctx.out(), fmt::runtime("(and"));
    for (const auto& constraint : value.constraints)
        out = fmt::format_to(out, fmt::runtime(" {}"), constraint.get());
    return fmt::format_to(out, fmt::runtime(")"));
}

format_context::iterator formatter<loki::ast::ConstraintOr, char>::format(const loki::ast::ConstraintOr& value, format_context& ctx) const
{
    auto out = fmt::format_to(ctx.out(), fmt::runtime("(or"));
    for (const auto& constraint : value.constraints)
        out = fmt::format_to(out, fmt::runtime(" {}"), constraint.get());
    return fmt::format_to(out, fmt::runtime(")"));
}

format_context::iterator formatter<loki::ast::ConstraintNot, char>::format(const loki::ast::ConstraintNot& value, format_context& ctx) const
{ return fmt::format_to(ctx.out(), fmt::runtime("(not {})"), value.constraint.get()); }

format_context::iterator formatter<loki::ast::ConstraintImply, char>::format(const loki::ast::ConstraintImply& value, format_context& ctx) const
{ return fmt::format_to(ctx.out(), fmt::runtime("(imply {} {})"), value.left.get(), value.right.get()); }

format_context::iterator formatter<loki::ast::ConstraintExists, char>::format(const loki::ast::ConstraintExists& value, format_context& ctx) const
{
    auto out = fmt::format_to(ctx.out(), fmt::runtime("(exists ("));
    auto first = true;
    for (const auto& parameter : value.parameters)
    {
        if (!first)
            out = fmt::format_to(out, fmt::runtime(" "));
        first = false;
        out = fmt::format_to(out, fmt::runtime("{}"), parameter);
    }
    return fmt::format_to(out, fmt::runtime(") {})"), value.constraint.get());
}

format_context::iterator formatter<loki::ast::ConstraintState, char>::format(const loki::ast::ConstraintState& value, format_context& ctx) const
{ return fmt::format_to(ctx.out(), fmt::runtime("{}"), value.condition); }

format_context::iterator formatter<loki::ast::ConstraintForall, char>::format(const loki::ast::ConstraintForall& value, format_context& ctx) const
{
    auto out = fmt::format_to(ctx.out(), fmt::runtime("(forall ("));
    auto first = true;
    for (const auto& parameter : value.parameters)
    {
        if (!first)
            out = fmt::format_to(out, fmt::runtime(" "));
        first = false;
        out = fmt::format_to(out, fmt::runtime("{}"), parameter);
    }
    return fmt::format_to(out, fmt::runtime(") {})"), value.constraint.get());
}

format_context::iterator formatter<loki::ast::ConstraintPreference, char>::format(const loki::ast::ConstraintPreference& value, format_context& ctx) const
{
    if (value.name)
        return fmt::format_to(ctx.out(), fmt::runtime("(preference {} {})"), *value.name, value.constraint.get());
    return fmt::format_to(ctx.out(), fmt::runtime("(preference {})"), value.constraint.get());
}

format_context::iterator formatter<loki::ast::ConstraintModal, char>::format(const loki::ast::ConstraintModal& value, format_context& ctx) const
{
    const auto* operation = value.operation == 'E' ? "at end" :
                            value.operation == 'A' ? "always" :
                            value.operation == 'S' ? "sometime" :
                            value.operation == 'W' ? "within" :
                            value.operation == 'O' ? "at-most-once" :
                            value.operation == 'a' ? "sometime-after" :
                            value.operation == 'b' ? "sometime-before" :
                            value.operation == 'w' ? "always-within" :
                            value.operation == 'h' ? "hold-during" :
                            value.operation == 'H' ? "hold-after" :
                            value.operation == 'n' ? "next" :
                            value.operation == 'N' ? "weak-next" :
                            value.operation == 'u' ? "until" :
                            value.operation == 'r' ? "release" :
                            value.operation == 'y' ? "yesterday" :
                            value.operation == 'Y' ? "weak-yesterday" :
                            value.operation == 's' ? "since" :
                            value.operation == 't' ? "triggers" :
                            value.operation == 'o' ? "once" :
                                                     "historically";
    // :trajectory-formulas' operators write their interval as (interval lo hi), PDDL 3's their bounds bare.
    const auto trajectory = std::string_view("nNuryYstoi").find(value.operation) != std::string_view::npos;
    auto out = fmt::format_to(ctx.out(), fmt::runtime("({}"), operation);
    if (trajectory && value.numbers.size() == 2)
        out = fmt::format_to(out, fmt::runtime(" (interval {} {})"), value.numbers[0], value.numbers[1]);
    else
        for (const auto number : value.numbers)
            out = fmt::format_to(out, fmt::runtime(" {}"), number);
    for (const auto& constraint : value.constraints)
        out = fmt::format_to(out, fmt::runtime(" {}"), constraint.get());
    return fmt::format_to(out, fmt::runtime(")"));
}

format_context::iterator formatter<loki::ast::Constraint, char>::format(const loki::ast::Constraint& value, format_context& ctx) const
{
    auto out = ctx.out();
    boost::apply_visitor([&](const auto& node) { out = fmt::format_to(out, fmt::runtime("{}"), node); }, value);
    return out;
}

format_context::iterator formatter<loki::ast::ConstraintSection, char>::format(const loki::ast::ConstraintSection& value, format_context& ctx) const
{ return fmt::format_to(ctx.out(), fmt::runtime("(:constraints {})"), value.constraint); }

format_context::iterator formatter<loki::ast::DurativeAction, char>::format(const loki::ast::DurativeAction& value, format_context& ctx) const
{
    auto out = fmt::format_to(ctx.out(), fmt::runtime("(:durative-action {} :parameters ("), value.name);
    auto first = true;
    for (const auto& parameter : value.parameters)
    {
        if (!first)
            out = fmt::format_to(out, fmt::runtime(" "));
        first = false;
        out = fmt::format_to(out, fmt::runtime("{}"), parameter);
    }
    out = fmt::format_to(out, fmt::runtime(") :duration {}"), value.duration);
    if (value.condition)
        out = fmt::format_to(out, fmt::runtime(" :condition {}"), *value.condition);
    if (value.effect)
        out = fmt::format_to(out, fmt::runtime(" :effect {}"), *value.effect);
    return fmt::format_to(out, fmt::runtime(")"));
}

format_context::iterator formatter<loki::ast::Axiom, char>::format(const loki::ast::Axiom& value, format_context& ctx) const
{ return fmt::format_to(ctx.out(), fmt::runtime("(:derived {} {})"), value.head, value.condition); }

format_context::iterator formatter<loki::ast::Metric, char>::format(const loki::ast::Metric& value, format_context& ctx) const
{ return fmt::format_to(ctx.out(), fmt::runtime("(:metric {} {})"), value.optimization, value.expression); }

format_context::iterator formatter<loki::ast::InitialFunctionValue, char>::format(const loki::ast::InitialFunctionValue& value, format_context& ctx) const
{ return fmt::format_to(ctx.out(), fmt::runtime("(= {} {})"), value.function, value.value); }

format_context::iterator formatter<loki::ast::TimedInitialLiteral, char>::format(const loki::ast::TimedInitialLiteral& value, format_context& ctx) const
{ return fmt::format_to(ctx.out(), fmt::runtime("(at {} {})"), value.time, value.literal); }

format_context::iterator formatter<loki::ast::InitialAtom, char>::format(const loki::ast::InitialAtom& value, format_context& ctx) const
{
    auto out = ctx.out();
    boost::apply_visitor([&](const auto& node) { out = fmt::format_to(out, fmt::runtime("{}"), node); }, value);
    return out;
}

format_context::iterator formatter<loki::ast::ProbabilisticInitialAlternative, char>::format(const loki::ast::ProbabilisticInitialAlternative& value, format_context& ctx) const
{
    auto out = ctx.out();
    if (value.denominator)
        out = fmt::format_to(out, fmt::runtime("{}/{} "), value.probability, *value.denominator);
    else
        out = fmt::format_to(out, fmt::runtime("{} "), value.probability);
    if (value.atoms.size() == 1)
        return fmt::format_to(out, fmt::runtime("{}"), value.atoms.front());
    out = fmt::format_to(out, fmt::runtime("(and"));
    for (const auto& atom : value.atoms)
        out = fmt::format_to(out, fmt::runtime(" {}"), atom);
    return fmt::format_to(out, fmt::runtime(")"));
}

format_context::iterator formatter<loki::ast::InitialProbabilistic, char>::format(const loki::ast::InitialProbabilistic& value, format_context& ctx) const
{
    auto out = fmt::format_to(ctx.out(), fmt::runtime("(probabilistic"));
    for (const auto& alternative : value.alternatives)
        out = fmt::format_to(out, fmt::runtime(" {}"), alternative);
    return fmt::format_to(out, fmt::runtime(")"));
}

format_context::iterator formatter<loki::ast::InitialElement, char>::format(const loki::ast::InitialElement& value, format_context& ctx) const
{
    auto out = ctx.out();
    boost::apply_visitor([&](const auto& node) { out = fmt::format_to(out, fmt::runtime("{}"), node); }, value);
    return out;
}

format_context::iterator formatter<loki::ast::Domain, char>::format(const loki::ast::Domain& value, format_context& ctx) const
{
    auto os = std::stringstream {};
    os << "(define (domain " << value.name.text << ')';
    {
        ygg::IndentScope section(os);
        const auto print_list = [&](const char* header, const auto& elements)
        {
            os << '\n' << ygg::print_indent << header;
            auto first = true;
            for (const auto& element : elements)
            {
                if (!first)
                    os << ' ';
                first = false;
                fmt::print(os, fmt::runtime("{}"), element);
            }
            os << ')';
        };
        const auto print_lines = [&](const char* header, const auto& elements)
        {
            os << '\n' << ygg::print_indent << header;
            {
                ygg::IndentScope lines(os);
                for (const auto& element : elements)
                {
                    os << '\n' << ygg::print_indent;
                    fmt::print(os, fmt::runtime("{}"), element);
                }
            }
            os << ')';
        };
        // In source order: a domain's sections can come in any order.
        for (const auto& element : value.elements)
        {
            boost::apply_visitor(
                [&]<typename Node>(const Node& node)
                {
                    if constexpr (std::is_same_v<Node, loki::ast::RequirementSection>)
                    {
                        os << '\n' << ygg::print_indent << "(:requirements";
                        for (const auto& requirement : node.requirements)
                            fmt::print(os, fmt::runtime(" {}"), requirement);
                        os << ')';
                    }
                    else if constexpr (std::is_same_v<Node, loki::ast::TypeSection>)
                        print_list("(:types ", node.types);
                    else if constexpr (std::is_same_v<Node, loki::ast::ConstantSection>)
                        print_list("(:constants ", node.constants);
                    else if constexpr (std::is_same_v<Node, loki::ast::PredicateSection>)
                        print_lines("(:predicates", node.predicates);
                    else if constexpr (std::is_same_v<Node, loki::ast::FunctionSection>)
                        print_lines("(:functions", node.functions);
                    else
                    {
                        os << '\n' << ygg::print_indent;
                        fmt::print(os, fmt::runtime("{}"), node);
                    }
                },
                element);
        }
    }
    os << "\n)";
    return fmt::format_to(ctx.out(), fmt::runtime("{}"), os.view());
}

format_context::iterator formatter<loki::ast::Task, char>::format(const loki::ast::Task& value, format_context& ctx) const
{
    auto os = std::stringstream {};
    os << "(define (problem " << value.name.text << ')';
    {
        ygg::IndentScope section(os);
        os << '\n' << ygg::print_indent << "(:domain " << value.domain_name.text << ')';
        if (!value.requirements.empty())
        {
            os << '\n' << ygg::print_indent << "(:requirements";
            for (const auto& requirement : value.requirements)
                fmt::print(os, fmt::runtime(" {}"), requirement);
            os << ')';
        }
        if (!value.types.empty())
        {
            os << '\n' << ygg::print_indent << "(:types ";
            auto first = true;
            for (const auto& type : value.types)
            {
                if (!first)
                    os << ' ';
                first = false;
                fmt::print(os, fmt::runtime("{}"), type);
            }
            os << ')';
        }
        if (!value.predicates.empty())
        {
            os << '\n' << ygg::print_indent << "(:predicates";
            {
                ygg::IndentScope predicates(os);
                for (const auto& predicate : value.predicates)
                {
                    os << '\n' << ygg::print_indent;
                    fmt::print(os, fmt::runtime("{}"), predicate);
                }
            }
            os << ')';
        }
        if (!value.objects.empty())
        {
            os << '\n' << ygg::print_indent << "(:objects ";
            auto first = true;
            for (const auto& object : value.objects)
            {
                if (!first)
                    os << ' ';
                first = false;
                fmt::print(os, fmt::runtime("{}"), object);
            }
            os << ')';
        }
        os << '\n' << ygg::print_indent << "(:init";
        for (const auto& initial : value.initial)
            fmt::print(os, fmt::runtime(" {}"), initial);
        os << ')';
        if (value.goal)
        {
            os << '\n' << ygg::print_indent;
            fmt::print(os, fmt::runtime("(:goal {})"), *value.goal);
        }
        if (value.goal_reward)
        {
            os << '\n' << ygg::print_indent;
            fmt::print(os, fmt::runtime("(:goal-reward {})"), *value.goal_reward);
        }
        if (value.constraints)
        {
            os << '\n' << ygg::print_indent;
            fmt::print(os, fmt::runtime("(:constraints {})"), *value.constraints);
        }
        if (value.metric)
        {
            os << '\n' << ygg::print_indent;
            fmt::print(os, fmt::runtime("{}"), *value.metric);
        }
        for (const auto& axiom : value.axioms)
        {
            os << '\n' << ygg::print_indent;
            fmt::print(os, fmt::runtime("{}"), axiom);
        }
    }
    os << "\n)";
    return fmt::format_to(ctx.out(), fmt::runtime("{}"), os.view());
}

format_context::iterator formatter<loki::ast::File, char>::format(const loki::ast::File& value, format_context& ctx) const
{
    auto out = ctx.out();
    boost::apply_visitor([&](const auto& node) { out = fmt::format_to(out, fmt::runtime("{}"), node); }, value);
    return out;
}

}  // namespace fmt
