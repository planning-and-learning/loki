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
#include <string>
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
    std::vector<TypeExpressionPtr> alternatives;
};

struct TypeExpression : boost::spirit::x3::position_tagged, boost::spirit::x3::variant<TypeReference, EitherType>
{
    using base_type::base_type;
    using base_type::operator=;
};

struct TypedGroup : boost::spirit::x3::position_tagged
{
    std::vector<Identifier> names;
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
    std::vector<Term> terms;
};

struct Literal : boost::spirit::x3::position_tagged
{
    Atom atom;
    bool positive = true;
};

struct FunctionTerm : boost::spirit::x3::position_tagged
{
    Identifier function;
    std::vector<Term> terms;
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
    std::vector<FunctionExpressionPtr> expressions;
};

struct FunctionExpression :
    boost::spirit::x3::position_tagged,
    boost::spirit::x3::variant<FunctionExpressionNumber, FunctionExpressionFunction, FunctionExpressionUnary, FunctionExpressionBinary, FunctionExpressionMulti>
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
    std::vector<ConditionPtr> conditions;
};

struct ConditionOr : boost::spirit::x3::position_tagged
{
    std::vector<ConditionPtr> conditions;
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
    std::vector<TypedGroup> parameters;
    ConditionPtr condition;
};

struct ConditionForall : boost::spirit::x3::position_tagged
{
    std::vector<TypedGroup> parameters;
    ConditionPtr condition;
};

struct ConditionNumericConstraint : boost::spirit::x3::position_tagged
{
    std::string comparator;
    FunctionExpressionPtr left;
    FunctionExpressionPtr right;
};

struct Condition :
    boost::spirit::x3::position_tagged,
    boost::spirit::x3::
        variant<ConditionLiteral, ConditionAnd, ConditionOr, ConditionNot, ConditionImply, ConditionExists, ConditionForall, ConditionNumericConstraint>
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
    std::vector<EffectPtr> effects;
};

struct EffectNumeric : boost::spirit::x3::position_tagged
{
    std::string op;
    FunctionTerm function;
    FunctionExpressionPtr expression;
};

struct EffectForall : boost::spirit::x3::position_tagged
{
    std::vector<TypedGroup> parameters;
    EffectPtr effect;
};

struct EffectWhen : boost::spirit::x3::position_tagged
{
    ConditionPtr condition;
    EffectPtr effect;
};

struct EffectOneOf : boost::spirit::x3::position_tagged
{
    std::vector<EffectPtr> effects;
};

struct ProbabilisticEffectAlternative : boost::spirit::x3::position_tagged
{
    double probability = 0.0;
    EffectPtr effect;
};

struct EffectProbabilistic : boost::spirit::x3::position_tagged
{
    std::vector<ProbabilisticEffectAlternative> alternatives;
};

struct Effect :
    boost::spirit::x3::position_tagged,
    boost::spirit::x3::variant<EffectLiteral, EffectAnd, EffectNumeric, EffectForall, EffectWhen, EffectOneOf, EffectProbabilistic>
{
    using base_type::base_type;
    using base_type::operator=;
};

struct PredicateDeclaration : boost::spirit::x3::position_tagged
{
    Identifier name;
    std::vector<TypedGroup> parameters;
};

struct FunctionDeclaration : boost::spirit::x3::position_tagged
{
    Identifier name;
    std::vector<TypedGroup> parameters;
    boost::optional<TypeExpression> type;
};

struct Action : boost::spirit::x3::position_tagged
{
    Identifier name;
    std::vector<TypedGroup> parameters;
    boost::optional<Condition> precondition;
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

struct InitialElement : boost::spirit::x3::position_tagged, boost::spirit::x3::variant<Literal, InitialFunctionValue>
{
    using base_type::base_type;
    using base_type::operator=;
};

using DomainDeclaration = boost::spirit::x3::variant<Axiom, Action>;

struct Domain : boost::spirit::x3::position_tagged
{
    Identifier name;
    std::vector<Requirement> requirements;
    std::vector<TypedGroup> types;
    std::vector<TypedGroup> constants;
    std::vector<PredicateDeclaration> predicates;
    std::vector<FunctionDeclaration> functions;
    std::vector<DomainDeclaration> declarations;
};

struct Task : boost::spirit::x3::position_tagged
{
    Identifier name;
    Identifier domain_name;
    std::vector<Requirement> requirements;
    std::vector<TypedGroup> objects;
    std::vector<InitialElement> initial;
    boost::optional<Condition> goal;
    boost::optional<Metric> metric;
    std::vector<Axiom> axioms;
};

using File = boost::spirit::x3::variant<Domain, Task>;
}

#endif
