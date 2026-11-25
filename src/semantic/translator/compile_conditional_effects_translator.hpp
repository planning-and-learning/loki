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

#ifndef LOKI_SEMANTIC_TRANSLATOR_COMPILE_CONDITIONAL_EFFECTS_TRANSLATOR_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_COMPILE_CONDITIONAL_EFFECTS_TRANSLATOR_HPP_

#include "copy_translator_component.hpp"
#include "loki/formalism/choices.hpp"
#include "loki/formalism/conditions.hpp"
#include "loki/formalism/effects.hpp"
#include "loki/formalism/views.hpp"
#include "loki/semantic/errors.hpp"

#include <algorithm>
#include <limits>
#include <string>
#include <urd/repr/maps/unordered_flat_map.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/repr/values/std_set.hpp>
#include <urd/repr/values/unordered_flat_set.hpp>
#include <urd/store.hpp>
#include <vector>

namespace loki::semantic::detail
{

template<typename Derived>
class CompileConditionalEffectsTranslator : public CopyTranslatorComponent<Derived, CompileConditionalEffectsTranslator<Derived>>
{
public:
    explicit CompileConditionalEffectsTranslator(CopyContext& context) : CopyTranslatorComponent<Derived, CompileConditionalEffectsTranslator<Derived>>(context)
    {
    }

    void compile_conditional_effect_actions(formalism::EntityListView<formalism::Action> actions,
                                            urd::built_t<urd::flat_vector<urd::index<formalism::Action>>, formalism::Repository>& result);

    /// The conditional effects of an effect that holds a choice: those reachable through conjunctions and outcomes, each
    /// once. Their conditions read the state the action is applied in, whichever outcome is drawn.
    void collect_choice_conditional_effects(formalism::EffectView effect,
                                            urd::Store<urd::sequence<formalism::EntityView<formalism::Effect<formalism::WhenTag>>>>& result) const;

