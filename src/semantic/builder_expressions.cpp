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

#include "builder.hpp"
#include "checks.hpp"
#include "context.hpp"
#include "loki/ast/ast.hpp"
#include "loki/formalism/repository.hpp"
#include "mappings.hpp"

#include <utility>

namespace loki::semantic
{

formalism::TermView AstBuilder::parse_term(const ast::Term& node)
{
    auto value = node.variable ? ygg::Data<formalism::Term>::Variant(lookup_variable(m_parse_context, m_diagnostics, node.name).get_index()) :
                                 ygg::Data<formalism::Term>::Variant(lookup_object(m_domain_context, m_parse_context, m_diagnostics, node.name).get_index());
    auto data = formalism::checkout<formalism::Term>(m_builder);
    data->variant = std::move(value);
    return formalism::insert(repo(), *data).first;
}

formalism::AtomView AstBuilder::parse_atom(const ast::Atom& node)
{
    const auto name = key(node.predicate.text);
    auto pred = name == "=" ? equality_predicate(node.predicate, node.terms.size()) : predicate(node.predicate, node.terms.size());
    if (m_domain_context.declared_predicates.contains(name))
        checks().check_argument_types(name, pred.get_parameters(), node.terms, node.predicate);
    auto data = formalism::checkout<formalism::Atom>(m_builder);
    data->predicate = pred.get_index();
    for (const auto& term : node.terms)
        data->terms.push_back(parse_term(term).get_index());
    return formalism::insert(repo(), *data).first;
}

formalism::LiteralView AstBuilder::parse_literal(const ast::Literal& node)
{
    checks().mark_requirement_used(formalism::RequirementKind::Strips);
    const auto atom = parse_atom(node.atom);
    auto data = formalism::checkout<formalism::Literal>(m_builder);
    data->atom = atom.get_index();
    data->m_polarity = node.positive;
    return formalism::insert(repo(), *data).first;
}

formalism::ConditionView AstBuilder::parse_condition(const ast::Condition& condition)
{
    return boost::apply_visitor([&](const auto& node) { return parse_condition_node(node); }, condition);
}

formalism::ConditionView AstBuilder::wrap_condition(ygg::Data<formalism::Condition<>>::Variant value)
{
    auto data = formalism::checkout<formalism::Condition<>>(m_builder);
    data->variant = std::move(value);
    return formalism::insert(repo(), *data).first;
}

formalism::ConditionView AstBuilder::parse_condition_node(const ast::ConditionLiteral& node)
{
    const auto literal = parse_literal(node.literal);
    auto data = formalism::checkout<formalism::Condition<formalism::LiteralTag>>(m_builder);
    data->literal = literal.get_index();
    return wrap_condition(formalism::insert(repo(), *data).first.get_index());
}

formalism::ConditionView AstBuilder::parse_condition_node(const ast::ConditionAnd& node)
{
    auto data = formalism::checkout<formalism::Condition<formalism::AndTag>>(m_builder);
    for (const auto& child : node.conditions)
        data->conditions.push_back(parse_condition(child.get()).get_index());
    return wrap_condition(formalism::insert(repo(), *data).first.get_index());
}

formalism::ConditionView AstBuilder::parse_condition_node(const ast::ConditionOr& node)
{
    checks().require_requirement(formalism::RequirementKind::DisjunctivePreconditions, node);
    auto data = formalism::checkout<formalism::Condition<formalism::OrTag>>(m_builder);
    for (const auto& child : node.conditions)
        data->conditions.push_back(parse_condition(child.get()).get_index());
    return wrap_condition(formalism::insert(repo(), *data).first.get_index());
}

formalism::ConditionView AstBuilder::parse_condition_node(const ast::ConditionNot& node)
{
    checks().require_requirement(formalism::RequirementKind::NegativePreconditions, node);
    const auto condition = parse_condition(node.condition.get());
    auto data = formalism::checkout<formalism::Condition<formalism::NotTag>>(m_builder);
    data->condition = condition.get_index();
    return wrap_condition(formalism::insert(repo(), *data).first.get_index());
}

formalism::ConditionView AstBuilder::parse_condition_node(const ast::ConditionImply& node)
{
    checks().require_requirement(formalism::RequirementKind::DisjunctivePreconditions, node);
    const auto left = parse_condition(node.left.get());
    const auto right = parse_condition(node.right.get());
    auto data = formalism::checkout<formalism::Condition<formalism::ImplyTag>>(m_builder);
    data->left = left.get_index();
    data->right = right.get_index();
    return wrap_condition(formalism::insert(repo(), *data).first.get_index());
}

formalism::ConditionView AstBuilder::parse_condition_node(const ast::ConditionExists& node)
{
    checks().require_requirement(formalism::RequirementKind::ExistentialPreconditions, node);
    auto scope = VariableScope(m_parse_context);
    auto parameters = parse_parameters(node.parameters);
    auto child = parse_condition(node.condition.get());
    auto data = formalism::checkout<formalism::Condition<formalism::ExistsTag>>(m_builder);
    append_indices(parameters, data->parameters);
    data->condition = child.get_index();
    return wrap_condition(formalism::insert(repo(), *data).first.get_index());
}

formalism::ConditionView AstBuilder::parse_condition_node(const ast::ConditionForall& node)
{
    checks().require_requirement(formalism::RequirementKind::UniversalPreconditions, node);
    auto scope = VariableScope(m_parse_context);
    auto parameters = parse_parameters(node.parameters);
    auto child = parse_condition(node.condition.get());
    auto data = formalism::checkout<formalism::Condition<formalism::ForallTag>>(m_builder);
    append_indices(parameters, data->parameters);
    data->condition = child.get_index();
    return wrap_condition(formalism::insert(repo(), *data).first.get_index());
}

formalism::ConditionView AstBuilder::parse_condition_node(const ast::ConditionNumericConstraint& node)
{
    checks().require_requirement(formalism::RequirementKind::NumericFluents, node);
    const auto left = parse_function_expression(node.left.get());
    const auto right = parse_function_expression(node.right.get());
    auto data = formalism::checkout<formalism::Condition<formalism::NumericConstraintTag>>(m_builder);
    data->comparator = comparator(node, m_diagnostics);
    data->left = left.get_index();
    data->right = right.get_index();
    return wrap_condition(formalism::insert(repo(), *data).first.get_index());
}

formalism::FunctionTermView AstBuilder::parse_function_term(const ast::FunctionTerm& node)
{
    const auto name = key(node.function.text);
    if (m_parse_context.active_action_costs && name == "total-cost")
        checks().mark_requirement_used(formalism::RequirementKind::ActionCosts);
    else
        checks().require_requirement(formalism::RequirementKind::NumericFluents, node.function);
    auto skeleton = function(node.function, node.terms.size());
    if (m_domain_context.declared_functions.contains(name))
        checks().check_argument_types(name, skeleton.get_parameters(), node.terms, node.function);
    auto data = formalism::checkout<formalism::FunctionTerm>(m_builder);
    data->function = skeleton.get_index();
    for (const auto& term : node.terms)
        data->terms.push_back(parse_term(term).get_index());
    return formalism::insert(repo(), *data).first;
}

formalism::FunctionExpressionView AstBuilder::parse_function_expression(const ast::FunctionExpression& expression)
{
    return boost::apply_visitor([&](const auto& node) { return parse_function_expression_node(node); }, expression);
}

formalism::FunctionExpressionView AstBuilder::wrap_function_expression(ygg::Data<formalism::FunctionExpression<>>::Variant value)
{
    auto data = formalism::checkout<formalism::FunctionExpression<>>(m_builder);
    data->variant = std::move(value);
    return formalism::insert(repo(), *data).first;
}

formalism::FunctionExpressionView AstBuilder::parse_function_expression_node(const ast::FunctionExpressionNumber& node)
{
    auto data = formalism::checkout<formalism::FunctionExpression<formalism::NumberTag>>(m_builder);
    data->value = node.value;
    return wrap_function_expression(formalism::insert(repo(), *data).first.get_index());
}

formalism::FunctionExpressionView AstBuilder::parse_function_expression_node(const ast::FunctionExpressionFunction& node)
{
    return wrap_function_expression(parse_function_term(node.term).get_index());
}

formalism::FunctionExpressionView AstBuilder::parse_function_expression_node(const ast::FunctionExpressionUnary& node)
{
    const auto expression = parse_function_expression(node.expression.get());
    auto data = formalism::checkout<formalism::FunctionExpression<formalism::UnaryTag>>(m_builder);
    data->op = formalism::UnaryArithmeticOperator::Sub;
    data->expression = expression.get_index();
    return wrap_function_expression(formalism::insert(repo(), *data).first.get_index());
}

formalism::FunctionExpressionView AstBuilder::parse_function_expression_node(const ast::FunctionExpressionBinary& node)
{
    const auto left = parse_function_expression(node.left.get());
    const auto right = parse_function_expression(node.right.get());
    auto data = formalism::checkout<formalism::FunctionExpression<formalism::BinaryTag>>(m_builder);
    data->op = binary_operator(node.op);
    data->left = left.get_index();
    data->right = right.get_index();
    return wrap_function_expression(formalism::insert(repo(), *data).first.get_index());
}

formalism::FunctionExpressionView AstBuilder::parse_function_expression_node(const ast::FunctionExpressionMulti& node)
{
    const auto op = multi_operator(node.op);
    if (node.expressions.empty())
    {
        const auto unit = op == formalism::MultiArithmeticOperator::Add ? 0.0 : 1.0;
        auto data = formalism::checkout<formalism::FunctionExpression<formalism::NumberTag>>(m_builder);
        data->value = unit;
        return wrap_function_expression(formalism::insert(repo(), *data).first.get_index());
    }
    if (node.expressions.size() == 1)
        return parse_function_expression(node.expressions.front().get());

    auto data = formalism::checkout<formalism::FunctionExpression<formalism::MultiTag>>(m_builder);
    data->op = op;
    data->args.reserve(node.expressions.size());
    for (const auto& expression : node.expressions)
        data->args.push_back(parse_function_expression(expression.get()).get_index());
    return wrap_function_expression(formalism::insert(repo(), *data).first.get_index());
}

}  // namespace loki::semantic
