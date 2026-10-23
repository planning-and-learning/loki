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

#ifndef LOKI_AST_FORMATTER_HPP_
#define LOKI_AST_FORMATTER_HPP_

#include "loki/ast/ast.hpp"

#include <boost/variant/apply_visitor.hpp>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <sstream>
#include <yggdrasil/io/iostream.hpp>

namespace fmt
{

template<>
struct formatter<loki::ast::Identifier, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::Identifier& value, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), fmt::runtime("{}"), value.text);
    }
};

template<>
struct formatter<loki::ast::TypeReference, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::TypeReference& value, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), fmt::runtime("{}"), value.name);
    }
};

template<>
struct formatter<loki::ast::EitherType, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::EitherType& value, FormatContext& ctx) const
    {
        auto out = fmt::format_to(ctx.out(), fmt::runtime("(either"));
        for (const auto& type : value.alternatives)
            out = fmt::format_to(out, fmt::runtime(" {}"), type.get());
        return fmt::format_to(out, fmt::runtime(")"));
    }
};

template<>
struct formatter<loki::ast::TypeExpression, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::TypeExpression& value, FormatContext& ctx) const
    {
        auto out = ctx.out();
        boost::apply_visitor([&](const auto& node) { out = fmt::format_to(out, fmt::runtime("{}"), node); }, value);
        return out;
    }
};

template<>
struct formatter<loki::ast::TypedGroup, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::TypedGroup& value, FormatContext& ctx) const
    {
        auto out = ctx.out();
        for (std::size_t i = 0; i < value.names.size(); ++i)
            out = fmt::format_to(out, fmt::runtime("{}{}"), i == 0 ? "" : " ", value.names[i]);
        if (value.type)
            out = fmt::format_to(out, fmt::runtime(" - {}"), *value.type);
        return out;
    }
};

template<>
struct formatter<loki::ast::Requirement, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::Requirement& value, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), fmt::runtime(":{}"), value.name);
    }
};

template<>
struct formatter<loki::ast::Term, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::Term& value, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), fmt::runtime("{}"), value.name);
    }
};

template<>
struct formatter<loki::ast::Atom, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::Atom& value, FormatContext& ctx) const
    {
        auto out = fmt::format_to(ctx.out(), fmt::runtime("({}"), value.predicate);
        for (const auto& term : value.terms)
            out = fmt::format_to(out, fmt::runtime(" {}"), term);
        return fmt::format_to(out, fmt::runtime(")"));
    }
};

template<>
struct formatter<loki::ast::Literal, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::Literal& value, FormatContext& ctx) const
    {
        if (value.positive)
            return fmt::format_to(ctx.out(), fmt::runtime("{}"), value.atom);
        return fmt::format_to(ctx.out(), fmt::runtime("(not {})"), value.atom);
    }
};

template<>
struct formatter<loki::ast::FunctionTerm, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::FunctionTerm& value, FormatContext& ctx) const
    {
        auto out = fmt::format_to(ctx.out(), fmt::runtime("({}"), value.function);
        for (const auto& term : value.terms)
            out = fmt::format_to(out, fmt::runtime(" {}"), term);
        return fmt::format_to(out, fmt::runtime(")"));
    }
};

template<>
struct formatter<loki::ast::FunctionExpressionNumber, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::FunctionExpressionNumber& value, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), fmt::runtime("{}"), value.value);
    }
};

template<>
struct formatter<loki::ast::FunctionExpressionFunction, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::FunctionExpressionFunction& value, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), fmt::runtime("{}"), value.term);
    }
};

template<>
struct formatter<loki::ast::FunctionExpressionUnary, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::FunctionExpressionUnary& value, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), fmt::runtime("({} {})"), value.op, value.expression.get());
    }
};

template<>
struct formatter<loki::ast::FunctionExpressionBinary, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::FunctionExpressionBinary& value, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), fmt::runtime("({} {} {})"), value.op, value.left.get(), value.right.get());
    }
};

template<>
struct formatter<loki::ast::FunctionExpressionMulti, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::FunctionExpressionMulti& value, FormatContext& ctx) const
    {
        auto out = fmt::format_to(ctx.out(), fmt::runtime("({}"), value.op);
        for (const auto& expression : value.expressions)
            out = fmt::format_to(out, fmt::runtime(" {}"), expression.get());
        return fmt::format_to(out, fmt::runtime(")"));
    }
};

