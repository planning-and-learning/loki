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

#ifndef LOKI_FORMALISM_FORMATTER_HPP_
#define LOKI_FORMALISM_FORMATTER_HPP_

#include "loki/formalism/conditions.hpp"
#include "loki/formalism/constraints.hpp"
#include "loki/formalism/declarations.hpp"
#include "loki/formalism/effects.hpp"
#include "loki/formalism/function_expressions.hpp"
#include "loki/formalism/term.hpp"

#include <fmt/format.h>
#include <string>
#include <yggdrasil/core/config.hpp>

#if !YGG_ENABLE_FMT_FORMATTERS
#error "loki requires yggdrasil's fmt formatters (YGG_ENABLE_FMT_FORMATTERS=1)."
#endif

namespace loki::formalism::format
{

std::string to_string(RequirementView value);
std::string to_string(TypeView value);
std::string to_string(ObjectView value);
std::string to_string(VariableView value);
std::string to_string(ParameterView value);
std::string to_string(PredicateView value);
std::string to_string(FunctionSkeletonView value);
std::string to_string(TermView value);
std::string to_string(AtomView value);
std::string to_string(LiteralView value);
std::string to_string(EntityView<FunctionExpression<NumberTag>> value);
std::string to_string(FunctionTermView value);
std::string to_string(EntityView<FunctionExpression<UnaryTag>> value);
std::string to_string(EntityView<FunctionExpression<BinaryTag>> value);
std::string to_string(EntityView<FunctionExpression<MultiTag>> value);
std::string to_string(EntityView<FunctionExpression<DurationTag>> value);
std::string to_string(EntityView<FunctionExpression<IsViolatedTag>> value);
std::string to_string(FunctionExpressionView value);
std::string to_string(EntityView<Condition<LiteralTag>> value);
std::string to_string(EntityView<Condition<AndTag>> value);
std::string to_string(EntityView<Condition<OrTag>> value);
std::string to_string(EntityView<Condition<NotTag>> value);
std::string to_string(EntityView<Condition<ImplyTag>> value);
std::string to_string(EntityView<Condition<ExistsTag>> value);
std::string to_string(EntityView<Condition<ForallTag>> value);
std::string to_string(EntityView<Condition<NumericConstraintTag>> value);
std::string to_string(EntityView<Condition<TimedTag>> value);
std::string to_string(EntityView<Condition<PreferenceTag>> value);
std::string to_string(ConditionView value);
std::string to_string(EntityView<Effect<LiteralTag>> value);
std::string to_string(EntityView<Effect<AndTag>> value);
std::string to_string(EntityView<Effect<NumericTag>> value);
std::string to_string(EntityView<Effect<ForallTag>> value);
std::string to_string(EntityView<Effect<WhenTag>> value);
std::string to_string(EntityView<Effect<OneOfTag>> value);
std::string to_string(EffectProbabilisticAlternativeView value);
std::string to_string(EntityView<Effect<ProbabilisticTag>> value);
std::string to_string(EntityView<Effect<TimedTag>> value);
std::string to_string(EffectView value);
std::string to_string(EntityView<Constraint<StateTag>> value);
std::string to_string(EntityView<Constraint<AndTag>> value);
std::string to_string(EntityView<Constraint<OrTag>> value);
std::string to_string(EntityView<Constraint<NotTag>> value);
std::string to_string(EntityView<Constraint<ExistsTag>> value);
std::string to_string(EntityView<Constraint<ForallTag>> value);
std::string to_string(EntityView<Constraint<PreferenceTag>> value);
std::string to_string(EntityView<Constraint<ModalTag>> value);
std::string to_string(ConstraintView value);
std::string to_string(ActionView value);
std::string to_string(DurativeActionView value);
std::string to_string(AxiomView value);
std::string to_string(MetricView value);
std::string to_string(InitialFunctionValueView value);
std::string to_string(TimedInitialLiteralView value);
std::string to_string(DomainView value);
std::string to_string(TaskView value);

}  // namespace loki::formalism::format

namespace fmt
{

template<typename T>
struct formatter<urd::index_view<T, loki::formalism::Repository>, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const urd::index_view<T, loki::formalism::Repository>& value, FormatContext& ctx) const
    { return fmt::format_to(ctx.out(), "{}", loki::formalism::format::to_string(value)); }
};

}  // namespace fmt

#endif