    /// The effect with each conditional effect in selected replaced by its effect and each other one dropped.
    formalism::EffectView select_conditional_effects(formalism::EffectView effect,
                                                     const urd::Store<urd::set<formalism::EntityView<formalism::Effect<formalism::WhenTag>>>>& selected);
};

template<typename Derived>
void CompileConditionalEffectsTranslator<Derived>::collect_choice_conditional_effects(
    formalism::EffectView effect,
    urd::Store<urd::sequence<formalism::EntityView<formalism::Effect<formalism::WhenTag>>>>& result) const
{
    std::visit(
        [&]<typename Node>(const Node& node)
        {
            if constexpr (std::same_as<Node, formalism::EntityView<formalism::Effect<formalism::WhenTag>>>)
            {
                if (std::ranges::find(result, node) == result.end())
                    result.push_back(node);
                // A conditional effect over a choice (ToEffectNormalFormTranslator::factor_shared_condition) holds more.
                this->collect_choice_conditional_effects(node.get_effect(), result);
            }
            else if constexpr (std::same_as<Node, formalism::EntityView<formalism::Effect<formalism::AndTag>>>
                               || std::same_as<Node, formalism::EntityView<formalism::Effect<formalism::OneOfTag>>>)
            {
                for (auto child : node.get_effects())
                    this->collect_choice_conditional_effects(child, result);
            }
            else if constexpr (std::same_as<Node, formalism::EntityView<formalism::Effect<formalism::ProbabilisticTag>>>)
            {
                for (auto alternative : node.get_alternatives())
                    this->collect_choice_conditional_effects(alternative.get_effect(), result);
            }
        },
        effect);
}

template<typename Derived>
formalism::EffectView CompileConditionalEffectsTranslator<Derived>::select_conditional_effects(
    formalism::EffectView effect,
    const urd::Store<urd::set<formalism::EntityView<formalism::Effect<formalism::WhenTag>>>>& selected)
{
    auto& context = this->copy_context();
    return std::visit(
        [&]<typename Node>(const Node& node) -> formalism::EffectView
        {
            if constexpr (std::same_as<Node, formalism::EntityView<formalism::Effect<formalism::WhenTag>>>)
            {
                if (selected.contains(node))
                    return this->select_conditional_effects(node.get_effect(), selected);
                auto data = formalism::checkout<formalism::Effect<formalism::AndTag>>(context.builder);
                return context.storage->repository.insert(*data, context.storage->builder).first;
            }
            else if constexpr (std::same_as<Node, formalism::EntityView<formalism::Effect<formalism::AndTag>>>)
            {
                auto data = formalism::checkout<formalism::Effect<formalism::AndTag>>(context.builder);
                for (auto child : node.get_effects())
                {
                    const auto part = this->select_conditional_effects(child, selected);
                    if (const auto* nested = std::get_if<formalism::EntityView<formalism::Effect<formalism::AndTag>>>(&part))
                        for (auto nested_part : nested->get_effects())
                            urd::append(data->effects, nested_part);
                    else
                        urd::append(data->effects, part);
                }
                return context.storage->repository.insert(*data, context.storage->builder).first;
            }
            else if constexpr (std::same_as<Node, formalism::EntityView<formalism::Effect<formalism::OneOfTag>>>)
            {
                auto data = formalism::checkout<formalism::Effect<formalism::OneOfTag>>(context.builder);
                for (auto child : node.get_effects())
                    urd::append(data->effects, this->select_conditional_effects(child, selected));
                return context.storage->repository.insert(*data, context.storage->builder).first;
            }
            else if constexpr (std::same_as<Node, formalism::EntityView<formalism::Effect<formalism::ProbabilisticTag>>>)
            {
                auto data = formalism::checkout<formalism::Effect<formalism::ProbabilisticTag>>(context.builder);
                auto alternative_data = formalism::checkout<formalism::EffectProbabilisticAlternative>(context.builder);
                for (auto alternative : node.get_alternatives())
                {
                    urd::clear_fields(*alternative_data);
                    alternative_data->probability = alternative.get_probability();
                    urd::store(alternative_data->effect, this->select_conditional_effects(alternative.get_effect(), selected));
                    data->alternatives.emplace_back(context.storage->repository.insert(*alternative_data, context.storage->builder).first.index());
                }
                return context.storage->repository.insert(*data, context.storage->builder).first;
            }
            else
                return node;
        },
        effect);
}

template<typename Derived>
void CompileConditionalEffectsTranslator<Derived>::compile_conditional_effect_actions(
    formalism::EntityListView<formalism::Action> actions,
    urd::built_t<urd::flat_vector<urd::index<formalism::Action>>, formalism::Repository>& result)
{
    struct ConditionalEffect
    {
        formalism::EntityView<formalism::Effect<formalism::WhenTag>> when;
        formalism::ConditionView condition;
        formalism::EffectView effect;
    };

    result.clear();
    auto seen = urd::Store<urd::set<formalism::ActionView>> {};

    for (auto action : actions)
    {
        auto unconditional = formalism::EffectList {};
        auto conditional = urd::Store<urd::sequence<ConditionalEffect>> {};

        const auto effect_view = action.get_effect();
        const auto has_choice = effect_view && formalism::contains_choice(effect_view.value());
        if (has_choice)
        {
            auto whens = urd::Store<urd::sequence<formalism::EntityView<formalism::Effect<formalism::WhenTag>>>> {};
            this->self().collect_choice_conditional_effects(effect_view.value(), whens);
            for (auto when : whens)
                conditional.push_back({ when, when.get_condition(), when.get_effect() });
        }
        else if (effect_view)
        {
            auto effects = urd::Store<urd::sequence<formalism::EffectView>> { effect_view.value() };
            if (const auto effect_and = this->self().template as_effect<formalism::Effect<formalism::AndTag>>(effect_view.value()))
            {
                effects.clear();
                for (auto effect : effect_and->get_effects())
                    effects.push_back(effect);
            }

            for (auto effect : effects)
            {
                if (const auto effect_when = this->self().template as_effect<formalism::Effect<formalism::WhenTag>>(effect))
                    conditional.push_back({ *effect_when, effect_when->get_condition(), effect_when->get_effect() });
                else
                    urd::append(unconditional, effect);
            }
        }

        if (conditional.empty())
        {
            this->self().push_unique(result, seen, action);
            continue;
        }

        if (conditional.size() >= std::numeric_limits<size_t>::digits)
            throw ygg::diagnostics::SemanticError("Too many conditional effects to compile.");

        const auto num_variants = size_t { 1 } << conditional.size();
        auto condition_data = formalism::checkout<formalism::Condition<formalism::AndTag>>(this->copy_context().builder);
        auto effect_data = formalism::checkout<formalism::Effect<formalism::AndTag>>(this->copy_context().builder);
        for (auto mask = size_t { 0 }; mask < num_variants; ++mask)
        {
            urd::clear_fields(*condition_data);
            if (const auto precondition = action.get_precondition())
                this->self().append_conjunct(*condition_data, *precondition);

            urd::clear_fields(*effect_data);
            for (auto effect : unconditional)
                urd::append(effect_data->effects, effect);
            auto selected_whens = urd::Store<urd::set<formalism::EntityView<formalism::Effect<formalism::WhenTag>>>> {};
            for (auto i = size_t { 0 }; i < conditional.size(); ++i)
            {
                const auto selected = (mask & (size_t { 1 } << i)) != 0;
                if (selected)
                {
                    this->self().append_conjunct(*condition_data, conditional[i].condition);
                    if (has_choice)
                        selected_whens.insert(conditional[i].when);
                    else
                        urd::append(effect_data->effects, conditional[i].effect);
                }
                else
                {
                    this->self().append_conjunct(*condition_data, this->self().negate_condition(conditional[i].condition));
                }
            }
            if (has_choice)
                urd::append(effect_data->effects, this->self().select_conditional_effects(effect_view.value(), selected_whens));

            // In the action builder's form: a single conjunct is moved over as it is held.
            auto precondition = urd::built_t<urd::flat_optional<formalism::ConditionVariant>, formalism::Repository> {};
            if (condition_data->conditions.size() == 1)
                precondition = std::move(condition_data->conditions.front());
            else if (!condition_data->conditions.empty())
                urd::store(precondition, this->self().make_conjunction(*condition_data));

            // In effect normal form again: two selected conditional effects may write one numeric target, whose effects
            // the normal form aggregates.
            auto effect = urd::built_t<urd::flat_optional<formalism::EffectVariant>, formalism::Repository> {};
            if (effect_data->effects.size() == 1 && !has_choice)
                effect = std::move(effect_data->effects.front());
            else if (!effect_data->effects.empty())
            {
                const auto conjunction =
                    formalism::EffectView(this->copy_context().storage->repository.insert(*effect_data, this->copy_context().storage->builder).first);
                auto normalized = this->self().normalize_effect(conjunction);
                if (const auto single = this->self().template as_effect<formalism::Effect<formalism::AndTag>>(normalized);
                    single && single->get_effects().size() == 1)
                    normalized = single->get_effects()[0];
                urd::store(effect, normalized);
            }

            auto name = std::string(action.get_name()) + "_" + std::to_string(action.index().value);
            for (const auto& item : conditional)
                name += "_" + std::to_string(item.when.index().value);
            name += "_" + std::to_string(mask);

            auto action_data = formalism::checkout<formalism::Action>(this->copy_context().builder);
            action_data->name = urd::flat_string(name);
            action_data->original_name = action.get_original_name();
            for (auto parameter : action.get_parameters())
                action_data->parameters.push_back(parameter.index());
            action_data->original_arity = action.get_original_arity();
            action_data->precondition = std::move(precondition);
            action_data->effect = std::move(effect);
            this->self().push_unique(result, seen, this->copy_context().storage->repository.insert(*action_data, this->copy_context().storage->builder).first);
        }
    }
}

}  // namespace loki::semantic::detail

#endif
