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

#include "copy_translator_component.hpp"

namespace loki::semantic::detail
{

template<typename Derived>
class TopLevelTranslator : public CopyTranslatorComponent<Derived, TopLevelTranslator<Derived>>
{
public:
    explicit TopLevelTranslator(CopyContext& context) : CopyTranslatorComponent<Derived, TopLevelTranslator<Derived>>(context) {}

    formalism::ActionView copy(formalism::ActionView source);

    /// A durative action: its parameters scope its duration, condition and effect. Its condition is never split or
    /// lifted into parameters: a disjunctive or existential timed condition is a derived predicate instead
    /// (RemoveUniversalQuantifiersTranslator), since an over all condition fixed to one disjunct or witness would hold
    /// throughout the interval only for it. Compiling typing adds the type conditions to its at start condition.
    formalism::DurativeActionView copy(formalism::DurativeActionView source);

    formalism::AxiomView copy(formalism::AxiomView source);

    formalism::MetricView copy(formalism::MetricView source);

    formalism::InitialFunctionValueView copy(formalism::InitialFunctionValueView source);
};

template<typename Derived>
formalism::ActionView TopLevelTranslator<Derived>::copy(formalism::ActionView source)
{
    if (this->copy_context().phase == TranslationPhase::RenameQuantifiedVariables && this->copy_context().renaming_enabled)
    {
        const auto renamed = this->self().rename_action_variables(source);
        const auto renaming_disabled = RenamingDisabled(this->copy_context());
        auto out = this->self().copy(renamed);
        remember(this->copy_context().storage->maps, source, out);
        return out;
    }

    return memoized(this->copy_context().storage->maps,
                    source,
                    [&]
                    {
                        this->copy_context().num_quantifications.clear();
                        this->self().increment_quantifications(source.parameters());
                        auto parameter_views = this->self().copy_parameter_views(source.parameters());
                        this->self().enter_scope(parameter_views);
                        auto precondition = std::optional<formalism::ConditionView> {};
                        if (const auto condition = source.precondition())
                        {
                            auto copied_condition = this->self().copy(condition.value());
                            if (this->copy_context().phase == TranslationPhase::MoveExistentialQuantifiers)
                            {
                                auto effect = std::optional<formalism::EffectView> {};
                                if (const auto effect_view = source.effect())
                                    effect = effect_view.value();
                                copied_condition = this->self().lift_top_level_exists(parameter_views, copied_condition, effect);
                            }
                            precondition = copied_condition;
                        }
                        if (this->copy_context().phase == TranslationPhase::CompileTyping)
                            this->self().prepend_type_conditions(precondition, source.parameters());
                        auto effect = urd::flat_optional<formalism::EffectVariant> {};
                        if (const auto effect_view = source.effect())
                            urd::store(effect, this->self().copy(effect_view.value()));
                        auto result = formalism::checkout<formalism::Action>(this->copy_context().builder);
                        result->name = source.name();
                        result->original_name = source.original_name();
                        if (this->self().compiles_typing_now())
                            this->self().copy_parameters_without_types(source.parameters(), result->parameters);
                        else
                            for (auto parameter : parameter_views)
                                result->parameters.push_back(parameter.index());
                        result->original_arity = source.original_arity();
                        if (precondition)
                            urd::store(result->precondition, *precondition);
                        urd::store(result->effect, effect);
                        auto out = this->copy_context().storage->repository.insert(*result, this->copy_context().storage->builder).first;
                        this->self().leave_scope();
                        return out;
                    });
}

template<typename Derived>
formalism::DurativeActionView TopLevelTranslator<Derived>::copy(formalism::DurativeActionView source)
{
    if (this->copy_context().phase == TranslationPhase::RenameQuantifiedVariables && this->copy_context().renaming_enabled)
    {
        this->copy_context().num_quantifications.clear();
        this->copy_context().renamed_variable_names.clear();
        const auto renamed = this->self().rename_variables(source);
        const auto renaming_disabled = RenamingDisabled(this->copy_context());
        auto out = this->self().copy(renamed);
        remember(this->copy_context().storage->maps, source, out);
        return out;
    }

    return memoized(this->copy_context().storage->maps,
                    source,
                    [&]
                    {
                        this->copy_context().num_quantifications.clear();
                        this->self().increment_quantifications(source.parameters());
                        auto parameter_views = this->self().copy_parameter_views(source.parameters());
                        this->self().enter_scope(parameter_views);
                        auto duration = std::optional<formalism::ConditionView> {};
                        if (const auto source_duration = source.duration())
                            duration = this->self().copy(source_duration.value());
                        auto condition = std::optional<formalism::ConditionView> {};
                        if (const auto source_condition = source.condition())
                            condition = this->self().copy(source_condition.value());
                        if (this->copy_context().phase == TranslationPhase::CompileTyping)
                            condition = this->self().prepend_start_type_conditions(condition, source.parameters());
                        auto effect = std::optional<formalism::EffectView> {};
                        if (const auto source_effect = source.effect())
                            effect = this->self().copy(source_effect.value());
                        auto result = formalism::checkout<formalism::DurativeAction>(this->copy_context().builder);
                        result->name = source.name();
                        result->original_name = source.original_name();
                        if (this->self().compiles_typing_now())
                            this->self().copy_parameters_without_types(source.parameters(), result->parameters);
                        else
                            for (auto parameter : parameter_views)
                                result->parameters.push_back(parameter.index());
                        result->original_arity = source.original_arity();
                        if (duration)
                            urd::store(result->duration, *duration);
                        if (condition)
                            urd::store(result->condition, *condition);
                        if (effect)
                            urd::store(result->effect, *effect);
                        auto out = this->copy_context().storage->repository.insert(*result, this->copy_context().storage->builder).first;
                        this->self().leave_scope();
                        return out;
                    });
}

template<typename Derived>
formalism::AxiomView TopLevelTranslator<Derived>::copy(formalism::AxiomView source)
{
    if (this->copy_context().phase == TranslationPhase::RenameQuantifiedVariables && this->copy_context().renaming_enabled)
    {
        const auto renamed = this->self().rename_axiom_variables(source);
        const auto renaming_disabled = RenamingDisabled(this->copy_context());
        auto out = this->self().copy(renamed);
        remember(this->copy_context().storage->maps, source, out);
        return out;
    }

    return memoized(this->copy_context().storage->maps,
                    source,
                    [&]
                    {
                        this->copy_context().num_quantifications.clear();
                        this->self().increment_quantifications(source.parameters());
                        auto parameter_views = this->self().copy_parameter_views(source.parameters());
                        this->self().enter_scope(parameter_views);
                        auto copied_condition = this->self().copy(source.condition());
                        if (this->copy_context().phase == TranslationPhase::MoveExistentialQuantifiers)
                            copied_condition = this->self().lift_top_level_exists(parameter_views, copied_condition, std::nullopt);
                        if (this->copy_context().phase == TranslationPhase::CompileTyping)
                            this->self().prepend_type_conditions(copied_condition, source.parameters());
                        const auto head = this->self().copy(source.head());
                        auto result = formalism::checkout<formalism::Axiom>(this->copy_context().builder);
                        if (this->self().compiles_typing_now())
                            this->self().copy_parameters_without_types(source.parameters(), result->parameters);
                        else
                            for (auto parameter : parameter_views)
                                result->parameters.push_back(parameter.index());
                        result->original_arity = source.original_arity();
                        urd::store(result->head, head);
                        urd::store(result->condition, copied_condition);
                        auto out = this->copy_context().storage->repository.insert(*result, this->copy_context().storage->builder).first;
                        this->self().leave_scope();
                        return out;
                    });
}

template<typename Derived>
formalism::MetricView TopLevelTranslator<Derived>::copy(formalism::MetricView source)
{
    return memoized(this->copy_context().storage->maps,
                    source,
                    [&]
                    {
                        const auto expression = this->self().copy(source.expression());
                        auto data = formalism::checkout<formalism::Metric>(this->copy_context().builder);
                        data->optimization_direction = source.optimization_direction();
                        urd::store(data->expression, expression);
                        return this->copy_context().storage->repository.insert(*data, this->copy_context().storage->builder).first;
                    });
}

template<typename Derived>
formalism::InitialFunctionValueView TopLevelTranslator<Derived>::copy(formalism::InitialFunctionValueView source)
{
    return memoized(this->copy_context().storage->maps,
                    source,
                    [&]
                    {
                        const auto function = this->self().copy(source.function());
                        const auto value = this->self().copy(source.value());
                        auto data = formalism::checkout<formalism::InitialFunctionValue>(this->copy_context().builder);
                        urd::store(data->function, function);
                        urd::store(data->value, value);
                        return this->copy_context().storage->repository.insert(*data, this->copy_context().storage->builder).first;
                    });
}

}  // namespace loki::semantic::detail

#endif
