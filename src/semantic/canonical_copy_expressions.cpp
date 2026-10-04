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

#include "loki/formalism/repository.hpp"
#include "loki/semantic/translator/canonical_copy_translator.hpp"
#include "loki/semantic/translator/common.hpp"

#include <utility>

namespace loki::semantic::detail
{

formalism::FunctionExpressionNumberView CanonicalCopyTranslator::copy(formalism::FunctionExpressionNumberView source)
{
    if (auto mapped = find_mapped(m_storage->numbers, source))
        return *mapped;
    auto data = formalism::checkout<formalism::FunctionExpressionNumber>(m_builder);
    data->value = source.get_value();
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->numbers, source, out);
    return out;
}

formalism::FunctionTermView CanonicalCopyTranslator::copy(formalism::FunctionTermView source)
{
    if (auto mapped = find_mapped(m_storage->function_terms, source))
        return *mapped;
    auto data = formalism::checkout<formalism::FunctionTerm>(m_builder);
    data->function = as_index(copy(source.get_function()));
    copy_list(source.get_terms(), data->terms);
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->function_terms, source, out);
    return out;
}

formalism::UnaryFunctionExpressionView CanonicalCopyTranslator::copy(formalism::UnaryFunctionExpressionView source)
{
    if (auto mapped = find_mapped(m_storage->unary_expressions, source))
        return *mapped;
    auto data = formalism::checkout<formalism::UnaryFunctionExpression>(m_builder);
    data->op = source.get_data().op;
    data->expression = as_index(copy(source.get_expression()));
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->unary_expressions, source, out);
    return out;
}

formalism::BinaryFunctionExpressionView CanonicalCopyTranslator::copy(formalism::BinaryFunctionExpressionView source)
{
    if (auto mapped = find_mapped(m_storage->binary_expressions, source))
        return *mapped;
    auto data = formalism::checkout<formalism::BinaryFunctionExpression>(m_builder);
    data->op = source.get_data().op;
    data->left = as_index(copy(source.get_left()));
    data->right = as_index(copy(source.get_right()));
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->binary_expressions, source, out);
    return out;
}

formalism::MultiFunctionExpressionView CanonicalCopyTranslator::copy(formalism::MultiFunctionExpressionView source)
{
    if (auto mapped = find_mapped(m_storage->multi_expressions, source))
        return *mapped;
    auto data = formalism::checkout<formalism::MultiFunctionExpression>(m_builder);
    data->op = source.get_operator();
    copy_list(source.get_args(), data->args);
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->multi_expressions, source, out);
    return out;
}

formalism::FunctionExpressionView CanonicalCopyTranslator::copy(formalism::FunctionExpressionView source)
{
    if (auto mapped = find_mapped(m_storage->function_expressions, source))
        return *mapped;
    auto data = formalism::checkout<formalism::FunctionExpression>(m_builder);
    data->variant = ygg::visit([&](const auto& arg) -> ygg::Data<formalism::FunctionExpression>::Variant { return as_index(copy(arg)); }, source.get_variant());
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->function_expressions, source, out);
    return out;
}

formalism::ConditionLiteralView CanonicalCopyTranslator::copy(formalism::ConditionLiteralView source)
{
    if (auto mapped = find_mapped(m_storage->condition_literals, source))
        return *mapped;
    auto data = formalism::checkout<formalism::ConditionLiteral>(m_builder);
    data->literal = as_index(copy(source.get_literal()));
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->condition_literals, source, out);
    return out;
}

formalism::ConditionAndView CanonicalCopyTranslator::copy(formalism::ConditionAndView source)
{
    if (auto mapped = find_mapped(m_storage->condition_ands, source))
        return *mapped;
    auto data = formalism::checkout<formalism::ConditionAnd>(m_builder);
    copy_list(source.get_conditions(), data->conditions);
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->condition_ands, source, out);
    return out;
}

formalism::ConditionOrView CanonicalCopyTranslator::copy(formalism::ConditionOrView source)
{
    if (auto mapped = find_mapped(m_storage->condition_ors, source))
        return *mapped;
    auto data = formalism::checkout<formalism::ConditionOr>(m_builder);
    copy_list(source.get_conditions(), data->conditions);
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->condition_ors, source, out);
    return out;
}

formalism::ConditionNotView CanonicalCopyTranslator::copy(formalism::ConditionNotView source)
{
    if (auto mapped = find_mapped(m_storage->condition_nots, source))
        return *mapped;
    auto data = formalism::checkout<formalism::ConditionNot>(m_builder);
    data->condition = as_index(copy(source.get_condition()));
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->condition_nots, source, out);
    return out;
}

formalism::ConditionImplyView CanonicalCopyTranslator::copy(formalism::ConditionImplyView source)
{
    if (auto mapped = find_mapped(m_storage->condition_implies, source))
        return *mapped;
    auto data = formalism::checkout<formalism::ConditionImply>(m_builder);
    data->left = as_index(copy(source.get_left()));
    data->right = as_index(copy(source.get_right()));
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->condition_implies, source, out);
    return out;
}

formalism::ConditionExistsView CanonicalCopyTranslator::copy(formalism::ConditionExistsView source)
{
    if (auto mapped = find_mapped(m_storage->condition_exists, source))
        return *mapped;
    auto data = formalism::checkout<formalism::ConditionExists>(m_builder);
    copy_list(source.get_parameters(), data->parameters);
    data->condition = as_index(copy(source.get_condition()));
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->condition_exists, source, out);
    return out;
}

formalism::ConditionForallView CanonicalCopyTranslator::copy(formalism::ConditionForallView source)
{
    if (auto mapped = find_mapped(m_storage->condition_foralls, source))
        return *mapped;
    auto data = formalism::checkout<formalism::ConditionForall>(m_builder);
    copy_list(source.get_parameters(), data->parameters);
    data->condition = as_index(copy(source.get_condition()));
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->condition_foralls, source, out);
    return out;
}

formalism::ConditionNumericConstraintView CanonicalCopyTranslator::copy(formalism::ConditionNumericConstraintView source)
{
    if (auto mapped = find_mapped(m_storage->condition_numeric_constraints, source))
        return *mapped;
    auto data = formalism::checkout<formalism::ConditionNumericConstraint>(m_builder);
    data->comparator = source.get_data().comparator;
    data->left = as_index(copy(source.get_left()));
    data->right = as_index(copy(source.get_right()));
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->condition_numeric_constraints, source, out);
    return out;
}

formalism::ConditionView CanonicalCopyTranslator::copy(formalism::ConditionView source)
{
    if (auto mapped = find_mapped(m_storage->conditions, source))
        return *mapped;
    auto data = formalism::checkout<formalism::Condition>(m_builder);
    data->variant = ygg::visit([&](const auto& arg) -> ygg::Data<formalism::Condition>::Variant { return as_index(copy(arg)); }, source.get_variant());
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->conditions, source, out);
    return out;
}

}  // namespace loki::semantic::detail
