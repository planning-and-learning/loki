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

#ifndef LOKI_SEMANTIC_TRANSLATOR_CONDITION_TRANSLATOR_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_CONDITION_TRANSLATOR_HPP_

#include "loki/semantic/translator/copy_translator_component.hpp"

namespace loki::semantic::detail
{

template<typename Derived>
class ConditionTranslator : public CopyTranslatorComponent<Derived, ConditionTranslator<Derived>>
{
public:
    explicit ConditionTranslator(CopyContext& context) : CopyTranslatorComponent<Derived, ConditionTranslator<Derived>>(context) {}

    formalism::EntityView<formalism::Condition<formalism::LiteralTag>> copy(formalism::EntityView<formalism::Condition<formalism::LiteralTag>> source);

    formalism::EntityView<formalism::Condition<formalism::AndTag>> copy(formalism::EntityView<formalism::Condition<formalism::AndTag>> source);

    formalism::EntityView<formalism::Condition<formalism::OrTag>> copy(formalism::EntityView<formalism::Condition<formalism::OrTag>> source);

    formalism::EntityView<formalism::Condition<formalism::NotTag>> copy(formalism::EntityView<formalism::Condition<formalism::NotTag>> source);

    formalism::EntityView<formalism::Condition<formalism::ImplyTag>> copy(formalism::EntityView<formalism::Condition<formalism::ImplyTag>> source);

    formalism::EntityView<formalism::Condition<formalism::ExistsTag>> copy(formalism::EntityView<formalism::Condition<formalism::ExistsTag>> source);

    formalism::EntityView<formalism::Condition<formalism::ForallTag>> copy(formalism::EntityView<formalism::Condition<formalism::ForallTag>> source);

    formalism::EntityView<formalism::Condition<formalism::NumericConstraintTag>> copy(formalism::EntityView<formalism::Condition<formalism::NumericConstraintTag>> source);

