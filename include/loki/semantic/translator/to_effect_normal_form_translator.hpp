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

#include <algorithm>
#include <optional>

namespace loki::semantic::detail
{

template<typename Derived>
class ToEffectNormalFormTranslator : public CopyTranslatorComponent<Derived, ToEffectNormalFormTranslator<Derived>>
{
public:
    explicit ToEffectNormalFormTranslator(CopyContext& context) : CopyTranslatorComponent<Derived, ToEffectNormalFormTranslator<Derived>>(context) {}

    formalism::EffectView wrap_effect(ygg::Data<formalism::Effect>::Variant value);
    template<typename T>
    formalism::EffectView wrap_effect(formalism::EntityView<T> value);
    template<typename T>
    std::optional<formalism::EntityView<T>> as_effect(formalism::EffectView effect) const;
    formalism::EffectView normalize_effect(formalism::EffectView effect);
    template<typename T>
    formalism::EffectView normalize_effect_node(formalism::EffectView effect, formalism::EntityView<T>);
    formalism::EffectView normalize_effect_node(formalism::EffectView, formalism::EffectAndView node);
    formalism::EffectView normalize_effect_node(formalism::EffectView, formalism::EffectForallView node);
    formalism::EffectView normalize_effect_node(formalism::EffectView, formalism::EffectWhenView node);
};

template<typename Derived>
formalism::EffectView ToEffectNormalFormTranslator<Derived>::wrap_effect(ygg::Data<formalism::Effect>::Variant value)
{
    auto data = formalism::checkout<formalism::Effect>(this->m_context.builder);
    data->variant = std::move(value);
    return formalism::insert(this->m_storage->repository, *data).first;
}

template<typename Derived>
template<typename T>
formalism::EffectView ToEffectNormalFormTranslator<Derived>::wrap_effect(formalism::EntityView<T> value)
{
    auto data = formalism::checkout<formalism::Effect>(this->m_context.builder);
    data->variant = ygg::Data<formalism::Effect>::Variant(value.get_index());
    return formalism::insert(this->m_storage->repository, *data).first;
}

template<typename Derived>
template<typename T>
std::optional<formalism::EntityView<T>> ToEffectNormalFormTranslator<Derived>::as_effect(formalism::EffectView effect) const
{
    auto result = std::optional<formalism::EntityView<T>> {};
    ygg::visit(
        [&](const auto& node)
        {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, formalism::EntityView<T>>)
                result = node;
        },
        effect.get_variant());
    return result;
}

template<typename Derived>
formalism::EffectView ToEffectNormalFormTranslator<Derived>::normalize_effect(formalism::EffectView effect)
{
    return ygg::visit([&](const auto& node) { return this->self().normalize_effect_node(effect, node); }, effect.get_variant());
}

template<typename Derived>
template<typename T>
formalism::EffectView ToEffectNormalFormTranslator<Derived>::normalize_effect_node(formalism::EffectView effect, formalism::EntityView<T>)
{
    return effect;
}

