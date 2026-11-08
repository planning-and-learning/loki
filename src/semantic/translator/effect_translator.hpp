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

#ifndef LOKI_SEMANTIC_TRANSLATOR_EFFECT_TRANSLATOR_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_EFFECT_TRANSLATOR_HPP_

#include "copy_translator_component.hpp"

#include <optional>

namespace loki::semantic::detail
{

template<typename Derived>
class EffectTranslator : public CopyTranslatorComponent<Derived, EffectTranslator<Derived>>
{
public:
    explicit EffectTranslator(CopyContext& context) : CopyTranslatorComponent<Derived, EffectTranslator<Derived>>(context) {}

    formalism::EntityView<formalism::Effect<formalism::ForallTag>> copy(formalism::EntityView<formalism::Effect<formalism::ForallTag>> source);

    formalism::EffectView copy(formalism::EffectView source);
};

template<typename Derived>
formalism::EntityView<formalism::Effect<formalism::ForallTag>>
EffectTranslator<Derived>::copy(formalism::EntityView<formalism::Effect<formalism::ForallTag>> source)
{
    this->self().increment_quantifications(source.get_parameters());
    auto parameter_views = this->self().copy_parameter_views(source.get_parameters());
    this->self().enter_scope(parameter_views);
    auto effect = urd::to_stored(this->self().copy(source.get_effect()));
    if (this->copy_context().phase == TranslationPhase::CompileTyping)
    {
        auto condition_data = formalism::checkout<formalism::Condition<formalism::AndTag>>(this->copy_context().builder);
        this->self().type_conditions_for_parameters(source.get_parameters(), *condition_data);
        const auto condition = this->self().make_conjunction(*condition_data);
        auto data = formalism::checkout<formalism::Effect<formalism::WhenTag>>(this->copy_context().builder);
        data->condition = urd::to_stored(condition);
        data->effect = effect;
        effect = formalism::insert(this->copy_context().storage->repository, *data).first.index();
    }
    auto data = formalism::checkout<formalism::Effect<formalism::ForallTag>>(this->copy_context().builder);
    if (this->self().compiles_typing_now())
        this->self().copy_parameters_without_types(source.get_parameters(), data->parameters);
    else
        for (auto parameter : parameter_views)
            data->parameters.push_back(parameter.index());
    data->effect = effect;
    const auto out = formalism::insert(this->copy_context().storage->repository, *data).first;
    this->self().leave_scope();
    return out;
}

template<typename Derived>
formalism::EffectView EffectTranslator<Derived>::copy(formalism::EffectView source)
{
    const auto* when = std::get_if<formalism::EntityView<formalism::Effect<formalism::WhenTag>>>(&source);
    if (this->copy_context().phase == TranslationPhase::SplitDisjunctiveConditions && when)
    {
        const auto condition = this->self().copy(when->get_condition());
        const auto effect = urd::to_stored(this->self().copy(when->get_effect()));
        if (const auto condition_or = this->self().as_or(condition))
        {
            auto data = formalism::checkout<formalism::Effect<formalism::AndTag>>(this->copy_context().builder);
            for (auto part : condition_or->get_conditions())
            {
                auto when_data = formalism::checkout<formalism::Effect<formalism::WhenTag>>(this->copy_context().builder);
                when_data->condition = urd::to_stored(part);
                when_data->effect = effect;
                const auto split = formalism::insert(this->copy_context().storage->repository, *when_data).first;
                data->effects.push_back(split.index());
            }
            return formalism::insert(this->copy_context().storage->repository, *data).first;
        }
        auto data = formalism::checkout<formalism::Effect<formalism::WhenTag>>(this->copy_context().builder);
        data->condition = urd::to_stored(condition);
        data->effect = effect;
        return formalism::insert(this->copy_context().storage->repository, *data).first;
    }

    const auto copied = std::visit([&](const auto& arg) -> formalism::EffectView { return this->self().copy(arg); }, source);
    if (this->copy_context().phase == TranslationPhase::ToEffectNormalForm)
        return this->self().normalize_effect(copied);
    return copied;
}

}  // namespace loki::semantic::detail

#endif
