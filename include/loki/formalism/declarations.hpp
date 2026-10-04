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

#ifndef LOKI_FORMALISM_DECLARATIONS_HPP_
#define LOKI_FORMALISM_DECLARATIONS_HPP_

#include "loki/formalism/enums.hpp"

#include <concepts>
#include <cstdint>
#include <memory>
#include <tuple>
#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>
#include <yggdrasil/semantics/comparison.hpp>

namespace ygg::formalism
{

template<typename... Ts>
class SymbolRepository;

template<typename ObjectTag, typename... Ts>
class RelationRepository;

template<typename SymbolRepo, typename RelationRepo>
class Repository;

template<typename SymbolRepo, typename RelationRepo>
class RepositoryFactory;

}

namespace loki::formalism
{

struct ObjectTag
{
};

using ObjectBinding = ::ygg::formalism::Object<ObjectTag>;
using Row = ::ygg::formalism::Row;

template<typename T>
using RelationBinding = ::ygg::formalism::RelationBinding<T, ObjectTag>;

struct Requirement
{
};
struct Type
{
};
struct Object
{
};
struct Variable
{
};
struct Parameter
{
};
struct Predicate
{
};
struct FunctionSkeleton
{
};
struct Term
{
};
struct Atom
{
};
struct Literal
{
};
struct FunctionExpressionNumber
{
};
struct FunctionTerm
{
};
struct UnaryFunctionExpression
{
};
struct BinaryFunctionExpression
{
};
struct MultiFunctionExpression
{
};
struct FunctionExpression
{
};
struct ConditionLiteral
{
};
struct ConditionAnd
{
};
struct ConditionOr
{
};
struct ConditionNot
{
};
struct ConditionImply
{
};
struct ConditionExists
{
};
struct ConditionForall
{
};
struct ConditionNumericConstraint
{
};
struct Condition
{
};
struct EffectLiteral
{
};
struct EffectAnd
{
};
struct EffectNumeric
{
};
struct EffectForall
{
};
struct EffectWhen
{
};
struct EffectOneOf
{
};
struct EffectProbabilisticAlternative
{
};
struct EffectProbabilistic
{
};
struct Effect
{
};
struct Action
{
};
struct Axiom
{
};
struct Metric
{
};
struct InitialFunctionValue
{
};
struct Domain
{
};
struct Task
{
};

using SymbolRepositoryTypes = ygg::TypeList<Requirement,
                                            Type,
                                            Object,
                                            Variable,
                                            Parameter,
                                            Predicate,
                                            FunctionSkeleton,
                                            Term,
                                            Atom,
                                            Literal,
                                            FunctionExpressionNumber,
                                            FunctionTerm,
                                            UnaryFunctionExpression,
                                            BinaryFunctionExpression,
                                            MultiFunctionExpression,
                                            FunctionExpression,
                                            ConditionLiteral,
                                            ConditionAnd,
                                            ConditionOr,
                                            ConditionNot,
                                            ConditionImply,
                                            ConditionExists,
                                            ConditionForall,
                                            ConditionNumericConstraint,
                                            Condition,
                                            EffectLiteral,
                                            EffectAnd,
                                            EffectNumeric,
                                            EffectForall,
                                            EffectWhen,
                                            EffectOneOf,
                                            EffectProbabilisticAlternative,
                                            EffectProbabilistic,
                                            Effect,
                                            Action,
                                            Axiom,
                                            Metric,
                                            InitialFunctionValue,
                                            Domain,
                                            Task>;
using RelationRepositoryTypes = ygg::TypeList<Atom>;

using SymbolRepository = ygg::ApplyTypeListT<::ygg::formalism::SymbolRepository, SymbolRepositoryTypes>;

using RelationRepository = ::ygg::formalism::RelationRepository<ObjectTag, Atom>;

using Repository = ::ygg::formalism::Repository<SymbolRepository, RelationRepository>;
using RepositoryPtr = std::shared_ptr<Repository>;
using RepositoryFactory = ::ygg::formalism::RepositoryFactory<SymbolRepository, RelationRepository>;
using RepositoryFactoryPtr = std::shared_ptr<RepositoryFactory>;

template<typename T>
using EntityView = ygg::View<ygg::Index<T>, Repository>;

template<typename T>
using EntityListView = ygg::View<ygg::IndexList<T>, Repository>;

using RequirementView = EntityView<Requirement>;
using TypeView = EntityView<Type>;
using ObjectView = EntityView<Object>;
using VariableView = EntityView<Variable>;
using ParameterView = EntityView<Parameter>;
using PredicateView = EntityView<Predicate>;
using FunctionSkeletonView = EntityView<FunctionSkeleton>;
using TermView = EntityView<Term>;
using AtomView = EntityView<Atom>;
using LiteralView = EntityView<Literal>;
using FunctionExpressionNumberView = EntityView<FunctionExpressionNumber>;
using FunctionTermView = EntityView<FunctionTerm>;
using UnaryFunctionExpressionView = EntityView<UnaryFunctionExpression>;
using BinaryFunctionExpressionView = EntityView<BinaryFunctionExpression>;
using MultiFunctionExpressionView = EntityView<MultiFunctionExpression>;
using FunctionExpressionView = EntityView<FunctionExpression>;
using ConditionLiteralView = EntityView<ConditionLiteral>;
using ConditionAndView = EntityView<ConditionAnd>;
using ConditionOrView = EntityView<ConditionOr>;
using ConditionNotView = EntityView<ConditionNot>;
using ConditionImplyView = EntityView<ConditionImply>;
using ConditionExistsView = EntityView<ConditionExists>;
using ConditionForallView = EntityView<ConditionForall>;
using ConditionNumericConstraintView = EntityView<ConditionNumericConstraint>;
using ConditionView = EntityView<Condition>;
using EffectLiteralView = EntityView<EffectLiteral>;
using EffectAndView = EntityView<EffectAnd>;
using EffectNumericView = EntityView<EffectNumeric>;
using EffectForallView = EntityView<EffectForall>;
using EffectWhenView = EntityView<EffectWhen>;
using EffectOneOfView = EntityView<EffectOneOf>;
using EffectProbabilisticAlternativeView = EntityView<EffectProbabilisticAlternative>;
using EffectProbabilisticView = EntityView<EffectProbabilistic>;
using EffectView = EntityView<Effect>;
using ActionView = EntityView<Action>;
using AxiomView = EntityView<Axiom>;
using MetricView = EntityView<Metric>;
using InitialFunctionValueView = EntityView<InitialFunctionValue>;
using DomainView = EntityView<Domain>;
using TaskView = EntityView<Task>;

}

#endif