template<typename Derived>
formalism::EffectView ToEffectNormalFormTranslator<Derived>::normalize_effect_node(formalism::EffectView, formalism::EffectAndView node)
{
    struct NumericGroup
    {
        formalism::NumericEffectOperator op {};
        ygg::Index<formalism::FunctionTerm> function;
        ygg::IndexList<formalism::FunctionExpression> expressions;
    };

    auto result = formalism::checkout<formalism::EffectAnd>(this->m_context.builder);
    auto numeric_groups = std::vector<NumericGroup> {};

    auto push_effect = [&](formalism::EffectView effect)
    {
        if (const auto numeric = this->self().template as_effect<formalism::EffectNumeric>(effect))
        {
            const auto& data = numeric->get_data();
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
        result->effects.push_back(effect.get_index());
    };

    for (auto child : node.get_effects())
    {
        const auto normalized = this->self().normalize_effect(child);
        if (const auto nested = this->self().template as_effect<formalism::EffectAnd>(normalized))
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
            auto multi_data = formalism::checkout<formalism::MultiFunctionExpression>(this->m_context.builder);
            multi_data->op = formalism::MultiArithmeticOperator::Add;
            for (const auto expression : group.expressions)
                multi_data->args.push_back(expression);
            const auto multi = formalism::insert(this->m_storage->repository, *multi_data).first.get_index();
            auto expression_data = formalism::checkout<formalism::FunctionExpression>(this->m_context.builder);
            expression_data->variant = ygg::Data<formalism::FunctionExpression>::Variant(multi);
            sum = formalism::insert(this->m_storage->repository, *expression_data).first.get_index();
        }
        auto numeric_data = formalism::checkout<formalism::EffectNumeric>(this->m_context.builder);
        numeric_data->op = group.op;
        numeric_data->function = group.function;
        numeric_data->expression = sum;
        result->effects.push_back(this->self().wrap_effect(formalism::insert(this->m_storage->repository, *numeric_data).first).get_index());
    }

    return this->self().wrap_effect(formalism::insert(this->m_storage->repository, *result).first);
}

template<typename Derived>
formalism::EffectView ToEffectNormalFormTranslator<Derived>::normalize_effect_node(formalism::EffectView, formalism::EffectForallView node)
{
    const auto& data = node.get_data();
    const auto nested = this->self().normalize_effect(node.get_effect());
    if (const auto nested_and = this->self().template as_effect<formalism::EffectAnd>(nested))
    {
        auto conjunction_data = formalism::checkout<formalism::EffectAnd>(this->m_context.builder);
        for (auto part : nested_and->get_effects())
        {
            auto forall_data = formalism::checkout<formalism::EffectForall>(this->m_context.builder);
            for (auto parameter : data.parameters)
                forall_data->parameters.push_back(parameter);
            forall_data->effect = part.get_index();
            const auto wrapped = this->self().wrap_effect(formalism::insert(this->m_storage->repository, *forall_data).first);
            conjunction_data->effects.push_back(this->self().normalize_effect(wrapped).get_index());
        }
        const auto conjunction = this->self().wrap_effect(formalism::insert(this->m_storage->repository, *conjunction_data).first);
        return this->self().normalize_effect(conjunction);
    }
    if (const auto nested_forall = this->self().template as_effect<formalism::EffectForall>(nested))
    {
        auto forall_data = formalism::checkout<formalism::EffectForall>(this->m_context.builder);
        for (auto parameter : data.parameters)
            forall_data->parameters.push_back(parameter);
        for (auto parameter : nested_forall->get_parameters())
            forall_data->parameters.push_back(parameter.get_index());
        forall_data->effect = nested_forall->get_effect().get_index();
        const auto wrapped = this->self().wrap_effect(formalism::insert(this->m_storage->repository, *forall_data).first);
        return this->self().normalize_effect(wrapped);
    }
    auto forall_data = formalism::checkout<formalism::EffectForall>(this->m_context.builder);
    for (auto parameter : data.parameters)
        forall_data->parameters.push_back(parameter);
    forall_data->effect = nested.get_index();
    return this->self().wrap_effect(formalism::insert(this->m_storage->repository, *forall_data).first);
}