template<>
struct formatter<loki::ast::FunctionExpression, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::FunctionExpression& value, FormatContext& ctx) const
    {
        auto out = ctx.out();
        boost::apply_visitor([&](const auto& node) { out = fmt::format_to(out, fmt::runtime("{}"), node); }, value);
        return out;
    }
};

template<>
struct formatter<loki::ast::ConditionLiteral, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::ConditionLiteral& value, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), fmt::runtime("{}"), value.literal);
    }
};

template<>
struct formatter<loki::ast::ConditionAnd, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::ConditionAnd& value, FormatContext& ctx) const
    {
        auto out = fmt::format_to(ctx.out(), fmt::runtime("(and"));
        for (const auto& condition : value.conditions)
            out = fmt::format_to(out, fmt::runtime(" {}"), condition.get());
        return fmt::format_to(out, fmt::runtime(")"));
    }
};

template<>
struct formatter<loki::ast::ConditionOr, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::ConditionOr& value, FormatContext& ctx) const
    {
        auto out = fmt::format_to(ctx.out(), fmt::runtime("(or"));
        for (const auto& condition : value.conditions)
            out = fmt::format_to(out, fmt::runtime(" {}"), condition.get());
        return fmt::format_to(out, fmt::runtime(")"));
    }
};

template<>
struct formatter<loki::ast::ConditionNot, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::ConditionNot& value, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), fmt::runtime("(not {})"), value.condition.get());
    }
};

template<>
struct formatter<loki::ast::ConditionImply, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::ConditionImply& value, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), fmt::runtime("(imply {} {})"), value.left.get(), value.right.get());
    }
};

template<>
struct formatter<loki::ast::ConditionExists, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::ConditionExists& value, FormatContext& ctx) const
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
};

template<>
struct formatter<loki::ast::ConditionForall, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::ConditionForall& value, FormatContext& ctx) const
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
};

template<>
struct formatter<loki::ast::ConditionNumericConstraint, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::ConditionNumericConstraint& value, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), fmt::runtime("({} {} {})"), value.comparator, value.left.get(), value.right.get());
    }
};

template<>
struct formatter<loki::ast::Condition, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::Condition& value, FormatContext& ctx) const
    {
        auto out = ctx.out();
        boost::apply_visitor([&](const auto& node) { out = fmt::format_to(out, fmt::runtime("{}"), node); }, value);
        return out;
    }
};

template<>
struct formatter<loki::ast::EffectLiteral, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::EffectLiteral& value, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), fmt::runtime("{}"), value.literal);
    }
};

template<>
struct formatter<loki::ast::EffectAnd, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::EffectAnd& value, FormatContext& ctx) const
    {
        auto out = fmt::format_to(ctx.out(), fmt::runtime("(and"));
        for (const auto& effect : value.effects)
            out = fmt::format_to(out, fmt::runtime(" {}"), effect.get());
        return fmt::format_to(out, fmt::runtime(")"));
    }
};

template<>
struct formatter<loki::ast::EffectNumeric, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::EffectNumeric& value, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), fmt::runtime("({} {} {})"), value.op, value.function, value.expression.get());
    }
};

template<>
struct formatter<loki::ast::EffectForall, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::EffectForall& value, FormatContext& ctx) const
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
};

template<>
struct formatter<loki::ast::EffectWhen, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::EffectWhen& value, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), fmt::runtime("(when {} {})"), value.condition.get(), value.effect.get());
    }
};

template<>
struct formatter<loki::ast::EffectOneOf, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::EffectOneOf& value, FormatContext& ctx) const
    {
        auto out = fmt::format_to(ctx.out(), fmt::runtime("(oneof"));
        for (const auto& effect : value.effects)
            out = fmt::format_to(out, fmt::runtime(" {}"), effect.get());
        return fmt::format_to(out, fmt::runtime(")"));
    }
};

template<>
struct formatter<loki::ast::ProbabilisticEffectAlternative, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::ProbabilisticEffectAlternative& value, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), fmt::runtime("{} {}"), value.probability, value.effect.get());
    }
};

