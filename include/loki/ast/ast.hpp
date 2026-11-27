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

#ifndef LOKI_AST_AST_HPP_
#define LOKI_AST_AST_HPP_

#include <boost/optional.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <fmt/base.h>
#include <string>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>

namespace loki::ast
{
struct Identifier : boost::spirit::x3::position_tagged
{
    std::string text;
};

struct TypeExpression;
using TypeExpressionPtr = boost::spirit::x3::forward_ast<TypeExpression>;

struct TypeReference : boost::spirit::x3::position_tagged
{
    Identifier name;
};

struct EitherType : boost::spirit::x3::position_tagged
{
    urd::Store<urd::sequence<TypeExpressionPtr>> alternatives;
};

struct TypeExpression : boost::spirit::x3::position_tagged, boost::spirit::x3::variant<TypeReference, EitherType>
{
    using base_type::base_type;
    using base_type::operator=;
};

struct TypedGroup : boost::spirit::x3::position_tagged
{
    urd::Store<urd::sequence<Identifier>> names;
    boost::optional<TypeExpression> type;
};

struct Requirement : boost::spirit::x3::position_tagged
{
    Identifier name;
};

struct Term : boost::spirit::x3::position_tagged
{
    Identifier name;
    bool variable = false;
};

struct Atom : boost::spirit::x3::position_tagged
{
    Identifier predicate;
    urd::Store<urd::sequence<Term>> terms;
};

struct Literal : boost::spirit::x3::position_tagged
{
    Atom atom;
    bool positive = true;
};

struct FunctionTerm : boost::spirit::x3::position_tagged
{
    Identifier function;
    urd::Store<urd::sequence<Term>> terms;
};

struct FunctionExpression;
using FunctionExpressionPtr = boost::spirit::x3::forward_ast<FunctionExpression>;

struct FunctionExpressionNumber : boost::spirit::x3::position_tagged
{
    double value = 0.0;
};

struct FunctionExpressionFunction : boost::spirit::x3::position_tagged
{
    FunctionTerm term;
};

struct FunctionExpressionUnary : boost::spirit::x3::position_tagged
{
    std::string op;
    FunctionExpressionPtr expression;
};

struct FunctionExpressionBinary : boost::spirit::x3::position_tagged
{
    std::string op;
    FunctionExpressionPtr left;
    FunctionExpressionPtr right;
};

struct FunctionExpressionMulti : boost::spirit::x3::position_tagged
{
    std::string op;
    urd::Store<urd::sequence<FunctionExpressionPtr>> expressions;
};

/// (is-violated name): the number of violations of the preferences of the name (PDDL 3).
struct FunctionExpressionIsViolated : boost::spirit::x3::position_tagged
{
    Identifier name;
};

/// ?duration: the duration of the durative action it occurs in.
struct FunctionExpressionDuration : boost::spirit::x3::position_tagged
{
};

struct FunctionExpression :
    boost::spirit::x3::position_tagged,
    boost::spirit::x3::variant<FunctionExpressionNumber,
                               FunctionExpressionFunction,
                               FunctionExpressionUnary,
                               FunctionExpressionBinary,
                               FunctionExpressionMulti,
                               FunctionExpressionDuration,
                               FunctionExpressionIsViolated>
{
    using base_type::base_type;
    using base_type::operator=;
};

struct Condition;
using ConditionPtr = boost::spirit::x3::forward_ast<Condition>;

struct ConditionLiteral : boost::spirit::x3::position_tagged
{
    Literal literal;
};

struct ConditionAnd : boost::spirit::x3::position_tagged
{
    urd::Store<urd::sequence<ConditionPtr>> conditions;
};

struct ConditionOr : boost::spirit::x3::position_tagged
{
    urd::Store<urd::sequence<ConditionPtr>> conditions;
};

struct ConditionNot : boost::spirit::x3::position_tagged
{
    ConditionPtr condition;
};

struct ConditionImply : boost::spirit::x3::position_tagged
{
    ConditionPtr left;
    ConditionPtr right;
};

struct ConditionExists : boost::spirit::x3::position_tagged
{
    urd::Store<urd::sequence<TypedGroup>> parameters;
    ConditionPtr condition;
};

struct ConditionForall : boost::spirit::x3::position_tagged
{
    urd::Store<urd::sequence<TypedGroup>> parameters;
    ConditionPtr condition;
};

struct ConditionNumericConstraint : boost::spirit::x3::position_tagged
{
    std::string comparator;
    FunctionExpressionPtr left;
    FunctionExpressionPtr right;
};

/// (preference [name] c): a soft condition (PDDL 3).
struct ConditionPreference : boost::spirit::x3::position_tagged
{
    boost::optional<Identifier> name;
    ConditionPtr condition;
};

/// (at start c), (at end c) or (over all c): time is 's', 'e' or 'a'.
struct ConditionTimed : boost::spirit::x3::position_tagged
{
    char time = 's';
    ConditionPtr condition;
};

struct Condition :
    boost::spirit::x3::position_tagged,
    boost::spirit::x3::variant<ConditionLiteral,
                               ConditionAnd,
                               ConditionOr,
                               ConditionNot,
                               ConditionImply,
                               ConditionExists,
                               ConditionForall,
                               ConditionNumericConstraint,
                               ConditionTimed,
                               ConditionPreference>
{
    using base_type::base_type;
    using base_type::operator=;
};

struct Effect;
using EffectPtr = boost::spirit::x3::forward_ast<Effect>;

struct EffectLiteral : boost::spirit::x3::position_tagged
{
    Literal literal;
};

struct EffectAnd : boost::spirit::x3::position_tagged
{
    urd::Store<urd::sequence<EffectPtr>> effects;
};

struct EffectNumeric : boost::spirit::x3::position_tagged
{
    std::string op;
    FunctionTerm function;
    FunctionExpressionPtr expression;
};

struct EffectForall : boost::spirit::x3::position_tagged
{
    urd::Store<urd::sequence<TypedGroup>> parameters;
    EffectPtr effect;
};

struct EffectWhen : boost::spirit::x3::position_tagged
{
    ConditionPtr condition;
    EffectPtr effect;
};

struct EffectOneOf : boost::spirit::x3::position_tagged
{
    urd::Store<urd::sequence<EffectPtr>> effects;
};

/// p e, where p is a number or a fraction n/d (PPDDL).
struct ProbabilisticEffectAlternative : boost::spirit::x3::position_tagged
{
    double probability = 0.0;
    boost::optional<double> denominator;
    EffectPtr effect;
};

struct EffectProbabilistic : boost::spirit::x3::position_tagged
{
    urd::Store<urd::sequence<ProbabilisticEffectAlternative>> alternatives;
};

/// (at start e) or (at end e): time is 's' or 'e'.
struct EffectTimed : boost::spirit::x3::position_tagged
{
    char time = 's';
    EffectPtr effect;
};

struct Effect :
    boost::spirit::x3::position_tagged,
    boost::spirit::x3::variant<EffectLiteral, EffectAnd, EffectNumeric, EffectForall, EffectWhen, EffectOneOf, EffectProbabilistic, EffectTimed>
{
    using base_type::base_type;
    using base_type::operator=;
};

struct PredicateDeclaration : boost::spirit::x3::position_tagged
{
    Identifier name;
    urd::Store<urd::sequence<TypedGroup>> parameters;
};

struct FunctionDeclaration : boost::spirit::x3::position_tagged
{
    Identifier name;
    urd::Store<urd::sequence<TypedGroup>> parameters;
    boost::optional<TypeExpression> type;
};

struct Action : boost::spirit::x3::position_tagged
{
    Identifier name;
    urd::Store<urd::sequence<TypedGroup>> parameters;
    boost::optional<Condition> precondition;
    boost::optional<Effect> effect;
};

struct Constraint;
using ConstraintPtr = boost::spirit::x3::forward_ast<Constraint>;

struct ConstraintAnd : boost::spirit::x3::position_tagged
{
    urd::Store<urd::sequence<ConstraintPtr>> constraints;
};

struct ConstraintOr : boost::spirit::x3::position_tagged
{
    urd::Store<urd::sequence<ConstraintPtr>> constraints;
};

struct ConstraintNot : boost::spirit::x3::position_tagged
{
    ConstraintPtr constraint;
};

struct ConstraintImply : boost::spirit::x3::position_tagged
{
    ConstraintPtr left;
    ConstraintPtr right;
};

struct ConstraintExists : boost::spirit::x3::position_tagged
{
    urd::Store<urd::sequence<TypedGroup>> parameters;
    ConstraintPtr constraint;
};

/// A goal description as a formula: it holds at the position it is read at.
struct ConstraintState : boost::spirit::x3::position_tagged
{
    Condition condition;
};

struct ConstraintForall : boost::spirit::x3::position_tagged
{
    urd::Store<urd::sequence<TypedGroup>> parameters;
    ConstraintPtr constraint;
};

struct ConstraintPreference : boost::spirit::x3::position_tagged
{
    boost::optional<Identifier> name;
    ConstraintPtr constraint;
};

/// A modal formula: its operator (PDDL 3's 'E' at end, 'A' always, 'S' sometime, 'W' within, 'O' at-most-once, 'a' sometime-after,
/// 'b' sometime-before, 'w' always-within, 'h' hold-during, 'H' hold-after; :trajectory-formulas' 'n' next, 'N' weak-next,
/// 'u' until, 'r' release, 'y' yesterday, 'Y' weak-yesterday, 's' since, 't' triggers, 'o' once, 'i' historically), its numbers
/// (PDDL 3's time bounds, or the interval of a metric operator), then its operands.
struct ConstraintModal : boost::spirit::x3::position_tagged
{
    char operation = 'A';
    urd::Store<urd::sequence<double>> numbers;
    urd::Store<urd::sequence<ConstraintPtr>> constraints;
};

/// A formula over the state trajectory: a PDDL 3 constraint, or one of :trajectory-formulas.
struct Constraint :
    boost::spirit::x3::position_tagged,
    boost::spirit::x3::variant<ConstraintAnd,
                               ConstraintOr,
                               ConstraintNot,
                               ConstraintImply,
                               ConstraintExists,
                               ConstraintForall,
                               ConstraintPreference,
                               ConstraintModal,
                               ConstraintState>
{
    using base_type::base_type;
    using base_type::operator=;
};

struct DurativeAction : boost::spirit::x3::position_tagged
{
    Identifier name;
    urd::Store<urd::sequence<TypedGroup>> parameters;
    Condition duration;
    boost::optional<Condition> condition;
    boost::optional<Effect> effect;
};

struct Axiom : boost::spirit::x3::position_tagged
{
    PredicateDeclaration head;
    Condition condition;
};

struct Metric : boost::spirit::x3::position_tagged
{
    Identifier optimization;
    FunctionExpression expression;
};

struct InitialFunctionValue : boost::spirit::x3::position_tagged
{
    FunctionTerm function;
    FunctionExpression value;
};

/// (at time literal): a literal that becomes true at a time point.
struct TimedInitialLiteral : boost::spirit::x3::position_tagged
{
    double time = 0.0;
    Literal literal;
};

/// A literal or a function value of a probabilistic initial element.
struct InitialAtom : boost::spirit::x3::position_tagged, boost::spirit::x3::variant<Literal, InitialFunctionValue>
{
    using base_type::base_type;
    using base_type::operator=;
};

/// p e of a probabilistic initial element: e a literal, a function value, or a conjunction of them; p as in effects.
struct ProbabilisticInitialAlternative : boost::spirit::x3::position_tagged
{
    double probability = 0.0;
    boost::optional<double> denominator;
    urd::Store<urd::sequence<InitialAtom>> atoms;
};

/// (probabilistic p1 e1 ... pn en) in :init (PPDDL): the initial state is drawn.
struct InitialProbabilistic : boost::spirit::x3::position_tagged
{
    urd::Store<urd::sequence<ProbabilisticInitialAlternative>> alternatives;
};

struct InitialElement : boost::spirit::x3::position_tagged, boost::spirit::x3::variant<Literal, InitialFunctionValue, TimedInitialLiteral, InitialProbabilistic>
{
    using base_type::base_type;
    using base_type::operator=;
};

using DomainDeclaration = boost::spirit::x3::variant<Axiom, Action, DurativeAction>;

struct RequirementSection : boost::spirit::x3::position_tagged
{
    urd::Store<urd::sequence<Requirement>> requirements;
};

struct TypeSection : boost::spirit::x3::position_tagged
{
    urd::Store<urd::sequence<TypedGroup>> types;
};

struct ConstantSection : boost::spirit::x3::position_tagged
{
    urd::Store<urd::sequence<TypedGroup>> constants;
};

struct PredicateSection : boost::spirit::x3::position_tagged
{
    urd::Store<urd::sequence<PredicateDeclaration>> predicates;
};

struct FunctionSection : boost::spirit::x3::position_tagged
{
    urd::Store<urd::sequence<FunctionDeclaration>> functions;
};

struct ConstraintSection : boost::spirit::x3::position_tagged
{
    Constraint constraint;
};

/// A section or declaration of a domain, in the order of its source: domains of the competitions order their
/// sections freely (constants after predicates, functions before types).
using DomainElement = boost::spirit::x3::
    variant<RequirementSection, TypeSection, ConstantSection, PredicateSection, FunctionSection, ConstraintSection, Axiom, Action, DurativeAction>;

struct Domain : boost::spirit::x3::position_tagged
{
    Identifier name;
    urd::Store<urd::sequence<DomainElement>> elements;
};

/// A domain's sections, each kind concatenated in source order, and its declarations in source order.
struct DomainParts
{
    urd::Store<urd::sequence<Requirement>> requirements;
    urd::Store<urd::sequence<TypedGroup>> types;
    urd::Store<urd::sequence<TypedGroup>> constants;
    urd::Store<urd::sequence<PredicateDeclaration>> predicates;
    urd::Store<urd::sequence<FunctionDeclaration>> functions;
    urd::Store<urd::sequence<Constraint>> constraints;
    urd::Store<urd::sequence<DomainDeclaration>> declarations;
};

inline DomainParts parts(const Domain& domain)
{
    auto result = DomainParts {};
    const auto append = [](auto& to, const auto& from) { to.insert(to.end(), from.begin(), from.end()); };
    for (const auto& element : domain.elements)
    {
        boost::apply_visitor(
            [&]<typename Node>(const Node& node)
            {
                if constexpr (std::is_same_v<Node, RequirementSection>)
                    append(result.requirements, node.requirements);
                else if constexpr (std::is_same_v<Node, TypeSection>)
                    append(result.types, node.types);
                else if constexpr (std::is_same_v<Node, ConstantSection>)
                    append(result.constants, node.constants);
                else if constexpr (std::is_same_v<Node, PredicateSection>)
                    append(result.predicates, node.predicates);
                else if constexpr (std::is_same_v<Node, FunctionSection>)
                    append(result.functions, node.functions);
                else if constexpr (std::is_same_v<Node, ConstraintSection>)
                    result.constraints.push_back(node.constraint);
                else
                    result.declarations.push_back(DomainDeclaration(node));
            },
            element);
    }
    return result;
}

struct Task : boost::spirit::x3::position_tagged
{
    Identifier name;
    Identifier domain_name;
    urd::Store<urd::sequence<Requirement>> requirements;
    urd::Store<urd::sequence<TypedGroup>> types;                 ///< Extends the domain's types; not standard PDDL.
    urd::Store<urd::sequence<PredicateDeclaration>> predicates;  ///< Extends the domain's predicates; not standard PDDL.
    urd::Store<urd::sequence<TypedGroup>> objects;
    urd::Store<urd::sequence<InitialElement>> initial;
    boost::optional<Constraint> goal;         ///< A goal description, or a formula over the state trajectory (:trajectory-formulas).
    boost::optional<double> goal_reward;      ///< PPDDL's (:goal-reward n).
    boost::optional<Constraint> constraints;  ///< PDDL 3's (:constraints c).
    boost::optional<Metric> metric;
    urd::Store<urd::sequence<Axiom>> axioms;
};

using File = boost::spirit::x3::variant<Domain, Task>;
}

