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

#include "loki/semantic/errors.hpp"
#include "loki/semantic/translator/copy_translator_component.hpp"

#include <limits>
#include <string>
#include <vector>
#include <yggdrasil/containers/unordered_set.hpp>

namespace loki::semantic::detail
{

template<typename Derived>
class CompileConditionalEffectsTranslator : public CopyTranslatorComponent<Derived, CompileConditionalEffectsTranslator<Derived>>
{
public:
    explicit CompileConditionalEffectsTranslator(CopyContext& context) : CopyTranslatorComponent<Derived, CompileConditionalEffectsTranslator<Derived>>(context)
    {
    }

    void compile_conditional_effect_actions(formalism::EntityListView<formalism::Action> actions, ygg::IndexList<formalism::Action>& result);
};

template<typename Derived>
void CompileConditionalEffectsTranslator<Derived>::compile_conditional_effect_actions(formalism::EntityListView<formalism::Action> actions,
                                                                                      ygg::IndexList<formalism::Action>& result)
{
    struct ConditionalEffect
    {
        formalism::EntityView<formalism::Effect<formalism::WhenTag>> when;
        formalism::ConditionView condition;
        formalism::EffectView effect;
    };

    result.clear();
    auto seen = ygg::UnorderedSet<formalism::ActionView> {};

    for (auto action : actions)
    {
        const auto& data = action.get_data();
        auto unconditional = ygg::IndexList<formalism::Effect<>> {};
        auto conditional = std::vector<ConditionalEffect> {};

        if (const auto effect_view = action.get_effect())
        {
            auto effects = std::vector<formalism::EffectView> { effect_view.value() };
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
                    unconditional.push_back(effect.get_index());
            }
        }

        if (conditional.empty())
        {
            this->self().push_unique(result, seen, action);
            continue;
        }

        if (conditional.size() >= std::numeric_limits<size_t>::digits)
            throw SemanticError("Too many conditional effects to compile.");

        const auto num_variants = size_t { 1 } << conditional.size();
        auto condition_data = formalism::checkout<formalism::Condition<formalism::AndTag>>(this->m_context.builder);
        auto effect_data = formalism::checkout<formalism::Effect<formalism::AndTag>>(this->m_context.builder);
        for (auto mask = size_t { 0 }; mask < num_variants; ++mask)
        {
            condition_data->clear();
            if (const auto precondition = action.get_precondition())
                this->self().append_conjunct(*condition_data, *precondition);

            effect_data->clear();
            for (auto effect : unconditional)
                effect_data->effects.push_back(effect);
            for (auto i = size_t { 0 }; i < conditional.size(); ++i)
            {
                const auto selected = (mask & (size_t { 1 } << i)) != 0;
                if (selected)
                {
                    this->self().append_conjunct(*condition_data, conditional[i].condition);
                    effect_data->effects.push_back(conditional[i].effect.get_index());
                }
                else
                {
                    this->self().append_conjunct(*condition_data, this->self().negate_condition(conditional[i].condition));
                }
            }

            auto precondition = cista::optional<ygg::Index<formalism::Condition<>>> {};
            if (condition_data->conditions.size() == 1)
                precondition = condition_data->conditions.front();
            else if (!condition_data->conditions.empty())
                precondition = as_index(this->self().make_conjunction(*condition_data));

            auto effect = cista::optional<ygg::Index<formalism::Effect<>>> {};
            if (effect_data->effects.size() == 1)
                effect = effect_data->effects.front();
            else if (!effect_data->effects.empty())
            {
                effect = this->self().wrap_effect(formalism::insert(this->m_context.storage->repository, *effect_data).first).get_index();
            }

            auto name = std::string(data.name) + "_" + std::to_string(action.get_index().get_value());
            for (const auto& item : conditional)
                name += "_" + std::to_string(item.when.get_index().get_value());
            name += "_" + std::to_string(mask);

            auto action_data = formalism::checkout<formalism::Action>(this->m_context.builder);
            action_data->name = cista::offset::string(name);
            action_data->original_name = data.original_name;
            for (auto parameter : data.parameters)
                action_data->parameters.push_back(parameter);
            action_data->original_arity = data.original_arity;
            action_data->precondition = precondition;
            action_data->effect = effect;
            this->self().push_unique(result, seen, formalism::insert(this->m_context.storage->repository, *action_data).first);
        }
    }
}

}  // namespace loki::semantic::detail

#endif
