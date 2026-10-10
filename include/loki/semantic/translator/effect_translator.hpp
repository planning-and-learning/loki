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

#include "loki/semantic/translator/copy_translator_component.hpp"

#include <optional>

namespace loki::semantic::detail
{

template<typename Derived>
class EffectTranslator : public CopyTranslatorComponent<Derived, EffectTranslator<Derived>>
{
public:
    explicit EffectTranslator(CopyContext& context) : CopyTranslatorComponent<Derived, EffectTranslator<Derived>>(context) {}

    formalism::EntityView<formalism::Effect<formalism::LiteralTag>> copy(formalism::EntityView<formalism::Effect<formalism::LiteralTag>> source);

    formalism::EntityView<formalism::Effect<formalism::AndTag>> copy(formalism::EntityView<formalism::Effect<formalism::AndTag>> source);

    formalism::EntityView<formalism::Effect<formalism::NumericTag>> copy(formalism::EntityView<formalism::Effect<formalism::NumericTag>> source);

    formalism::EntityView<formalism::Effect<formalism::ForallTag>> copy(formalism::EntityView<formalism::Effect<formalism::ForallTag>> source);

    formalism::EntityView<formalism::Effect<formalism::WhenTag>> copy(formalism::EntityView<formalism::Effect<formalism::WhenTag>> source);

    formalism::EntityView<formalism::Effect<formalism::OneOfTag>> copy(formalism::EntityView<formalism::Effect<formalism::OneOfTag>> source);

    formalism::EffectProbabilisticAlternativeView copy(formalism::EffectProbabilisticAlternativeView source);

    formalism::EntityView<formalism::Effect<formalism::ProbabilisticTag>> copy(formalism::EntityView<formalism::Effect<formalism::ProbabilisticTag>> source);

