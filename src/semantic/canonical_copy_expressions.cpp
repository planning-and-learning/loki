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

formalism::EntityView<formalism::FunctionExpression<formalism::NumberTag>>
CanonicalCopyTranslator::copy(formalism::EntityView<formalism::FunctionExpression<formalism::NumberTag>> source)
{
    if (auto mapped = find_mapped(m_storage->maps, source))
        return *mapped;
    auto data = formalism::checkout<formalism::FunctionExpression<formalism::NumberTag>>(m_builder);
    data->value = source.get_value();
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->maps, source, out);
    return out;
}

formalism::FunctionTermView CanonicalCopyTranslator::copy(formalism::FunctionTermView source)
{
    if (auto mapped = find_mapped(m_storage->maps, source))
        return *mapped;
    auto data = formalism::checkout<formalism::FunctionTerm>(m_builder);
    data->function = urd::to_stored(copy(source.get_function()));
    copy_list(source.get_terms(), data->terms);
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->maps, source, out);
    return out;
}

formalism::EntityView<formalism::FunctionExpression<formalism::UnaryTag>>
CanonicalCopyTranslator::copy(formalism::EntityView<formalism::FunctionExpression<formalism::UnaryTag>> source)
{
    if (auto mapped = find_mapped(m_storage->maps, source))
        return *mapped;
    auto data = formalism::checkout<formalism::FunctionExpression<formalism::UnaryTag>>(m_builder);
    data->op = source.data().op;
    data->expression = urd::to_stored(copy(source.get_expression()));
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->maps, source, out);
    return out;
}

formalism::EntityView<formalism::FunctionExpression<formalism::BinaryTag>>
CanonicalCopyTranslator::copy(formalism::EntityView<formalism::FunctionExpression<formalism::BinaryTag>> source)
{
    if (auto mapped = find_mapped(m_storage->maps, source))
        return *mapped;
    auto data = formalism::checkout<formalism::FunctionExpression<formalism::BinaryTag>>(m_builder);
    data->op = source.data().op;
    data->left = urd::to_stored(copy(source.get_left()));
    data->right = urd::to_stored(copy(source.get_right()));
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->maps, source, out);
    return out;
}

formalism::EntityView<formalism::FunctionExpression<formalism::MultiTag>>
CanonicalCopyTranslator::copy(formalism::EntityView<formalism::FunctionExpression<formalism::MultiTag>> source)
{
    if (auto mapped = find_mapped(m_storage->maps, source))
        return *mapped;
    auto data = formalism::checkout<formalism::FunctionExpression<formalism::MultiTag>>(m_builder);
    data->op = source.get_operator();
    copy_list(source.get_args(), data->args);
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->maps, source, out);
    return out;
}

formalism::FunctionExpressionView CanonicalCopyTranslator::copy(formalism::FunctionExpressionView source)
{
    return std::visit([&](const auto& arg) -> formalism::FunctionExpressionView { return copy(arg); }, source);
}

formalism::EntityView<formalism::Condition<formalism::LiteralTag>>
CanonicalCopyTranslator::copy(formalism::EntityView<formalism::Condition<formalism::LiteralTag>> source)
{
    if (auto mapped = find_mapped(m_storage->maps, source))
        return *mapped;
    auto data = formalism::checkout<formalism::Condition<formalism::LiteralTag>>(m_builder);
    data->literal = urd::to_stored(copy(source.get_literal()));
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->maps, source, out);
    return out;
}

formalism::EntityView<formalism::Condition<formalism::AndTag>>
CanonicalCopyTranslator::copy(formalism::EntityView<formalism::Condition<formalism::AndTag>> source)
{
    if (auto mapped = find_mapped(m_storage->maps, source))
        return *mapped;
    auto data = formalism::checkout<formalism::Condition<formalism::AndTag>>(m_builder);
    copy_list(source.get_conditions(), data->conditions);
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->maps, source, out);
    return out;
}

formalism::EntityView<formalism::Condition<formalism::OrTag>>
CanonicalCopyTranslator::copy(formalism::EntityView<formalism::Condition<formalism::OrTag>> source)
{
    if (auto mapped = find_mapped(m_storage->maps, source))
        return *mapped;
    auto data = formalism::checkout<formalism::Condition<formalism::OrTag>>(m_builder);
    copy_list(source.get_conditions(), data->conditions);
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->maps, source, out);
    return out;
}

formalism::EntityView<formalism::Condition<formalism::NotTag>>
CanonicalCopyTranslator::copy(formalism::EntityView<formalism::Condition<formalism::NotTag>> source)
{
    if (auto mapped = find_mapped(m_storage->maps, source))
        return *mapped;
    auto data = formalism::checkout<formalism::Condition<formalism::NotTag>>(m_builder);
    data->condition = urd::to_stored(copy(source.get_condition()));
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->maps, source, out);
    return out;
}

formalism::EntityView<formalism::Condition<formalism::ImplyTag>>
CanonicalCopyTranslator::copy(formalism::EntityView<formalism::Condition<formalism::ImplyTag>> source)
{
    if (auto mapped = find_mapped(m_storage->maps, source))
        return *mapped;
    auto data = formalism::checkout<formalism::Condition<formalism::ImplyTag>>(m_builder);
    data->left = urd::to_stored(copy(source.get_left()));
    data->right = urd::to_stored(copy(source.get_right()));
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->maps, source, out);
    return out;
}

formalism::EntityView<formalism::Condition<formalism::ExistsTag>>
CanonicalCopyTranslator::copy(formalism::EntityView<formalism::Condition<formalism::ExistsTag>> source)
{
    if (auto mapped = find_mapped(m_storage->maps, source))
        return *mapped;
    auto data = formalism::checkout<formalism::Condition<formalism::ExistsTag>>(m_builder);
    copy_list(source.get_parameters(), data->parameters);
    data->condition = urd::to_stored(copy(source.get_condition()));
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->maps, source, out);
    return out;
}

formalism::EntityView<formalism::Condition<formalism::ForallTag>>
CanonicalCopyTranslator::copy(formalism::EntityView<formalism::Condition<formalism::ForallTag>> source)
{
    if (auto mapped = find_mapped(m_storage->maps, source))
        return *mapped;
    auto data = formalism::checkout<formalism::Condition<formalism::ForallTag>>(m_builder);
    copy_list(source.get_parameters(), data->parameters);
    data->condition = urd::to_stored(copy(source.get_condition()));
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->maps, source, out);
    return out;
}

formalism::EntityView<formalism::Condition<formalism::NumericConstraintTag>>
CanonicalCopyTranslator::copy(formalism::EntityView<formalism::Condition<formalism::NumericConstraintTag>> source)
{
    if (auto mapped = find_mapped(m_storage->maps, source))
        return *mapped;
    auto data = formalism::checkout<formalism::Condition<formalism::NumericConstraintTag>>(m_builder);
    data->comparator = source.data().comparator;
    data->left = urd::to_stored(copy(source.get_left()));
    data->right = urd::to_stored(copy(source.get_right()));
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->maps, source, out);
    return out;
}

formalism::ConditionView CanonicalCopyTranslator::copy(formalism::ConditionView source)
{
    return std::visit([&](const auto& arg) -> formalism::ConditionView { return copy(arg); }, source);
}

}  // namespace loki::semantic::detail
