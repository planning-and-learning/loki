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

#include <boost/mp11.hpp>
#include <string_view>
#include <urd/elements/concepts.hpp>
#include <urd/elements/index.hpp>
#include <urd/tags.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg::formalism
{

template<typename... Ts>
class RelationRepository;

template<typename SymbolRepo, typename RelationRepo>
class Repository;

}

namespace loki::formalism
{

struct Requirement
{
    static constexpr std::string_view name = "Requirement";
};

struct Type
{
    static constexpr std::string_view name = "Type";
};

struct Object
{
    static constexpr std::string_view name = "Object";
};

struct Variable
{
    static constexpr std::string_view name = "Variable";
};

struct Parameter
{
    static constexpr std::string_view name = "Parameter";
};

struct Predicate
{
    static constexpr std::string_view name = "Predicate";
};

struct FunctionSkeleton
{
    static constexpr std::string_view name = "FunctionSkeleton";
};

struct Atom
{
    static constexpr std::string_view name = "Atom";
};

struct Literal
{
    static constexpr std::string_view name = "Literal";
};

struct FunctionTerm
{
    static constexpr std::string_view name = "FunctionTerm";
};

struct EffectProbabilisticAlternative
{
    static constexpr std::string_view name = "EffectProbabilisticAlternative";
};

// Shared tags
struct LiteralTag
{
    static constexpr std::string_view name = "Literal";
};

struct AndTag
{
    static constexpr std::string_view name = "And";
};

struct ForallTag
{
    static constexpr std::string_view name = "Forall";
};

// Condition-only tags
struct OrTag
{
    static constexpr std::string_view name = "Or";
};

struct NotTag
{
    static constexpr std::string_view name = "Not";
};

struct ImplyTag
{
    static constexpr std::string_view name = "Imply";
};

struct ExistsTag
{
    static constexpr std::string_view name = "Exists";
};

struct NumericConstraintTag
{
    static constexpr std::string_view name = "NumericConstraint";
};

// Effect-only tags
struct NumericTag
{
    static constexpr std::string_view name = "Numeric";
};

struct WhenTag
{
    static constexpr std::string_view name = "When";
};

struct OneOfTag
{
    static constexpr std::string_view name = "OneOf";
};

struct ProbabilisticTag
{
    static constexpr std::string_view name = "Probabilistic";
};

// FunctionExpression tags
struct NumberTag
{
    static constexpr std::string_view name = "Number";
};

struct UnaryTag
{
    static constexpr std::string_view name = "Unary";
};

struct BinaryTag
{
    static constexpr std::string_view name = "Binary";
};

struct MultiTag
{
    static constexpr std::string_view name = "Multi";
};

using FunctionExpressionTags = boost::mp11::mp_list<NumberTag, UnaryTag, BinaryTag, MultiTag>;
using ConditionTags = boost::mp11::mp_list<LiteralTag, AndTag, OrTag, NotTag, ImplyTag, ExistsTag, ForallTag, NumericConstraintTag>;
using EffectTags = boost::mp11::mp_list<LiteralTag, AndTag, NumericTag, ForallTag, WhenTag, OneOfTag, ProbabilisticTag>;

template<typename Tag>
    requires boost::mp11::mp_contains<FunctionExpressionTags, Tag>::value
struct FunctionExpression
{
    static constexpr std::string_view name = urd::joined_name<"FunctionExpression", Tag::name>;
};

template<typename Tag>
    requires boost::mp11::mp_contains<ConditionTags, Tag>::value
struct Condition
{
    static constexpr std::string_view name = urd::joined_name<"Condition", Tag::name>;
};

template<typename Tag>
    requires boost::mp11::mp_contains<EffectTags, Tag>::value
struct Effect
{
    static constexpr std::string_view name = urd::joined_name<"Effect", Tag::name>;
};

using FunctionExpressionTypes = boost::mp11::mp_append<boost::mp11::mp_list<FunctionExpression<NumberTag>, FunctionTerm>,
                                                       boost::mp11::mp_transform<FunctionExpression, boost::mp11::mp_list<UnaryTag, BinaryTag, MultiTag>>>;

struct Action
{
    static constexpr std::string_view name = "Action";
};

struct Axiom
{
    static constexpr std::string_view name = "Axiom";
};

struct Metric
{
    static constexpr std::string_view name = "Metric";
};

struct InitialFunctionValue
{
    static constexpr std::string_view name = "InitialFunctionValue";
};

struct Domain
{
    static constexpr std::string_view name = "Domain";
};

struct Task
{
    static constexpr std::string_view name = "Task";
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

/// No family has rows in loki.
using RelationRepository = ::ygg::formalism::RelationRepository<>;

/// Named by what it holds (yggdrasil's Symbols and Relations), not by its stores: a call with a repository argument
/// then never instantiates them (yggdrasil/formalism/declarations.hpp).
using Repository =
    ::ygg::formalism::Repository<boost::mp11::mp_apply<::ygg::formalism::Symbols, SymbolRepositoryTypes>, ::ygg::formalism::Relations<>>;

template<typename T>
using EntityView = urd::index_view<T, Repository>;

using RequirementView = EntityView<Requirement>;
using TypeView = EntityView<Type>;
using ObjectView = EntityView<Object>;
using VariableView = EntityView<Variable>;
using ParameterView = EntityView<Parameter>;
using PredicateView = EntityView<Predicate>;
using FunctionSkeletonView = EntityView<FunctionSkeleton>;
using AtomView = EntityView<Atom>;
using LiteralView = EntityView<Literal>;
using FunctionTermView = EntityView<FunctionTerm>;
using EffectProbabilisticAlternativeView = EntityView<EffectProbabilisticAlternative>;
using ActionView = EntityView<Action>;
using AxiomView = EntityView<Axiom>;
using MetricView = EntityView<Metric>;
using InitialFunctionValueView = EntityView<InitialFunctionValue>;
using DomainView = EntityView<Domain>;
using TaskView = EntityView<Task>;

}

#endif