    formalism::ConditionView copy(formalism::ConditionView source);
};

template<typename Derived>
formalism::EntityView<formalism::Condition<formalism::LiteralTag>> ConditionTranslator<Derived>::copy(formalism::EntityView<formalism::Condition<formalism::LiteralTag>> source)
{
    const auto literal = as_index(this->self().copy(source.get_literal()));
    auto data = formalism::checkout<formalism::Condition<formalism::LiteralTag>>(this->m_context.builder);
    data->literal = literal;
    return formalism::insert(this->m_context.storage->repository, *data).first;
}

template<typename Derived>
formalism::EntityView<formalism::Condition<formalism::AndTag>> ConditionTranslator<Derived>::copy(formalism::EntityView<formalism::Condition<formalism::AndTag>> source)
{
    auto data = formalism::checkout<formalism::Condition<formalism::AndTag>>(this->m_context.builder);
    for (auto condition : source.get_conditions())
        data->conditions.push_back(as_index(this->self().copy(condition)));
    return formalism::insert(this->m_context.storage->repository, *data).first;
}

template<typename Derived>
formalism::EntityView<formalism::Condition<formalism::OrTag>> ConditionTranslator<Derived>::copy(formalism::EntityView<formalism::Condition<formalism::OrTag>> source)
{
    auto data = formalism::checkout<formalism::Condition<formalism::OrTag>>(this->m_context.builder);
    for (auto condition : source.get_conditions())
        data->conditions.push_back(as_index(this->self().copy(condition)));
    return formalism::insert(this->m_context.storage->repository, *data).first;
}

template<typename Derived>
formalism::EntityView<formalism::Condition<formalism::NotTag>> ConditionTranslator<Derived>::copy(formalism::EntityView<formalism::Condition<formalism::NotTag>> source)
{
    const auto condition = as_index(this->self().copy(source.get_condition()));
    auto data = formalism::checkout<formalism::Condition<formalism::NotTag>>(this->m_context.builder);
    data->condition = condition;
    return formalism::insert(this->m_context.storage->repository, *data).first;
}

template<typename Derived>
formalism::EntityView<formalism::Condition<formalism::ImplyTag>> ConditionTranslator<Derived>::copy(formalism::EntityView<formalism::Condition<formalism::ImplyTag>> source)
{
    const auto left = as_index(this->self().copy(source.get_left()));
    const auto right = as_index(this->self().copy(source.get_right()));
    auto data = formalism::checkout<formalism::Condition<formalism::ImplyTag>>(this->m_context.builder);
    data->left = left;
    data->right = right;
    return formalism::insert(this->m_context.storage->repository, *data).first;
}

template<typename Derived>
formalism::EntityView<formalism::Condition<formalism::ExistsTag>> ConditionTranslator<Derived>::copy(formalism::EntityView<formalism::Condition<formalism::ExistsTag>> source)
{
    this->self().increment_quantifications(source.get_parameters());
    auto parameter_views = this->self().copy_parameter_views(source.get_parameters());
    this->self().enter_scope(parameter_views);
    const auto condition = this->self().copy(source.get_condition());
    this->self().leave_scope();
    auto typed_condition = condition;
    if (this->m_context.phase == TranslationPhase::CompileTyping)
        this->self().prepend_type_conditions(typed_condition, source.get_parameters());
    auto data = formalism::checkout<formalism::Condition<formalism::ExistsTag>>(this->m_context.builder);
    if (this->self().compiles_typing_now())
        this->self().copy_parameters_without_types(source.get_parameters(), data->parameters);
    else
        for (auto parameter : parameter_views)
            data->parameters.push_back(parameter.get_index());
    data->condition = typed_condition.get_index();
    return formalism::insert(this->m_context.storage->repository, *data).first;
}

template<typename Derived>
formalism::EntityView<formalism::Condition<formalism::ForallTag>> ConditionTranslator<Derived>::copy(formalism::EntityView<formalism::Condition<formalism::ForallTag>> source)
{
    this->self().increment_quantifications(source.get_parameters());
    auto parameter_views = this->self().copy_parameter_views(source.get_parameters());
    this->self().enter_scope(parameter_views);
    const auto condition = this->self().copy(source.get_condition());
    this->self().leave_scope();
    auto typed_condition = condition;
    if (this->m_context.phase == TranslationPhase::CompileTyping)
        this->self().prepend_type_conditions(typed_condition, source.get_parameters());
    auto data = formalism::checkout<formalism::Condition<formalism::ForallTag>>(this->m_context.builder);
    if (this->self().compiles_typing_now())
        this->self().copy_parameters_without_types(source.get_parameters(), data->parameters);
    else
        for (auto parameter : parameter_views)
            data->parameters.push_back(parameter.get_index());
    data->condition = typed_condition.get_index();
    return formalism::insert(this->m_context.storage->repository, *data).first;
}

template<typename Derived>
formalism::EntityView<formalism::Condition<formalism::NumericConstraintTag>> ConditionTranslator<Derived>::copy(formalism::EntityView<formalism::Condition<formalism::NumericConstraintTag>> source)
{
    const auto& data = source.get_data();
    const auto left = as_index(this->self().copy(source.get_left()));
    const auto right = as_index(this->self().copy(source.get_right()));
    auto result = formalism::checkout<formalism::Condition<formalism::NumericConstraintTag>>(this->m_context.builder);
    result->comparator = data.comparator;
    result->left = left;
    result->right = right;
    return formalism::insert(this->m_context.storage->repository, *result).first;
}

template<typename Derived>
formalism::ConditionView ConditionTranslator<Derived>::copy(formalism::ConditionView source)
{
    switch (this->m_context.phase)
    {
        case TranslationPhase::ToNegationNormalForm:
            return this->self().flatten_condition(ygg::visit([&](const auto& arg) { return this->self().copy_condition_node(arg); }, source.get_variant()));
        case TranslationPhase::RemoveUniversalQuantifiers:
            return this->self().remove_universal_quantifiers(source);
        case TranslationPhase::ToDisjunctiveNormalForm:
        {
            const auto copied = ygg::visit([&](const auto& arg) { return this->self().wrap_condition(this->self().copy(arg)); }, source.get_variant());
            return this->self().flatten_condition(this->self().to_dnf(copied));
        }
        default:
            return this->self().flatten_condition(
                ygg::visit([&](const auto& arg) { return this->self().wrap_condition(this->self().copy(arg)); }, source.get_variant()));
    }
}

}  // namespace loki::semantic::detail

#endif
