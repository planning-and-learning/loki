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

#include "copy_translator_component.hpp"
#include "loki/formalism/preferences.hpp"

#include <algorithm>
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

    /// The condition, a preference or a conjunction of preferences, with each preference's condition guarded by the type
    /// conditions of the parameters (CompileTypingTranslator::guard_with_types).
    formalism::ConditionView guard_preferences(formalism::ConditionView condition, formalism::EntityListView<formalism::Parameter> parameters);
};

template<typename Derived>
formalism::ConditionView ConditionTranslator<Derived>::guard_preferences(formalism::ConditionView condition,
                                                                         formalism::EntityListView<formalism::Parameter> parameters)
{
    auto& context = this->copy_context();
    if (const auto* conjunction = std::get_if<formalism::EntityView<formalism::Condition<formalism::AndTag>>>(&condition))
    {
        auto data = formalism::checkout<formalism::Condition<formalism::AndTag>>(context.builder);
        for (auto child : conjunction->get_conditions())
            this->self().append_conjunct(*data, this->self().guard_preferences(child, parameters));
        return this->self().make_conjunction(*data);
    }
    if (const auto* preference = std::get_if<formalism::EntityView<formalism::Condition<formalism::PreferenceTag>>>(&condition))
    {
        auto data = formalism::checkout<formalism::Condition<formalism::PreferenceTag>>(context.builder);
        data->name = preference->get_name();
        urd::store(data->condition, this->self().guard_with_types(preference->get_condition(), parameters));
        return formalism::ConditionView(context.storage->repository.insert(*data, context.storage->builder).first);
    }
    return condition;
}

template<typename Derived>
template<typename Tag>
    requires std::same_as<Tag, formalism::ExistsTag> || std::same_as<Tag, formalism::ForallTag>
formalism::EntityView<formalism::Condition<Tag>> ConditionTranslator<Derived>::copy(formalism::EntityView<formalism::Condition<Tag>> source)
{
    this->self().increment_quantifications(source.get_parameters());
    auto parameter_views = this->self().copy_parameter_views(source.get_parameters());
    this->self().enter_scope(parameter_views);
    const auto condition = this->self().copy(source.get_condition());
    auto typed_condition = condition;
    if (this->copy_context().phase == TranslationPhase::CompileTyping)
    {
        if constexpr (std::same_as<Tag, formalism::ForallTag>)
        {
            // A universal preference holds vacuously outside the types: each preference's condition is guarded, where typing
            // goes away (typed parameters restrict their bindings themselves, untyped ones (or object) need no guard).
            if (formalism::contains_preference(condition))
            {
                if (this->self().compiles_typing_now()
                    && std::ranges::any_of(source.get_parameters(),
                                           [](auto parameter)
                                           { return std::ranges::any_of(parameter.get_types(), [](auto type) { return !type.get_bases().empty(); }); }))
                    typed_condition = this->self().guard_preferences(condition, source.get_parameters());
            }
            else
                this->self().prepend_type_conditions(typed_condition, source.get_parameters());
        }
        else
        {
            this->self().prepend_type_conditions(typed_condition, source.get_parameters());
        }
    }
    // After typing: a guard's derived predicate takes the parameters in scope.
    this->self().leave_scope();
    auto data = formalism::checkout<formalism::Condition<Tag>>(this->copy_context().builder);
    if (this->self().compiles_typing_now())
        this->self().copy_parameters_without_types(source.get_parameters(), data->parameters);
    else
        for (auto parameter : parameter_views)
            data->parameters.push_back(parameter.index());
    urd::store(data->condition, typed_condition);
    return this->copy_context().storage->repository.insert(*data, this->copy_context().storage->builder).first;
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
            return this->self().flatten_condition(std::visit([&](const auto& arg) { return formalism::ConditionView(this->self().copy(arg)); }, source));
    }
}

}  // namespace loki::semantic::detail

#endif