    formalism::EffectView copy(formalism::EffectView source);
};

template<typename Derived>
formalism::EntityView<formalism::Effect<formalism::LiteralTag>> EffectTranslator<Derived>::copy(formalism::EntityView<formalism::Effect<formalism::LiteralTag>> source)
{
    const auto literal = as_index(this->self().copy(source.get_literal()));
    auto data = formalism::checkout<formalism::Effect<formalism::LiteralTag>>(this->m_context.builder);
    data->literal = literal;
    return formalism::insert(this->m_context.storage->repository, *data).first;
}

template<typename Derived>
formalism::EntityView<formalism::Effect<formalism::AndTag>> EffectTranslator<Derived>::copy(formalism::EntityView<formalism::Effect<formalism::AndTag>> source)
{
    auto data = formalism::checkout<formalism::Effect<formalism::AndTag>>(this->m_context.builder);
    for (auto effect : source.get_effects())
        data->effects.push_back(as_index(this->self().copy(effect)));
    return formalism::insert(this->m_context.storage->repository, *data).first;
}

template<typename Derived>
formalism::EntityView<formalism::Effect<formalism::NumericTag>> EffectTranslator<Derived>::copy(formalism::EntityView<formalism::Effect<formalism::NumericTag>> source)
{
    const auto& data = source.get_data();
    const auto function = as_index(this->self().copy(source.get_function()));
    const auto expression = as_index(this->self().copy(source.get_expression()));
    auto result = formalism::checkout<formalism::Effect<formalism::NumericTag>>(this->m_context.builder);
    result->op = data.op;
    result->function = function;
    result->expression = expression;
    return formalism::insert(this->m_context.storage->repository, *result).first;
}

template<typename Derived>
formalism::EntityView<formalism::Effect<formalism::ForallTag>> EffectTranslator<Derived>::copy(formalism::EntityView<formalism::Effect<formalism::ForallTag>> source)
{
    this->self().increment_quantifications(source.get_parameters());
    auto parameter_views = this->self().copy_parameter_views(source.get_parameters());
    this->self().enter_scope(parameter_views);
    auto effect = as_index(this->self().copy(source.get_effect()));
    if (this->m_context.phase == TranslationPhase::CompileTyping)
    {
        auto condition_data = formalism::checkout<formalism::Condition<formalism::AndTag>>(this->m_context.builder);
        this->self().type_conditions_for_parameters(source.get_parameters(), *condition_data);
        const auto condition = this->self().make_conjunction(*condition_data);
        auto data = formalism::checkout<formalism::Effect<formalism::WhenTag>>(this->m_context.builder);
        data->condition = condition.get_index();
        data->effect = effect;
        effect = this->self().wrap_effect(formalism::insert(this->m_context.storage->repository, *data).first).get_index();
    }
    auto data = formalism::checkout<formalism::Effect<formalism::ForallTag>>(this->m_context.builder);
    if (this->self().compiles_typing_now())
        this->self().copy_parameters_without_types(source.get_parameters(), data->parameters);
    else
        for (auto parameter : parameter_views)
            data->parameters.push_back(parameter.get_index());
    data->effect = effect;
    const auto out = formalism::insert(this->m_context.storage->repository, *data).first;
    this->self().leave_scope();
    return out;
}

template<typename Derived>
formalism::EntityView<formalism::Effect<formalism::WhenTag>> EffectTranslator<Derived>::copy(formalism::EntityView<formalism::Effect<formalism::WhenTag>> source)
{
    // Sequence the child copies: argument evaluation order is unspecified, and both children
    // may pull from the generated-name counter (compiler-independent output requires a fixed order).
    const auto condition = as_index(this->self().copy(source.get_condition()));
    const auto effect = as_index(this->self().copy(source.get_effect()));
    auto data = formalism::checkout<formalism::Effect<formalism::WhenTag>>(this->m_context.builder);
    data->condition = condition;
    data->effect = effect;
    return formalism::insert(this->m_context.storage->repository, *data).first;
}

template<typename Derived>
formalism::EntityView<formalism::Effect<formalism::OneOfTag>> EffectTranslator<Derived>::copy(formalism::EntityView<formalism::Effect<formalism::OneOfTag>> source)
{
    auto data = formalism::checkout<formalism::Effect<formalism::OneOfTag>>(this->m_context.builder);
    for (auto effect : source.get_effects())
        data->effects.push_back(as_index(this->self().copy(effect)));
    return formalism::insert(this->m_context.storage->repository, *data).first;
}

template<typename Derived>
formalism::EffectProbabilisticAlternativeView EffectTranslator<Derived>::copy(formalism::EffectProbabilisticAlternativeView source)
{
    const auto& data = source.get_data();
    const auto effect = as_index(this->self().copy(source.get_effect()));
    auto result = formalism::checkout<formalism::EffectProbabilisticAlternative>(this->m_context.builder);
    result->probability = data.probability;
    result->effect = effect;
    return formalism::insert(this->m_context.storage->repository, *result).first;
}

template<typename Derived>
formalism::EntityView<formalism::Effect<formalism::ProbabilisticTag>> EffectTranslator<Derived>::copy(formalism::EntityView<formalism::Effect<formalism::ProbabilisticTag>> source)
{
    auto data = formalism::checkout<formalism::Effect<formalism::ProbabilisticTag>>(this->m_context.builder);
    for (auto alternative : source.get_alternatives())
        data->alternatives.push_back(as_index(this->self().copy(alternative)));
    return formalism::insert(this->m_context.storage->repository, *data).first;
}

template<typename Derived>
formalism::EffectView EffectTranslator<Derived>::copy(formalism::EffectView source)
{
    if (this->m_context.phase == TranslationPhase::SplitDisjunctiveConditions)
    {
        auto split = std::optional<formalism::EffectView> {};
        ygg::visit(
            [&](const auto& arg)
            {
                using Node = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::WhenTag>>>)
                {
                    const auto condition = this->self().copy(arg.get_condition());
                    const auto effect = as_index(this->self().copy(arg.get_effect()));
                    if (const auto condition_or = this->self().as_or(condition))
                    {
                        auto data = formalism::checkout<formalism::Effect<formalism::AndTag>>(this->m_context.builder);
                        for (auto part : condition_or->get_conditions())
                        {
                            auto when_data = formalism::checkout<formalism::Effect<formalism::WhenTag>>(this->m_context.builder);
                            when_data->condition = part.get_index();
                            when_data->effect = effect;
                            const auto when = formalism::insert(this->m_context.storage->repository, *when_data).first;
                            data->effects.push_back(this->self().wrap_effect(when).get_index());
                        }
                        split = this->self().wrap_effect(formalism::insert(this->m_context.storage->repository, *data).first);
                    }
                    else
                    {
                        auto data = formalism::checkout<formalism::Effect<formalism::WhenTag>>(this->m_context.builder);
                        data->condition = condition.get_index();
                        data->effect = effect;
                        split = this->self().wrap_effect(formalism::insert(this->m_context.storage->repository, *data).first);
                    }
                }
            },
            source.get_variant());
        if (split)
            return *split;
    }

    auto value = ygg::visit([&](const auto& arg) -> ygg::Data<formalism::Effect<>>::Variant
                            { return ygg::Data<formalism::Effect<>>::Variant(as_index(this->self().copy(arg))); },
                            source.get_variant());
    auto data = formalism::checkout<formalism::Effect<>>(this->m_context.builder);
    data->variant = std::move(value);
    auto copied = formalism::insert(this->m_context.storage->repository, *data).first;
    if (this->m_context.phase == TranslationPhase::ToEffectNormalForm)
        return this->self().normalize_effect(copied);
    return copied;
}

}  // namespace loki::semantic::detail

#endif
