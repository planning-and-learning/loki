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

#include <cista/containers/variant.h>
#include <cista/containers/vector.h>
#include <concepts>
#include <cstdint>
#include <memory>
#include <tuple>
#include <variant>
#include <boost/mp11.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg::formalism
{

template<typename... Ts>
class SymbolRepository;

template<typename ObjectTag, typename... Ts>
class RelationRepository;

template<typename SymbolRepo, typename RelationRepo>
class Repository;

}

namespace loki::formalism
{

struct ObjectTag
{
};

using ObjectBinding = ::ygg::formalism::Object<ObjectTag>;

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

/// A term: a variable or an object. Stored as it is, not interned.
using Term = urd::flat_variant<urd::index<Variable>, urd::index<Object>>;
using TermList = urd::flat_vector<Term>;

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

using FunctionExpressionTags = boost::mp11::mp_list<NumberTag, UnaryTag, BinaryTag, MultiTag>;
using ConditionTags = boost::mp11::mp_list<LiteralTag, AndTag, OrTag, NotTag, ImplyTag, ExistsTag, ForallTag, NumericConstraintTag>;
using EffectTags = boost::mp11::mp_list<LiteralTag, AndTag, NumericTag, ForallTag, WhenTag, OneOfTag, ProbabilisticTag>;

template<typename Tag>
    requires boost::mp11::mp_contains<FunctionExpressionTags, Tag>::value
struct FunctionExpression
{
};
template<typename Tag>
    requires boost::mp11::mp_contains<ConditionTags, Tag>::value
struct Condition
{
};
template<typename Tag>
    requires boost::mp11::mp_contains<EffectTags, Tag>::value
struct Effect
{
};

using FunctionExpressionTypes = boost::mp11::mp_append<boost::mp11::mp_list<FunctionExpression<NumberTag>, FunctionTerm>,
                                                      boost::mp11::mp_transform<FunctionExpression, boost::mp11::mp_list<UnaryTag, BinaryTag, MultiTag>>>;

/// A function expression, condition or effect: one of the family's records. Stored as it is, not interned.
using FunctionExpressionVariant = urd::index_variant<FunctionExpressionTypes>;
using FunctionExpressionList = urd::flat_vector<FunctionExpressionVariant>;
using ConditionVariant = urd::index_variant<boost::mp11::mp_transform<Condition, ConditionTags>>;
using ConditionList = urd::flat_vector<ConditionVariant>;
using EffectVariant = urd::index_variant<boost::mp11::mp_transform<Effect, EffectTags>>;
using EffectList = urd::flat_vector<EffectVariant>;

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

using SymbolRepositoryTypes =
    boost::mp11::mp_append<boost::mp11::mp_list<Requirement, Type, Object, Variable, Parameter, Predicate, FunctionSkeleton, Atom, Literal>,
                          FunctionExpressionTypes,
                          boost::mp11::mp_transform<Condition, ConditionTags>,
                          boost::mp11::mp_list<Effect<LiteralTag>,
                                        Effect<AndTag>,
                                        Effect<NumericTag>,
                                        Effect<ForallTag>,
                                        Effect<WhenTag>,
                                        Effect<OneOfTag>,
                                        EffectProbabilisticAlternative,
                                        Effect<ProbabilisticTag>,
                                        Action,
                                        Axiom,
                                        Metric,
                                        InitialFunctionValue,
                                        Domain,
                                        Task>>;

using SymbolRepository = boost::mp11::mp_apply<::ygg::formalism::SymbolRepository, SymbolRepositoryTypes>;

/// No family has rows in loki.
using RelationRepository = ::ygg::formalism::RelationRepository<ObjectTag>;

using Repository = ::ygg::formalism::Repository<SymbolRepository, RelationRepository>;

template<typename T>
using EntityView = urd::index_view<T, Repository>;

template<typename T>
using EntityListView = urd::resolved_range<urd::flat_vector<urd::index<T>>, Repository>;

using RequirementView = EntityView<Requirement>;
using TypeView = EntityView<Type>;
using ObjectView = EntityView<Object>;
using VariableView = EntityView<Variable>;
using ParameterView = EntityView<Parameter>;
using PredicateView = EntityView<Predicate>;
using FunctionSkeletonView = EntityView<FunctionSkeleton>;
/// A term read through the repository: its variable's or its object's view.
using TermView = std::variant<VariableView, ObjectView>;
using TermListView = urd::resolved_range<TermList, Repository>;
using AtomView = EntityView<Atom>;
using LiteralView = EntityView<Literal>;
using FunctionTermView = EntityView<FunctionTerm>;
/// A function expression, condition or effect read through the repository: the view of the record it holds.
using FunctionExpressionView = boost::mp11::mp_apply<std::variant, boost::mp11::mp_transform<EntityView, FunctionExpressionTypes>>;
using FunctionExpressionListView = urd::resolved_range<FunctionExpressionList, Repository>;
using ConditionView = boost::mp11::mp_apply<std::variant, boost::mp11::mp_transform<EntityView, boost::mp11::mp_transform<Condition, ConditionTags>>>;
using ConditionListView = urd::resolved_range<ConditionList, Repository>;
using EffectProbabilisticAlternativeView = EntityView<EffectProbabilisticAlternative>;
using EffectView = boost::mp11::mp_apply<std::variant, boost::mp11::mp_transform<EntityView, boost::mp11::mp_transform<Effect, EffectTags>>>;
using EffectListView = urd::resolved_range<EffectList, Repository>;
using ActionView = EntityView<Action>;
using AxiomView = EntityView<Axiom>;
using MetricView = EntityView<Metric>;
using InitialFunctionValueView = EntityView<InitialFunctionValue>;
using DomainView = EntityView<Domain>;
using TaskView = EntityView<Task>;

}

#endif
