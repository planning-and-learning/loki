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
#include "loki/formalism/conditions.hpp"
#include "loki/formalism/effects.hpp"
#include "loki/formalism/views.hpp"
#include "loki/semantic/errors.hpp"

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

    void compile_conditional_effect_actions(formalism::EntityListView<formalism::Action> actions, urd::built_t<urd::flat_vector<urd::index<formalism::Action>>, formalism::Repository>& result);
};

template<typename Derived>
void CompileConditionalEffectsTranslator<Derived>::compile_conditional_effect_actions(formalism::EntityListView<formalism::Action> actions,
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

        if (const auto effect_view = action.get_effect())
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
            for (auto i = size_t { 0 }; i < conditional.size(); ++i)
            {
                const auto selected = (mask & (size_t { 1 } << i)) != 0;
                if (selected)
                {
                    this->self().append_conjunct(*condition_data, conditional[i].condition);
                    urd::append(effect_data->effects, conditional[i].effect);
                }
                else
                {
                    this->self().append_conjunct(*condition_data, this->self().negate_condition(conditional[i].condition));
                }
            }

            // In the action builder's form: a single conjunct is moved over as it is held.
            auto precondition = urd::built_t<urd::flat_optional<formalism::ConditionVariant>, formalism::Repository> {};
            if (condition_data->conditions.size() == 1)
                precondition = std::move(condition_data->conditions.front());
            else if (!condition_data->conditions.empty())
                urd::store(precondition, this->self().make_conjunction(*condition_data));

            auto effect = urd::built_t<urd::flat_optional<formalism::EffectVariant>, formalism::Repository> {};
            if (effect_data->effects.size() == 1)
                effect = std::move(effect_data->effects.front());
            else if (!effect_data->effects.empty())
            {
                urd::store(effect, this->copy_context().storage->repository.insert(*effect_data, this->copy_context().storage->builder).first);
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
