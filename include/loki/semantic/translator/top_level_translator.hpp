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

#ifndef LOKI_SEMANTIC_TRANSLATOR_TOP_LEVEL_TRANSLATOR_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_TOP_LEVEL_TRANSLATOR_HPP_

#include "loki/semantic/translator/copy_translator_component.hpp"

namespace loki::semantic::detail
{

template<typename Derived>
class TopLevelTranslator : public CopyTranslatorComponent<Derived, TopLevelTranslator<Derived>>
{
public:
    explicit TopLevelTranslator(CopyContext& context) : CopyTranslatorComponent<Derived, TopLevelTranslator<Derived>>(context) {}

    formalism::ActionView copy(formalism::ActionView source);

    formalism::AxiomView copy(formalism::AxiomView source);

    formalism::MetricView copy(formalism::MetricView source);

    formalism::InitialFunctionValueView copy(formalism::InitialFunctionValueView source);
};

template<typename Derived>
formalism::ActionView TopLevelTranslator<Derived>::copy(formalism::ActionView source)
{
    if (this->m_context.phase == TranslationPhase::RenameQuantifiedVariables && this->m_context.renaming_enabled)
    {
        const auto renamed = this->self().rename_action_variables(source);
        const auto previous = this->m_context.renaming_enabled;
        this->m_context.renaming_enabled = false;
        auto out = this->self().copy(renamed);
        this->m_context.renaming_enabled = previous;
        remember(this->m_context.storage->actions, source, out);
        return out;
    }

    if (auto mapped = find_mapped(this->m_context.storage->actions, source))
        return *mapped;
    const auto& data = source.get_data();
    this->m_context.num_quantifications.clear();
    this->self().increment_quantifications(source.get_parameters());
    auto parameter_views = this->self().copy_parameter_views(source.get_parameters());
    this->self().enter_scope(parameter_views);
    auto precondition = std::optional<formalism::ConditionView> {};
    if (const auto condition = source.get_precondition())
    {
        auto copied_condition = this->self().copy(condition.value());
        if (this->m_context.phase == TranslationPhase::MoveExistentialQuantifiers)
        {
            auto effect = std::optional<formalism::EffectView> {};
            if (const auto effect_view = source.get_effect())
                effect = effect_view.value();
            copied_condition = this->self().lift_top_level_exists(parameter_views, copied_condition, effect);
        }
        precondition = copied_condition;
    }
    if (this->m_context.phase == TranslationPhase::CompileTyping)
        this->self().prepend_type_conditions(precondition, source.get_parameters());
    auto effect = cista::optional<ygg::Index<formalism::Effect<>>> {};
    if (const auto effect_view = source.get_effect())
        effect = as_index(this->self().copy(effect_view.value()));
    auto result = formalism::checkout<formalism::Action>(this->m_context.builder);
    result->name = data.name;
    result->original_name = data.original_name;
    if (this->self().compiles_typing_now())
        this->self().copy_parameters_without_types(source.get_parameters(), result->parameters);
    else
        for (auto parameter : parameter_views)
            result->parameters.push_back(parameter.get_index());
    result->original_arity = data.original_arity;
    if (precondition)
        result->precondition = precondition->get_index();
    result->effect = effect;
    auto out = formalism::insert(this->m_context.storage->repository, *result).first;
    this->self().leave_scope();
    remember(this->m_context.storage->actions, source, out);
    return out;
}

template<typename Derived>
formalism::AxiomView TopLevelTranslator<Derived>::copy(formalism::AxiomView source)
{
    if (this->m_context.phase == TranslationPhase::RenameQuantifiedVariables && this->m_context.renaming_enabled)
    {
        const auto renamed = this->self().rename_axiom_variables(source);
        const auto previous = this->m_context.renaming_enabled;
        this->m_context.renaming_enabled = false;
        auto out = this->self().copy(renamed);
        this->m_context.renaming_enabled = previous;
        remember(this->m_context.storage->axioms, source, out);
        return out;
    }

    if (auto mapped = find_mapped(this->m_context.storage->axioms, source))
        return *mapped;
    const auto& data = source.get_data();
    this->m_context.num_quantifications.clear();
    this->self().increment_quantifications(source.get_parameters());
    auto parameter_views = this->self().copy_parameter_views(source.get_parameters());
    this->self().enter_scope(parameter_views);
    auto copied_condition = this->self().copy(source.get_condition());
    if (this->m_context.phase == TranslationPhase::MoveExistentialQuantifiers)
        copied_condition = this->self().lift_top_level_exists(parameter_views, copied_condition, std::nullopt);
    if (this->m_context.phase == TranslationPhase::CompileTyping)
        this->self().prepend_type_conditions(copied_condition, source.get_parameters());
    const auto head = as_index(this->self().copy(source.get_head()));
    auto result = formalism::checkout<formalism::Axiom>(this->m_context.builder);
    if (this->self().compiles_typing_now())
        this->self().copy_parameters_without_types(source.get_parameters(), result->parameters);
    else
        for (auto parameter : parameter_views)
            result->parameters.push_back(parameter.get_index());
    result->original_arity = data.original_arity;
    result->head = head;
    result->condition = copied_condition.get_index();
    auto out = formalism::insert(this->m_context.storage->repository, *result).first;
    this->self().leave_scope();
    remember(this->m_context.storage->axioms, source, out);
    return out;
}

template<typename Derived>
formalism::MetricView TopLevelTranslator<Derived>::copy(formalism::MetricView source)
{
    if (auto mapped = find_mapped(this->m_context.storage->metrics, source))
        return *mapped;
    const auto expression = as_index(this->self().copy(source.get_expression()));
    auto data = formalism::checkout<formalism::Metric>(this->m_context.builder);
    data->optimization_direction = source.get_optimization_direction();
    data->expression = expression;
    auto out = formalism::insert(this->m_context.storage->repository, *data).first;
    remember(this->m_context.storage->metrics, source, out);
    return out;
}

template<typename Derived>
formalism::InitialFunctionValueView TopLevelTranslator<Derived>::copy(formalism::InitialFunctionValueView source)
{
    if (auto mapped = find_mapped(this->m_context.storage->initial_function_values, source))
        return *mapped;
    const auto function = as_index(this->self().copy(source.get_function()));
    const auto value = as_index(this->self().copy(source.get_value()));
    auto data = formalism::checkout<formalism::InitialFunctionValue>(this->m_context.builder);
    data->function = function;
    data->value = value;
    auto out = formalism::insert(this->m_context.storage->repository, *data).first;
    remember(this->m_context.storage->initial_function_values, source, out);
    return out;
}

}  // namespace loki::semantic::detail

#endif
