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
struct FunctionTerm
{
};
struct EffectProbabilisticAlternative
{
};

// Shared tags
struct LiteralTag
{
};
struct AndTag
{
};
struct ForallTag
{
};

// Condition-only tags
struct OrTag
{
};
struct NotTag
{
};
struct ImplyTag
{
};
struct ExistsTag
{
};
struct NumericConstraintTag
{
};

// Effect-only tags
struct NumericTag
{
};
struct WhenTag
{
};
struct OneOfTag
{
};
struct ProbabilisticTag
{
};

// FunctionExpression tags
struct NumberTag
{
};
struct UnaryTag
{
};
struct BinaryTag
{
};
struct MultiTag
{
};

using FunctionExpressionTags = ygg::TypeList<NumberTag, UnaryTag, BinaryTag, MultiTag>;
using ConditionTags = ygg::TypeList<LiteralTag, AndTag, OrTag, NotTag, ImplyTag, ExistsTag, ForallTag, NumericConstraintTag>;
using EffectTags = ygg::TypeList<LiteralTag, AndTag, NumericTag, ForallTag, WhenTag, OneOfTag, ProbabilisticTag>;

/// Tag void is the type-erased record holding one of the family's concrete records.
template<typename Tag = void>
    requires(std::is_void_v<Tag> || ygg::InTypeList<Tag, FunctionExpressionTags>)
struct FunctionExpression
{
};
template<typename Tag = void>
    requires(std::is_void_v<Tag> || ygg::InTypeList<Tag, ConditionTags>)
struct Condition
{
};
template<typename Tag = void>
    requires(std::is_void_v<Tag> || ygg::InTypeList<Tag, EffectTags>)
struct Effect
{
};

using FunctionExpressionTypes =
    ygg::ConcatTypeListsT<ygg::TypeList<FunctionExpression<NumberTag>, FunctionTerm>, ygg::MapTypeListT<FunctionExpression, ygg::TypeList<UnaryTag, BinaryTag, MultiTag>>>;

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

using SymbolRepositoryTypes = ygg::ConcatTypeListsT<ygg::TypeList<Requirement,
                                                                   Type,
                                                                   Object,
                                                                   Variable,
                                                                   Parameter,
                                                                   Predicate,
                                                                   FunctionSkeleton,
                                                                   Term,
                                                                   Atom,
                                                                   Literal>,
                                                    FunctionExpressionTypes,
                                                    ygg::TypeList<FunctionExpression<>>,
                                                    ygg::MapTypeListT<Condition, ConditionTags>,
                                                    ygg::TypeList<Condition<>,
                                                                  Effect<LiteralTag>,
                                                                  Effect<AndTag>,
                                                                  Effect<NumericTag>,
                                                                  Effect<ForallTag>,
                                                                  Effect<WhenTag>,
                                                                  Effect<OneOfTag>,
                                                                  EffectProbabilisticAlternative,
                                                                  Effect<ProbabilisticTag>,
                                                                  Effect<>,
                                                                  Action,
                                                                  Axiom,
                                                                  Metric,
                                                                  InitialFunctionValue,
                                                                  Domain,
                                                                  Task>>;
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
using FunctionTermView = EntityView<FunctionTerm>;
using FunctionExpressionView = EntityView<FunctionExpression<>>;
using ConditionView = EntityView<Condition<>>;
using EffectProbabilisticAlternativeView = EntityView<EffectProbabilisticAlternative>;
using EffectView = EntityView<Effect<>>;
using ActionView = EntityView<Action>;
using AxiomView = EntityView<Axiom>;
using MetricView = EntityView<Metric>;
using InitialFunctionValueView = EntityView<InitialFunctionValue>;
using DomainView = EntityView<Domain>;
using TaskView = EntityView<Task>;

}

#endif
