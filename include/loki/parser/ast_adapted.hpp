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
BOOST_FUSION_ADAPT_STRUCT(loki::ast::ConditionTimed, time, condition)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::ConditionPreference, name, condition)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::FunctionExpressionIsViolated, name)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::ConstraintAnd, constraints)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::ConstraintForall, parameters, constraint)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::ConstraintPreference, name, constraint)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::ConstraintOr, constraints)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::ConstraintNot, constraint)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::ConstraintImply, left, right)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::ConstraintExists, parameters, constraint)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::ConstraintState, condition)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::ConstraintModal, operation, numbers, constraints)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::ConstraintSection, constraint)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::EffectLiteral, literal)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::EffectAnd, effects)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::EffectNumeric, op, function, expression)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::EffectForall, parameters, effect)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::EffectWhen, condition, effect)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::EffectOneOf, effects)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::ProbabilisticEffectAlternative, probability, denominator, effect)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::EffectProbabilistic, alternatives)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::EffectTimed, time, effect)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::PredicateDeclaration, name, parameters)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::FunctionDeclaration, name, parameters, type)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::Action, name, parameters, precondition, effect)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::DurativeAction, name, parameters, duration, condition, effect)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::Axiom, head, condition)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::Metric, optimization, expression)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::InitialFunctionValue, function, value)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::TimedInitialLiteral, time, literal)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::ProbabilisticInitialAlternative, probability, denominator, atoms)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::InitialProbabilistic, alternatives)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::RequirementSection, requirements)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::TypeSection, types)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::ConstantSection, constants)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::PredicateSection, predicates)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::FunctionSection, functions)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::Domain, name, elements)
BOOST_FUSION_ADAPT_STRUCT(loki::ast::Task, name, domain_name, requirements, types, predicates, objects, initial, goal, goal_reward, constraints, metric, axioms)

#endif