template<>
struct formatter<loki::ast::EffectProbabilistic, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::EffectProbabilistic& value, FormatContext& ctx) const
    {
        auto out = fmt::format_to(ctx.out(), fmt::runtime("(probabilistic"));
        for (const auto& alternative : value.alternatives)
            out = fmt::format_to(out, fmt::runtime(" {}"), alternative);
        return fmt::format_to(out, fmt::runtime(")"));
    }
};

template<>
struct formatter<loki::ast::Effect, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::Effect& value, FormatContext& ctx) const
    {
        auto out = ctx.out();
        boost::apply_visitor([&](const auto& node) { out = fmt::format_to(out, fmt::runtime("{}"), node); }, value);
        return out;
    }
};

template<>
struct formatter<loki::ast::PredicateDeclaration, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::PredicateDeclaration& value, FormatContext& ctx) const
    {
        auto out = fmt::format_to(ctx.out(), fmt::runtime("({}"), value.name);
        for (const auto& parameter : value.parameters)
            out = fmt::format_to(out, fmt::runtime(" {}"), parameter);
        return fmt::format_to(out, fmt::runtime(")"));
    }
};

template<>
struct formatter<loki::ast::FunctionDeclaration, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::FunctionDeclaration& value, FormatContext& ctx) const
    {
        auto out = fmt::format_to(ctx.out(), fmt::runtime("({}"), value.name);
        for (const auto& parameter : value.parameters)
            out = fmt::format_to(out, fmt::runtime(" {}"), parameter);
        out = fmt::format_to(out, fmt::runtime(")"));
        if (value.type)
            out = fmt::format_to(out, fmt::runtime(" - {}"), *value.type);
        return out;
    }
};

template<>
struct formatter<loki::ast::Action, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::Action& value, FormatContext& ctx) const
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
};

template<>
struct formatter<loki::ast::Axiom, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::Axiom& value, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), fmt::runtime("(:derived {} {})"), value.head, value.condition);
    }
};

template<>
struct formatter<loki::ast::Metric, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::Metric& value, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), fmt::runtime("(:metric {} {})"), value.optimization, value.expression);
    }
};

template<>
struct formatter<loki::ast::InitialFunctionValue, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::InitialFunctionValue& value, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), fmt::runtime("(= {} {})"), value.function, value.value);
    }
};

template<>
struct formatter<loki::ast::InitialElement, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::InitialElement& value, FormatContext& ctx) const
    {
        auto out = ctx.out();
        boost::apply_visitor([&](const auto& node) { out = fmt::format_to(out, fmt::runtime("{}"), node); }, value);
        return out;
    }
};

template<>
struct formatter<loki::ast::Domain, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::Domain& value, FormatContext& ctx) const
    {
        auto os = std::stringstream {};
        os << "(define (domain " << value.name.text << ')';
        {
            ygg::IndentScope section(os);
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
            if (!value.constants.empty())
            {
                os << '\n' << ygg::print_indent << "(:constants ";
                auto first = true;
                for (const auto& constant : value.constants)
                {
                    if (!first)
                        os << ' ';
                    first = false;
                    fmt::print(os, fmt::runtime("{}"), constant);
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
            if (!value.functions.empty())
            {
                os << '\n' << ygg::print_indent << "(:functions";
                {
                    ygg::IndentScope functions(os);
                    for (const auto& function : value.functions)
                    {
                        os << '\n' << ygg::print_indent;
                        fmt::print(os, fmt::runtime("{}"), function);
                    }
                }
                os << ')';
            }
            for (const auto& declaration : value.declarations)
            {
                os << '\n' << ygg::print_indent;
                boost::apply_visitor([&](const auto& node) { fmt::print(os, fmt::runtime("{}"), node); }, declaration);
            }
        }
        os << "\n)";
        return fmt::format_to(ctx.out(), fmt::runtime("{}"), os.view());
    }
};

template<>
struct formatter<loki::ast::Task, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::Task& value, FormatContext& ctx) const
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
};

template<>
struct formatter<loki::ast::File, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template<typename FormatContext>
    auto format(const loki::ast::File& value, FormatContext& ctx) const
    {
        auto out = ctx.out();
        boost::apply_visitor([&](const auto& node) { out = fmt::format_to(out, fmt::runtime("{}"), node); }, value);
        return out;
    }
};

}  // namespace fmt

#endif
