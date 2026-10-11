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

#ifndef LOKI_AST_AST_FWD_HPP_
#define LOKI_AST_AST_FWD_HPP_

#include <boost/spirit/home/x3/support/ast/variant.hpp>

namespace loki::ast
{
struct Identifier;
struct TypeExpression;
struct TypeReference;
struct EitherType;
struct TypedGroup;
struct Requirement;
struct Term;
struct Atom;
struct Literal;
struct FunctionTerm;
struct FunctionExpression;
struct FunctionExpressionNumber;
struct FunctionExpressionFunction;
struct FunctionExpressionUnary;
struct FunctionExpressionBinary;
struct FunctionExpressionMulti;
struct Condition;
struct ConditionLiteral;
struct ConditionAnd;
struct ConditionOr;
struct ConditionNot;
struct ConditionImply;
struct ConditionExists;
struct ConditionForall;
struct ConditionNumericConstraint;
struct Effect;
struct EffectLiteral;
struct EffectAnd;
struct EffectNumeric;
struct EffectForall;
struct EffectWhen;
struct EffectOneOf;
struct EffectProbabilistic;
struct PredicateDeclaration;
struct FunctionDeclaration;
struct Action;
struct Axiom;
struct Metric;
struct Domain;
struct InitialFunctionValue;
struct InitialElement;
struct Task;

using DomainDeclaration = boost::spirit::x3::variant<Axiom, Action>;
using File = boost::spirit::x3::variant<Domain, Task>;
}

#endif
