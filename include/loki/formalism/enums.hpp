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

#ifndef LOKI_FORMALISM_ENUMS_HPP_
#define LOKI_FORMALISM_ENUMS_HPP_

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace loki::formalism
{

enum class BinaryComparator : std::uint8_t
{
    Eq,
    Ne,
    Lt,
    Le,
    Gt,
    Ge
};
enum class UnaryArithmeticOperator : std::uint8_t
{
    Sub
};
enum class BinaryArithmeticOperator : std::uint8_t
{
    Add,
    Sub,
    Mul,
    Div
};
enum class MultiArithmeticOperator : std::uint8_t
{
    Add,
    Mul
};
enum class NumericEffectOperator : std::uint8_t
{
    Assign,
    Increase,
    Decrease,
    ScaleUp,
    ScaleDown
};
enum class OptimizationDirection : std::uint8_t
{
    Minimize,
    Maximize,
};
/// When a timed condition or effect of a durative action holds or happens: at its start, at its end, or over the
/// open interval between them (conditions only).
/// The operator of a modal constraint: PDDL 3's ten, then the future, past and metric operators of :trajectory-formulas
/// (LTLf, PLTLf, MTLf over finite traces); a metric operator's interval is its numbers.
enum class ConstraintOperator : std::uint8_t
{
    AtEnd,
    Always,
    Sometime,
    Within,
    AtMostOnce,
    SometimeAfter,
    SometimeBefore,
    AlwaysWithin,
    HoldDuring,
    HoldAfter,
    Next,
    WeakNext,
    Until,
    Release,
    Yesterday,
    WeakYesterday,
    Since,
    Triggers,
    Once,
    Historically,
};
enum class TimeSpecifier : std::uint8_t
{
    Start,
    End,
    OverAll,
};

enum class RequirementKind : std::uint8_t
{
    Strips,
    Typing,
    NegativePreconditions,
    DisjunctivePreconditions,
    Equality,
    ExistentialPreconditions,
    UniversalPreconditions,
    QuantifiedPreconditions,
    Adl,
    ConditionalEffects,
    Fluents,
    NumericFluents,
    ActionCosts,
    DurativeActions,
    DerivedPredicates,
    NonDeterministic,
    ProbabilisticEffects,
    DurationInequalities,
    TimedInitialLiterals,
    Rewards,
    Preferences,
    Constraints,
    Mdp,
    TrajectoryFormulas,
};

// One name table per enum, indexed by the enumerator's value.
inline constexpr std::array<const char*, 1> unary_arithmetic_operator_names = { "-" };
inline constexpr std::array<const char*, 6> binary_comparator_names = { "=", "!=", "<", "<=", ">", ">=" };
inline constexpr std::array<const char*, 4> binary_arithmetic_operator_names = { "+", "-", "*", "/" };
inline constexpr std::array<const char*, 2> multi_arithmetic_operator_names = { "+", "*" };
inline constexpr std::array<const char*, 5> numeric_effect_operator_names = { "assign", "increase", "decrease", "scale-up", "scale-down" };
inline constexpr std::array<const char*, 2> optimization_direction_names = { "minimize", "maximize" };
inline constexpr std::array<const char*, 3> time_specifier_names = { "at start", "at end", "over all" };
inline constexpr std::array<const char*, 20> constraint_operator_names = {
    "at end", "always",    "sometime", "within",  "at-most-once", "sometime-after", "sometime-before", "always-within", "hold-during", "hold-after",
    "next",   "weak-next", "until",    "release", "yesterday",    "weak-yesterday", "since",           "triggers",      "once",        "historically",
};
inline constexpr std::array<const char*, 24> requirement_kind_names = {
    ":strips",
    ":typing",
    ":negative-preconditions",
    ":disjunctive-preconditions",
    ":equality",
    ":existential-preconditions",
    ":universal-preconditions",
    ":quantified-preconditions",
    ":adl",
    ":conditional-effects",
    ":fluents",
    ":numeric-fluents",
    ":action-costs",
    ":durative-actions",
    ":derived-predicates",
    ":non-deterministic",
    ":probabilistic-effects",
    ":duration-inequalities",
    ":timed-initial-literals",
    ":rewards",
    ":preferences",
    ":constraints",
    ":mdp",
    ":trajectory-formulas",
};

inline const char* to_string(BinaryComparator op) { return binary_comparator_names.at(static_cast<std::size_t>(op)); }

inline const char* to_string(BinaryArithmeticOperator op) { return binary_arithmetic_operator_names.at(static_cast<std::size_t>(op)); }

inline const char* to_string(MultiArithmeticOperator op) { return multi_arithmetic_operator_names.at(static_cast<std::size_t>(op)); }

inline const char* to_string(NumericEffectOperator op) { return numeric_effect_operator_names.at(static_cast<std::size_t>(op)); }

inline const char* to_string(OptimizationDirection direction) { return optimization_direction_names.at(static_cast<std::size_t>(direction)); }

inline const char* to_string(ConstraintOperator operation) { return constraint_operator_names.at(static_cast<std::size_t>(operation)); }

/// Whether the operator belongs to :trajectory-formulas rather than to PDDL 3.
inline bool is_trajectory_operator(ConstraintOperator operation) { return operation >= ConstraintOperator::Next; }

/// Whether the operator takes an interval (interval lo hi): the binary and the past operators but next and yesterday.
inline bool takes_interval(ConstraintOperator operation)
{
    using enum ConstraintOperator;
    return operation == Until || operation == Release || operation == Since || operation == Triggers || operation == Once || operation == Historically;
}

/// The number of operands of an operator.
inline std::size_t operand_count(ConstraintOperator operation)
{
    using enum ConstraintOperator;
    switch (operation)
    {
        case SometimeAfter:
        case SometimeBefore:
        case AlwaysWithin:
        case Until:
        case Release:
        case Since:
        case Triggers:
            return 2;
        default:
            return 1;
    }
}

inline const char* to_string(TimeSpecifier time) { return time_specifier_names.at(static_cast<std::size_t>(time)); }

inline const char* to_string(RequirementKind kind) { return requirement_kind_names.at(static_cast<std::size_t>(kind)); }

inline const char* to_string(UnaryArithmeticOperator op) { return unary_arithmetic_operator_names.at(static_cast<std::size_t>(op)); }

}  // namespace loki::formalism

#endif
