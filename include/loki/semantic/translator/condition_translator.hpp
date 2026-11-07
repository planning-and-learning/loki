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

#include <concepts>

namespace loki::semantic::detail
{

template<typename Derived>
class ConditionTranslator : public CopyTranslatorComponent<Derived, ConditionTranslator<Derived>>
{
public:
    explicit ConditionTranslator(CopyContext& context) : CopyTranslatorComponent<Derived, ConditionTranslator<Derived>>(context) {}

    /// A quantifier: its parameters scope its condition, which compiling typing prefixes with their type conditions.
    template<typename Tag>
        requires std::same_as<Tag, formalism::ExistsTag> || std::same_as<Tag, formalism::ForallTag>
    formalism::EntityView<formalism::Condition<Tag>> copy(formalism::EntityView<formalism::Condition<Tag>> source);

    formalism::ConditionView copy(formalism::ConditionView source);
};

template<typename Derived>
template<typename Tag>
    requires std::same_as<Tag, formalism::ExistsTag> || std::same_as<Tag, formalism::ForallTag>
formalism::EntityView<formalism::Condition<Tag>> ConditionTranslator<Derived>::copy(formalism::EntityView<formalism::Condition<Tag>> source)
{
    this->self().increment_quantifications(source.get_parameters());
    auto parameter_views = this->self().copy_parameter_views(source.get_parameters());
    this->self().enter_scope(parameter_views);
    const auto condition = this->self().copy(source.get_condition());
    this->self().leave_scope();
    auto typed_condition = condition;
    if (this->copy_context().phase == TranslationPhase::CompileTyping)
        this->self().prepend_type_conditions(typed_condition, source.get_parameters());
    auto data = formalism::checkout<formalism::Condition<Tag>>(this->copy_context().builder);
    if (this->self().compiles_typing_now())
        this->self().copy_parameters_without_types(source.get_parameters(), data->parameters);
    else
        for (auto parameter : parameter_views)
            data->parameters.push_back(parameter.index());
    data->condition = urd::to_stored(typed_condition);
    return formalism::insert(this->copy_context().storage->repository, *data).first;
}

template<typename Derived>
formalism::ConditionView ConditionTranslator<Derived>::copy(formalism::ConditionView source)
{
    switch (this->copy_context().phase)
    {
        case TranslationPhase::ToNegationNormalForm:
            return this->self().flatten_condition(std::visit([&](const auto& arg) { return this->self().copy_condition_node(arg); }, source));
        case TranslationPhase::RemoveUniversalQuantifiers:
            return this->self().remove_universal_quantifiers(source);
        case TranslationPhase::ToDisjunctiveNormalForm:
        {
            const auto copied = std::visit([&](const auto& arg) { return formalism::ConditionView(this->self().copy(arg)); }, source);
            return this->self().flatten_condition(this->self().to_dnf(copied));
        }
        default:
            return this->self().flatten_condition(
                std::visit([&](const auto& arg) { return formalism::ConditionView(this->self().copy(arg)); }, source));
    }
}

}  // namespace loki::semantic::detail

#endif
