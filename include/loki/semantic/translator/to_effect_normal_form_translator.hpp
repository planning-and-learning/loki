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

#ifndef LOKI_SEMANTIC_TRANSLATOR_TO_EFFECT_NORMAL_FORM_TRANSLATOR_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_TO_EFFECT_NORMAL_FORM_TRANSLATOR_HPP_

#include "loki/semantic/translator/copy_translator_component.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

#include <algorithm>
#include <optional>

namespace loki::semantic::detail
{

template<typename Derived>
class ToEffectNormalFormTranslator : public CopyTranslatorComponent<Derived, ToEffectNormalFormTranslator<Derived>>
{
public:
    explicit ToEffectNormalFormTranslator(CopyContext& context) : CopyTranslatorComponent<Derived, ToEffectNormalFormTranslator<Derived>>(context) {}

    template<typename T>
    std::optional<formalism::EntityView<T>> as_effect(formalism::EffectView effect) const;
    formalism::EffectView normalize_effect(formalism::EffectView effect);
    template<typename T>
    formalism::EffectView normalize_effect_node(formalism::EffectView effect, formalism::EntityView<T>);
    formalism::EffectView normalize_effect_node(formalism::EffectView, formalism::EntityView<formalism::Effect<formalism::AndTag>> node);
    formalism::EffectView normalize_effect_node(formalism::EffectView, formalism::EntityView<formalism::Effect<formalism::ForallTag>> node);
    formalism::EffectView normalize_effect_node(formalism::EffectView, formalism::EntityView<formalism::Effect<formalism::WhenTag>> node);
};

template<typename Derived>
template<typename T>
[[nodiscard]] std::optional<formalism::EntityView<T>> ToEffectNormalFormTranslator<Derived>::as_effect(formalism::EffectView effect) const
{
    if (std::holds_alternative<formalism::EntityView<T>>(effect))
        return std::get<formalism::EntityView<T>>(effect);
    return std::nullopt;
}

template<typename Derived>
formalism::EffectView ToEffectNormalFormTranslator<Derived>::normalize_effect(formalism::EffectView effect)
{
    return std::visit([&](const auto& node) { return this->self().normalize_effect_node(effect, node); }, effect);
}

template<typename Derived>
template<typename T>
formalism::EffectView ToEffectNormalFormTranslator<Derived>::normalize_effect_node(formalism::EffectView effect, formalism::EntityView<T>)
{ return effect; }

template<typename Derived>
formalism::EffectView ToEffectNormalFormTranslator<Derived>::normalize_effect_node(formalism::EffectView,
                                                                                   formalism::EntityView<formalism::Effect<formalism::AndTag>> node)
{
    struct NumericGroup
    {
        formalism::NumericEffectOperator op {};
        urd::index<formalism::FunctionTerm> function;
        formalism::FunctionExpressionList expressions;
    };

    auto result = formalism::checkout<formalism::Effect<formalism::AndTag>>(this->copy_context().builder);
    auto numeric_groups = urd::Store<urd::sequence<NumericGroup>> {};

    auto push_effect = [&](formalism::EffectView effect)
    {
        if (const auto numeric = this->self().template as_effect<formalism::Effect<formalism::NumericTag>>(effect))
        {
            const auto& data = numeric->data();
            auto it = std::find_if(numeric_groups.begin(),
                                   numeric_groups.end(),
                                   [&](const NumericGroup& group) { return group.op == data.op && group.function == data.function; });
            if (it == numeric_groups.end())
            {
                auto group = NumericGroup {};
                group.op = data.op;
                group.function = data.function;
                group.expressions.push_back(data.expression);
                numeric_groups.push_back(std::move(group));
            }
            else
            {
                it->expressions.push_back(data.expression);
            }
            return;
        }
        result->effects.push_back(as_index(effect));
    };

    for (auto child : node.get_effects())
    {
        const auto normalized = this->self().normalize_effect(child);
        if (const auto nested = this->self().template as_effect<formalism::Effect<formalism::AndTag>>(normalized))
        {
            for (auto part : nested->get_effects())
                push_effect(part);
        }
        else
        {
            push_effect(normalized);
        }
    }

    for (const auto& group : numeric_groups)
    {
        // A single expression stays as-is; only genuine aggregations get a sum, keeping
        // normalization idempotent across repeated translations.
        auto sum = group.expressions.front();
        if (group.expressions.size() > 1)
        {
            auto multi_data = formalism::checkout<formalism::FunctionExpression<formalism::MultiTag>>(this->copy_context().builder);
            multi_data->op = formalism::MultiArithmeticOperator::Add;
            for (const auto expression : group.expressions)
                multi_data->args.push_back(expression);
            sum = formalism::insert(this->copy_context().storage->repository, *multi_data).first.index();
        }
        auto numeric_data = formalism::checkout<formalism::Effect<formalism::NumericTag>>(this->copy_context().builder);
        numeric_data->op = group.op;
        numeric_data->function = group.function;
        numeric_data->expression = sum;
        result->effects.push_back(formalism::insert(this->copy_context().storage->repository, *numeric_data).first.index());
    }

    return formalism::EffectView(formalism::insert(this->copy_context().storage->repository, *result).first);
}

template<typename Derived>
formalism::EffectView ToEffectNormalFormTranslator<Derived>::normalize_effect_node(formalism::EffectView,
                                                                                   formalism::EntityView<formalism::Effect<formalism::ForallTag>> node)
{
    const auto& data = node.data();
    const auto nested = this->self().normalize_effect(node.get_effect());
    if (const auto nested_and = this->self().template as_effect<formalism::Effect<formalism::AndTag>>(nested))
    {
        auto conjunction_data = formalism::checkout<formalism::Effect<formalism::AndTag>>(this->copy_context().builder);
        for (auto part : nested_and->get_effects())
        {
            auto forall_data = formalism::checkout<formalism::Effect<formalism::ForallTag>>(this->copy_context().builder);
            for (auto parameter : data.parameters)
                forall_data->parameters.push_back(parameter);
            forall_data->effect = as_index(part);
            const auto wrapped = formalism::EffectView(formalism::insert(this->copy_context().storage->repository, *forall_data).first);
            conjunction_data->effects.push_back(as_index(this->self().normalize_effect(wrapped)));
        }
        const auto conjunction = formalism::EffectView(formalism::insert(this->copy_context().storage->repository, *conjunction_data).first);
        return this->self().normalize_effect(conjunction);
    }
    if (const auto nested_forall = this->self().template as_effect<formalism::Effect<formalism::ForallTag>>(nested))
    {
        auto forall_data = formalism::checkout<formalism::Effect<formalism::ForallTag>>(this->copy_context().builder);
        for (auto parameter : data.parameters)
            forall_data->parameters.push_back(parameter);
        for (auto parameter : nested_forall->get_parameters())
            forall_data->parameters.push_back(parameter.index());
        forall_data->effect = as_index(nested_forall->get_effect());
        const auto wrapped = formalism::EffectView(formalism::insert(this->copy_context().storage->repository, *forall_data).first);
        return this->self().normalize_effect(wrapped);
    }
    auto forall_data = formalism::checkout<formalism::Effect<formalism::ForallTag>>(this->copy_context().builder);
    for (auto parameter : data.parameters)
        forall_data->parameters.push_back(parameter);
    forall_data->effect = as_index(nested);
    return formalism::EffectView(formalism::insert(this->copy_context().storage->repository, *forall_data).first);
}

template<typename Derived>
formalism::EffectView ToEffectNormalFormTranslator<Derived>::normalize_effect_node(formalism::EffectView,
                                                                                   formalism::EntityView<formalism::Effect<formalism::WhenTag>> node)
{
    const auto moved_condition = this->self().move_existentials(node.get_condition());
    auto condition = this->self().flatten_condition(this->self().to_dnf(moved_condition));
    const auto nested = this->self().normalize_effect(node.get_effect());

    if (const auto condition_or = this->self().as_or(condition))
    {
        auto conjunction_data = formalism::checkout<formalism::Effect<formalism::AndTag>>(this->copy_context().builder);
        for (auto part : condition_or->get_conditions())
        {
            auto when_data = formalism::checkout<formalism::Effect<formalism::WhenTag>>(this->copy_context().builder);
            when_data->condition = as_index(part);
            when_data->effect = as_index(nested);
            const auto wrapped = formalism::EffectView(formalism::insert(this->copy_context().storage->repository, *when_data).first);
            conjunction_data->effects.push_back(as_index(this->self().normalize_effect(wrapped)));
        }
        const auto conjunction = formalism::EffectView(formalism::insert(this->copy_context().storage->repository, *conjunction_data).first);
        return this->self().normalize_effect(conjunction);
    }

    if (const auto nested_when = this->self().template as_effect<formalism::Effect<formalism::WhenTag>>(nested))
    {
        auto condition_data = formalism::checkout<formalism::Condition<formalism::AndTag>>(this->copy_context().builder);
        this->self().append_conjunct(*condition_data, condition);
        this->self().append_conjunct(*condition_data, nested_when->get_condition());
        const auto combined_condition = as_index(this->self().make_conjunction(*condition_data));
        auto when_data = formalism::checkout<formalism::Effect<formalism::WhenTag>>(this->copy_context().builder);
        when_data->condition = combined_condition;
        when_data->effect = as_index(nested_when->get_effect());
        const auto wrapped = formalism::EffectView(formalism::insert(this->copy_context().storage->repository, *when_data).first);
        return this->self().normalize_effect(wrapped);
    }
    if (const auto nested_and = this->self().template as_effect<formalism::Effect<formalism::AndTag>>(nested))
    {
        auto conjunction_data = formalism::checkout<formalism::Effect<formalism::AndTag>>(this->copy_context().builder);
        for (auto part : nested_and->get_effects())
        {
            auto when_data = formalism::checkout<formalism::Effect<formalism::WhenTag>>(this->copy_context().builder);
            when_data->condition = as_index(condition);
            when_data->effect = as_index(part);
            const auto wrapped = formalism::EffectView(formalism::insert(this->copy_context().storage->repository, *when_data).first);
            conjunction_data->effects.push_back(as_index(this->self().normalize_effect(wrapped)));
        }
        const auto conjunction = formalism::EffectView(formalism::insert(this->copy_context().storage->repository, *conjunction_data).first);
        return this->self().normalize_effect(conjunction);
    }
    if (const auto nested_forall = this->self().template as_effect<formalism::Effect<formalism::ForallTag>>(nested))
    {
        auto forall_data = formalism::checkout<formalism::Effect<formalism::ForallTag>>(this->copy_context().builder);
        for (auto parameter : nested_forall->get_parameters())
            forall_data->parameters.push_back(parameter.index());
        auto when_data = formalism::checkout<formalism::Effect<formalism::WhenTag>>(this->copy_context().builder);
        when_data->condition = as_index(condition);
        when_data->effect = as_index(nested_forall->get_effect());
        forall_data->effect = formalism::insert(this->copy_context().storage->repository, *when_data).first.index();
        const auto wrapped = formalism::EffectView(formalism::insert(this->copy_context().storage->repository, *forall_data).first);
        return this->self().normalize_effect(wrapped);
    }
    if (const auto exists = this->self().as_exists(condition))
    {
        auto forall_data = formalism::checkout<formalism::Effect<formalism::ForallTag>>(this->copy_context().builder);
        for (auto parameter : exists->get_parameters())
            forall_data->parameters.push_back(parameter.index());
        auto when_data = formalism::checkout<formalism::Effect<formalism::WhenTag>>(this->copy_context().builder);
        when_data->condition = as_index(exists->get_condition());
        when_data->effect = as_index(nested);
        forall_data->effect = formalism::insert(this->copy_context().storage->repository, *when_data).first.index();
        const auto wrapped = formalism::EffectView(formalism::insert(this->copy_context().storage->repository, *forall_data).first);
        return this->self().normalize_effect(wrapped);
    }

    auto when_data = formalism::checkout<formalism::Effect<formalism::WhenTag>>(this->copy_context().builder);
    when_data->condition = as_index(condition);
    when_data->effect = as_index(nested);
    return formalism::EffectView(formalism::insert(this->copy_context().storage->repository, *when_data).first);
}

}  // namespace loki::semantic::detail

#endif