// How each node is written (PDDL), defined in src/ast/formatter.cpp.

template<>
struct fmt::formatter<loki::ast::Identifier, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::Identifier& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::TypeReference, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::TypeReference& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::EitherType, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::EitherType& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::TypeExpression, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::TypeExpression& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::TypedGroup, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::TypedGroup& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::Requirement, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::Requirement& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::Term, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::Term& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::Atom, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::Atom& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::Literal, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::Literal& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::FunctionTerm, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::FunctionTerm& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::FunctionExpressionNumber, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::FunctionExpressionNumber& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::FunctionExpressionFunction, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::FunctionExpressionFunction& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::FunctionExpressionUnary, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::FunctionExpressionUnary& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::FunctionExpressionBinary, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::FunctionExpressionBinary& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::FunctionExpressionMulti, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::FunctionExpressionMulti& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::FunctionExpressionDuration, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::FunctionExpressionDuration& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::FunctionExpressionIsViolated, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::FunctionExpressionIsViolated& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::FunctionExpression, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::FunctionExpression& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::ConditionLiteral, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::ConditionLiteral& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::ConditionAnd, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::ConditionAnd& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::ConditionOr, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::ConditionOr& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::ConditionNot, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::ConditionNot& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::ConditionImply, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::ConditionImply& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::ConditionExists, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::ConditionExists& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::ConditionForall, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::ConditionForall& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::ConditionNumericConstraint, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::ConditionNumericConstraint& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::ConditionTimed, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::ConditionTimed& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::ConditionPreference, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::ConditionPreference& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::Condition, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::Condition& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::EffectLiteral, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::EffectLiteral& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::EffectAnd, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::EffectAnd& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::EffectNumeric, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::EffectNumeric& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::EffectForall, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::EffectForall& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::EffectWhen, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::EffectWhen& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::EffectOneOf, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::EffectOneOf& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::ProbabilisticEffectAlternative, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::ProbabilisticEffectAlternative& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::EffectProbabilistic, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::EffectProbabilistic& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::EffectTimed, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::EffectTimed& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::Effect, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::Effect& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::PredicateDeclaration, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::PredicateDeclaration& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::FunctionDeclaration, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::FunctionDeclaration& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::Action, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::Action& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::ConstraintAnd, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::ConstraintAnd& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::ConstraintOr, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::ConstraintOr& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::ConstraintNot, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::ConstraintNot& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::ConstraintImply, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::ConstraintImply& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::ConstraintExists, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::ConstraintExists& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::ConstraintState, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::ConstraintState& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::ConstraintForall, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::ConstraintForall& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::ConstraintPreference, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::ConstraintPreference& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::ConstraintModal, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::ConstraintModal& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::Constraint, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::Constraint& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::ConstraintSection, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::ConstraintSection& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::DurativeAction, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::DurativeAction& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::Axiom, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::Axiom& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::Metric, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::Metric& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::InitialFunctionValue, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::InitialFunctionValue& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::TimedInitialLiteral, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::TimedInitialLiteral& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::InitialAtom, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::InitialAtom& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::ProbabilisticInitialAlternative, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::ProbabilisticInitialAlternative& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::InitialProbabilistic, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::InitialProbabilistic& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::InitialElement, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::InitialElement& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::Domain, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::Domain& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::Task, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::Task& value, fmt::format_context& ctx) const;
};

template<>
struct fmt::formatter<loki::ast::File, char>
{
    constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

    fmt::format_context::iterator format(const loki::ast::File& value, fmt::format_context& ctx) const;
};

#endif
