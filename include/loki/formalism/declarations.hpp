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

/// A soft condition or constraint of PDDL 3.
struct PreferenceTag
{
    static constexpr std::string_view name = "Preference";
};

/// A condition or effect of a durative action at a time point or over its interval (TimeSpecifier).
struct TimedTag
{
    static constexpr std::string_view name = "Timed";
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

/// The number of violations of the preferences of a name: (is-violated name).
struct IsViolatedTag
{
    static constexpr std::string_view name = "IsViolated";
};

/// A modal constraint of PDDL 3 (ConstraintOperator).
struct ModalTag
{
    static constexpr std::string_view name = "Modal";
};

/// A goal description as a formula over the state trajectory: it holds at the position it is read at.
struct StateTag
{
    static constexpr std::string_view name = "State";
};

/// The duration of the durative action: ?duration.
struct DurationTag
{
    static constexpr std::string_view name = "Duration";
};

using FunctionExpressionTags = boost::mp11::mp_list<NumberTag, UnaryTag, BinaryTag, MultiTag, DurationTag, IsViolatedTag>;
using ConditionTags = boost::mp11::mp_list<LiteralTag, AndTag, OrTag, NotTag, ImplyTag, ExistsTag, ForallTag, NumericConstraintTag, TimedTag, PreferenceTag>;
using ConstraintTags = boost::mp11::mp_list<StateTag, AndTag, OrTag, NotTag, ExistsTag, ForallTag, PreferenceTag, ModalTag>;
using EffectTags = boost::mp11::mp_list<LiteralTag, AndTag, NumericTag, ForallTag, WhenTag, OneOfTag, ProbabilisticTag, TimedTag>;

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

template<typename Tag>
    requires boost::mp11::mp_contains<ConstraintTags, Tag>::value
struct Constraint
{
    static constexpr std::string_view name = urd::joined_name<"Constraint", Tag::name>;
};

using FunctionExpressionTypes =
    boost::mp11::mp_append<boost::mp11::mp_list<FunctionExpression<NumberTag>, FunctionTerm>,
                           boost::mp11::mp_transform<FunctionExpression, boost::mp11::mp_list<UnaryTag, BinaryTag, MultiTag, DurationTag, IsViolatedTag>>>;

struct Action
{
    static constexpr std::string_view name = "Action";
};

struct DurativeAction
{
    static constexpr std::string_view name = "DurativeAction";
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

struct TimedInitialLiteral
{
    static constexpr std::string_view name = "TimedInitialLiteral";
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
                                                Effect<TimedTag>,
                                                Constraint<StateTag>,
                                                Constraint<AndTag>,
                                                Constraint<OrTag>,
                                                Constraint<NotTag>,
                                                Constraint<ExistsTag>,
                                                Constraint<ForallTag>,
                                                Constraint<PreferenceTag>,
                                                Constraint<ModalTag>,
                                                Action,
                                                DurativeAction,
                                                Axiom,
                                                Metric,
                                                InitialFunctionValue,
                                                TimedInitialLiteral,
                                                Domain,
                                                Task>>;

/// Its symbols and relations (no family has rows in loki), named by what they hold (yggdrasil's Symbols and Relations),
/// not by their stores, and by classes of their own: a call with a repository argument then never instantiates the
/// stores (yggdrasil/formalism/declarations.hpp), and the names of everything over a Repository spell these, not every
/// one of its types (brokkr's template rule T20).
struct Symbols : boost::mp11::mp_apply<::ygg::formalism::Symbols, SymbolRepositoryTypes>
{
};

struct Relations : ::ygg::formalism::Relations<>
{
};

using Repository = ::ygg::formalism::Repository<Symbols, Relations>;

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
using DurativeActionView = EntityView<DurativeAction>;
using AxiomView = EntityView<Axiom>;
using MetricView = EntityView<Metric>;
using InitialFunctionValueView = EntityView<InitialFunctionValue>;
using TimedInitialLiteralView = EntityView<TimedInitialLiteral>;
using DomainView = EntityView<Domain>;
using TaskView = EntityView<Task>;

}

#endif
