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

#ifndef LOKI_PARSER_AST_ADAPTED_HPP_
#define LOKI_PARSER_AST_ADAPTED_HPP_

#include "loki/ast/ast.hpp"

#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(loki::ast::Identifier, text)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::TypeReference, name)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::EitherType, alternatives)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::TypedGroup, names, type)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::Requirement, name)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::Term, name, variable)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::Atom, predicate, terms)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::Literal, atom, positive)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::FunctionTerm, function, terms)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::FunctionExpressionNumber, value)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::FunctionExpressionFunction, term)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::FunctionExpressionUnary, op, expression)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::FunctionExpressionBinary, op, left, right)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::FunctionExpressionMulti, op, expressions)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::ConditionLiteral, literal)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::ConditionAnd, conditions)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::ConditionOr, conditions)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::ConditionNot, condition)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::ConditionImply, left, right)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::ConditionExists, parameters, condition)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::ConditionForall, parameters, condition)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::ConditionNumericConstraint, comparator, left, right)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::EffectLiteral, literal)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::EffectAnd, effects)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::EffectNumeric, op, function, expression)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::EffectForall, parameters, effect)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::EffectWhen, condition, effect)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::EffectOneOf, effects)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::ProbabilisticEffectAlternative, probability, effect)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::EffectProbabilistic, alternatives)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::PredicateDeclaration, name, parameters)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::FunctionDeclaration, name, parameters, type)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::Action, name, parameters, precondition, effect)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::Axiom, head, condition)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::Metric, optimization, expression)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::InitialFunctionValue, function, value)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::Domain, name, requirements, types, constants, predicates, functions, declarations)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::Task, name, domain_name, requirements, types, predicates, objects, initial, goal, metric, axioms)

#endif
