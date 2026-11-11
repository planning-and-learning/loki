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

#include "mappings.hpp"

#include "ast_mappings.hpp"
#include "diagnostics.hpp"
#include "loki/ast/ast.hpp"
#include "loki/semantic/errors.hpp"

#include <algorithm>
#include <cctype>
#include <optional>
#include <string_view>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>

namespace loki::semantic
{

std::string lowercase(std::string text)
{
    for (auto& c : text)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return text;
}

std::string key(std::string text)
{
    if (!text.empty() && text.front() == ':')
        text.erase(text.begin());
    return text;
}

namespace
{
/// The enumerator whose name in names is name, by its position in the table.
template<typename E, std::size_t N>
std::optional<E> from_name(const std::array<const char*, N>& names, std::string_view name)
{
    const auto it = std::ranges::find_if(names, [&](std::string_view candidate) { return candidate == name; });
    if (it == names.end())
        return std::nullopt;
    return static_cast<E>(it - names.begin());
}
}  // namespace

formalism::RequirementKind requirement_kind(const ast::Requirement& node, const DiagnosticContext& diagnostics)
{
    const auto& name = node.name.text;
    if (const auto kind = from_name<formalism::RequirementKind>(formalism::requirement_kind_names, ":" + name))
        return *kind;
    diagnostics.throw_at(node.name, UnsupportedRequirementError(name));
}

urd::Store<urd::sequence<formalism::RequirementKind>> requirement_capabilities(formalism::RequirementKind kind)
{
    using enum formalism::RequirementKind;
    if (kind == QuantifiedPreconditions)
        return { QuantifiedPreconditions, ExistentialPreconditions, UniversalPreconditions };
    if (kind == Adl)
        return { Adl,
                 Strips,
                 Typing,
                 NegativePreconditions,
                 DisjunctivePreconditions,
                 Equality,
                 QuantifiedPreconditions,
                 ExistentialPreconditions,
                 UniversalPreconditions,
                 ConditionalEffects };
    if (kind == Fluents || kind == ActionCosts)
        return { kind, NumericFluents };
    return { kind };
}

urd::Store<urd::sequence<formalism::RequirementKind>> requirement_capabilities(const ast::Requirement& node, const DiagnosticContext& diagnostics)
{ return requirement_capabilities(requirement_kind(node, diagnostics)); }

formalism::BinaryComparator comparator(const ast::ConditionNumericConstraint& node, const DiagnosticContext& diagnostics)
{
    auto op = key(node.comparator);
    if (const auto comparator = from_name<formalism::BinaryComparator>(formalism::binary_comparator_names, op))
        return *comparator;
    diagnostics.throw_at(node, InvalidNumericConstraintError(op));
}

formalism::BinaryArithmeticOperator binary_operator(std::string op)
{
    op = key(std::move(op));
    if (op == "/")
        return formalism::BinaryArithmeticOperator::Div;
    return formalism::BinaryArithmeticOperator::Sub;
}

formalism::MultiArithmeticOperator multi_operator(std::string op)
{ return key(std::move(op)) == "*" ? formalism::MultiArithmeticOperator::Mul : formalism::MultiArithmeticOperator::Add; }

formalism::NumericEffectOperator numeric_effect_operator(const ast::EffectNumeric& node, const DiagnosticContext& diagnostics)
{
    auto op = key(node.op);
    if (const auto effect_operator = from_name<formalism::NumericEffectOperator>(formalism::numeric_effect_operator_names, op))
        return *effect_operator;
    diagnostics.throw_at(node, InvalidNumericEffectError(op));
}

}  // namespace loki::semantic