template<typename Derived>
formalism::EffectView ToEffectNormalFormTranslator<Derived>::normalize_effect_node(formalism::EffectView, formalism::EffectWhenView node)
{
    const auto moved_condition = this->self().move_existentials(node.get_condition());
    auto condition = this->self().flatten_condition(this->self().to_dnf(moved_condition));
    const auto nested = this->self().normalize_effect(node.get_effect());

    if (const auto condition_or = this->self().as_or(condition))
    {
        auto conjunction_data = formalism::checkout<formalism::EffectAnd>(this->m_context.builder);
        for (auto part : condition_or->get_conditions())
        {
            auto when_data = formalism::checkout<formalism::EffectWhen>(this->m_context.builder);
            when_data->condition = part.get_index();
            when_data->effect = nested.get_index();
            const auto wrapped = this->self().wrap_effect(formalism::insert(this->m_storage->repository, *when_data).first);
            conjunction_data->effects.push_back(this->self().normalize_effect(wrapped).get_index());
        }
        const auto conjunction = this->self().wrap_effect(formalism::insert(this->m_storage->repository, *conjunction_data).first);
        return this->self().normalize_effect(conjunction);
    }

    if (const auto nested_when = this->self().template as_effect<formalism::EffectWhen>(nested))
    {
        auto condition_data = formalism::checkout<formalism::ConditionAnd>(this->m_context.builder);
        this->self().append_conjunct(*condition_data, condition);
        this->self().append_conjunct(*condition_data, nested_when->get_condition());
        const auto combined_condition = as_index(this->self().make_conjunction(*condition_data));
        auto when_data = formalism::checkout<formalism::EffectWhen>(this->m_context.builder);
        when_data->condition = combined_condition;
        when_data->effect = nested_when->get_effect().get_index();
        const auto wrapped = this->self().wrap_effect(formalism::insert(this->m_storage->repository, *when_data).first);
        return this->self().normalize_effect(wrapped);
    }
    if (const auto nested_and = this->self().template as_effect<formalism::EffectAnd>(nested))
    {
        auto conjunction_data = formalism::checkout<formalism::EffectAnd>(this->m_context.builder);
        for (auto part : nested_and->get_effects())
        {
            auto when_data = formalism::checkout<formalism::EffectWhen>(this->m_context.builder);
            when_data->condition = condition.get_index();
            when_data->effect = part.get_index();
            const auto wrapped = this->self().wrap_effect(formalism::insert(this->m_storage->repository, *when_data).first);
            conjunction_data->effects.push_back(this->self().normalize_effect(wrapped).get_index());
        }
        const auto conjunction = this->self().wrap_effect(formalism::insert(this->m_storage->repository, *conjunction_data).first);
        return this->self().normalize_effect(conjunction);
    }
    if (const auto nested_forall = this->self().template as_effect<formalism::EffectForall>(nested))
    {
        auto forall_data = formalism::checkout<formalism::EffectForall>(this->m_context.builder);
        for (auto parameter : nested_forall->get_parameters())
            forall_data->parameters.push_back(parameter.get_index());
        auto when_data = formalism::checkout<formalism::EffectWhen>(this->m_context.builder);
        when_data->condition = condition.get_index();
        when_data->effect = nested_forall->get_effect().get_index();
        forall_data->effect = this->self().wrap_effect(formalism::insert(this->m_storage->repository, *when_data).first).get_index();
        const auto wrapped = this->self().wrap_effect(formalism::insert(this->m_storage->repository, *forall_data).first);
        return this->self().normalize_effect(wrapped);
    }
    if (const auto exists = this->self().as_exists(condition))
    {
        auto forall_data = formalism::checkout<formalism::EffectForall>(this->m_context.builder);
        for (auto parameter : exists->get_parameters())
            forall_data->parameters.push_back(parameter.get_index());
        auto when_data = formalism::checkout<formalism::EffectWhen>(this->m_context.builder);
        when_data->condition = exists->get_condition().get_index();
        when_data->effect = nested.get_index();
        forall_data->effect = this->self().wrap_effect(formalism::insert(this->m_storage->repository, *when_data).first).get_index();
        const auto wrapped = this->self().wrap_effect(formalism::insert(this->m_storage->repository, *forall_data).first);
        return this->self().normalize_effect(wrapped);
    }

    auto when_data = formalism::checkout<formalism::EffectWhen>(this->m_context.builder);
    when_data->condition = condition.get_index();
    when_data->effect = nested.get_index();
    return this->self().wrap_effect(formalism::insert(this->m_storage->repository, *when_data).first);
}

}  // namespace loki::semantic::detail

#endif
