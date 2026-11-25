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

#include "copy_translator_component.hpp"

#include <optional>
#include <urd/repr/maps/ordered_map.hpp>
#include <urd/repr/values/indexed.hpp>
#include <urd/store.hpp>
#include <utility>

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
    /// A oneof with a oneof outcome flattened into it; a single outcome is the effect itself.
    formalism::EffectView normalize_effect_node(formalism::EffectView, formalism::EntityView<formalism::Effect<formalism::OneOfTag>> node);
    /// A probabilistic effect with a probabilistic outcome flattened into it (probabilities multiplied), equal outcomes
    /// merged (probabilities added), outcomes of probability 0 dropped, and the remaining probability mass an explicit
    /// empty outcome; an outcome of probability 1 is the effect itself.
    formalism::EffectView normalize_effect_node(formalism::EffectView, formalism::EntityView<formalism::Effect<formalism::ProbabilisticTag>> node);
    /// A choice whose outcomes are all conditional on one condition (or empty) as that condition over the choice of their
    /// effects: (oneof (when c a) (when c b)) is (when c (oneof a b)), since the condition reads the state the choice is
    /// drawn in. One conditional effect instead of one per outcome: what compiling conditional effects multiplies.
    formalism::EffectView factor_shared_condition(formalism::EffectView choice);
};

template<typename Derived>
formalism::EffectView ToEffectNormalFormTranslator<Derived>::factor_shared_condition(formalism::EffectView choice)
{
    using WhenView = formalism::EntityView<formalism::Effect<formalism::WhenTag>>;
    using AndView = formalism::EntityView<formalism::Effect<formalism::AndTag>>;
    auto& context = this->copy_context();
    auto condition = std::optional<formalism::ConditionView> {};
    auto shared = true;
    // An outcome is conditional on c if it is (when c e), or a conjunction of such (effect normal form distributes c over
    // a conjunction), or empty.
    const auto parts = [](formalism::EffectView outcome)
    {
        auto result = urd::Store<urd::sequence<formalism::EffectView>> {};
        if (const auto* conjunction = std::get_if<AndView>(&outcome))
            for (auto part : conjunction->get_effects())
                result.push_back(part);
        else
            result.push_back(outcome);
        return result;
    };
    const auto check = [&](formalism::EffectView outcome)
    {
        for (auto part : parts(outcome))
        {
            const auto* when = std::get_if<WhenView>(&part);
            if (!when || (condition && !(*condition == when->get_condition())))
                shared = false;
            else
                condition = when->get_condition();
        }
    };
    const auto body = [&](formalism::EffectView outcome) -> formalism::EffectView
    {
        auto data = formalism::checkout<formalism::Effect<formalism::AndTag>>(context.builder);
        for (auto part : parts(outcome))
            urd::append(data->effects, std::get<WhenView>(part).get_effect());
        const auto result = context.storage->repository.insert(*data, context.storage->builder).first;
        return result.get_effects().size() == 1 ? result.get_effects()[0] : formalism::EffectView(result);
    };
    auto factored = std::optional<formalism::EffectView> {};
    if (const auto* oneof = std::get_if<formalism::EntityView<formalism::Effect<formalism::OneOfTag>>>(&choice))
    {
        for (auto outcome : oneof->get_effects())
            check(outcome);
        if (!shared || !condition)
            return choice;
        auto data = formalism::checkout<formalism::Effect<formalism::OneOfTag>>(context.builder);
        for (auto outcome : oneof->get_effects())
            urd::append(data->effects, body(outcome));
        factored = formalism::EffectView(context.storage->repository.insert(*data, context.storage->builder).first);
    }
    else if (const auto* probabilistic = std::get_if<formalism::EntityView<formalism::Effect<formalism::ProbabilisticTag>>>(&choice))
    {
        for (auto alternative : probabilistic->get_alternatives())
            check(alternative.get_effect());
        if (!shared || !condition)
            return choice;
        auto data = formalism::checkout<formalism::Effect<formalism::ProbabilisticTag>>(context.builder);
        auto alternative_data = formalism::checkout<formalism::EffectProbabilisticAlternative>(context.builder);
        for (auto alternative : probabilistic->get_alternatives())
        {
            urd::clear_fields(*alternative_data);
            alternative_data->probability = alternative.get_probability();
            urd::store(alternative_data->effect, body(alternative.get_effect()));
            data->alternatives.emplace_back(context.storage->repository.insert(*alternative_data, context.storage->builder).first.index());
        }
        factored = formalism::EffectView(context.storage->repository.insert(*data, context.storage->builder).first);
    }
    else
    {
        return choice;
    }
    auto when_data = formalism::checkout<formalism::Effect<formalism::WhenTag>>(context.builder);
    urd::store(when_data->condition, *condition);
    urd::store(when_data->effect, this->self().normalize_effect(*factored));
    return formalism::EffectView(context.storage->repository.insert(*when_data, context.storage->builder).first);
}

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
    /// The expressions of the numeric effects with one operator on one function term, in the order of their first effect.
    using NumericGroupKey = std::pair<formalism::NumericEffectOperator, urd::index<formalism::FunctionTerm>>;

    auto result = formalism::checkout<formalism::Effect<formalism::AndTag>>(this->copy_context().builder);
    auto numeric_groups = urd::Store<urd::map<NumericGroupKey, formalism::FunctionExpressionList>, urd::insertion_ordered> {};

    auto push_effect = [&](formalism::EffectView effect)
    {
        if (const auto numeric = this->self().template as_effect<formalism::Effect<formalism::NumericTag>>(effect))
        {
            urd::append(numeric_groups[NumericGroupKey(numeric->get_operation(), numeric->get_function().index())], numeric->get_expression());
            return;
        }
        urd::append(result->effects, effect);
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

    for (const auto& [key, expressions] : numeric_groups)
    {
        // A single expression stays as-is; only genuine aggregations get a sum, keeping
        // normalization idempotent across repeated translations.
        auto sum = expressions.front();
        if (expressions.size() > 1)
        {
            auto multi_data = formalism::checkout<formalism::FunctionExpression<formalism::MultiTag>>(this->copy_context().builder);
            multi_data->operation = formalism::MultiArithmeticOperator::Add;
            for (const auto& expression : expressions)
                urd::append(multi_data->args, expression);
            sum = this->copy_context().storage->repository.insert(*multi_data, this->copy_context().storage->builder).first.index();
        }
        auto numeric_data = formalism::checkout<formalism::Effect<formalism::NumericTag>>(this->copy_context().builder);
        numeric_data->operation = key.first;
        numeric_data->function = key.second;
        urd::store(numeric_data->expression, sum);
        result->effects.push_back(this->copy_context().storage->repository.insert(*numeric_data, this->copy_context().storage->builder).first.index());
    }

    return formalism::EffectView(this->copy_context().storage->repository.insert(*result, this->copy_context().storage->builder).first);
}

template<typename Derived>
formalism::EffectView ToEffectNormalFormTranslator<Derived>::normalize_effect_node(formalism::EffectView,
                                                                                   formalism::EntityView<formalism::Effect<formalism::ForallTag>> node)
{
    const auto nested = this->self().normalize_effect(node.get_effect());
    if (const auto nested_and = this->self().template as_effect<formalism::Effect<formalism::AndTag>>(nested))
    {
        auto conjunction_data = formalism::checkout<formalism::Effect<formalism::AndTag>>(this->copy_context().builder);
        for (auto part : nested_and->get_effects())
        {
            auto forall_data = formalism::checkout<formalism::Effect<formalism::ForallTag>>(this->copy_context().builder);
            for (auto parameter : node.get_parameters())
                forall_data->parameters.push_back(parameter.index());
            urd::store(forall_data->effect, part);
            const auto wrapped =
                formalism::EffectView(this->copy_context().storage->repository.insert(*forall_data, this->copy_context().storage->builder).first);
            urd::append(conjunction_data->effects, this->self().normalize_effect(wrapped));
        }
        const auto conjunction =
            formalism::EffectView(this->copy_context().storage->repository.insert(*conjunction_data, this->copy_context().storage->builder).first);
        return this->self().normalize_effect(conjunction);
    }
    if (const auto nested_forall = this->self().template as_effect<formalism::Effect<formalism::ForallTag>>(nested))
    {
        auto forall_data = formalism::checkout<formalism::Effect<formalism::ForallTag>>(this->copy_context().builder);
        for (auto parameter : node.get_parameters())
            forall_data->parameters.push_back(parameter.index());
        for (auto parameter : nested_forall->get_parameters())
            forall_data->parameters.push_back(parameter.index());
        urd::store(forall_data->effect, nested_forall->get_effect());
        const auto wrapped = formalism::EffectView(this->copy_context().storage->repository.insert(*forall_data, this->copy_context().storage->builder).first);
        return this->self().normalize_effect(wrapped);
    }
    auto forall_data = formalism::checkout<formalism::Effect<formalism::ForallTag>>(this->copy_context().builder);
    for (auto parameter : node.get_parameters())
        forall_data->parameters.push_back(parameter.index());
    urd::store(forall_data->effect, nested);
    return formalism::EffectView(this->copy_context().storage->repository.insert(*forall_data, this->copy_context().storage->builder).first);
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
            urd::store(when_data->condition, part);
            urd::store(when_data->effect, nested);
            const auto wrapped =
                formalism::EffectView(this->copy_context().storage->repository.insert(*when_data, this->copy_context().storage->builder).first);
            urd::append(conjunction_data->effects, this->self().normalize_effect(wrapped));
        }
        const auto conjunction =
            formalism::EffectView(this->copy_context().storage->repository.insert(*conjunction_data, this->copy_context().storage->builder).first);
        return this->self().normalize_effect(conjunction);
    }

    if (const auto nested_when = this->self().template as_effect<formalism::Effect<formalism::WhenTag>>(nested))
    {
        auto condition_data = formalism::checkout<formalism::Condition<formalism::AndTag>>(this->copy_context().builder);
        this->self().append_conjunct(*condition_data, condition);
        this->self().append_conjunct(*condition_data, nested_when->get_condition());
        const auto combined_condition = this->self().make_conjunction(*condition_data);
        auto when_data = formalism::checkout<formalism::Effect<formalism::WhenTag>>(this->copy_context().builder);
        urd::store(when_data->condition, combined_condition);
        urd::store(when_data->effect, nested_when->get_effect());
        const auto wrapped = formalism::EffectView(this->copy_context().storage->repository.insert(*when_data, this->copy_context().storage->builder).first);
        return this->self().normalize_effect(wrapped);
    }
    if (const auto nested_and = this->self().template as_effect<formalism::Effect<formalism::AndTag>>(nested))
    {
        auto conjunction_data = formalism::checkout<formalism::Effect<formalism::AndTag>>(this->copy_context().builder);
        for (auto part : nested_and->get_effects())
        {
            auto when_data = formalism::checkout<formalism::Effect<formalism::WhenTag>>(this->copy_context().builder);
            urd::store(when_data->condition, condition);
            urd::store(when_data->effect, part);
            const auto wrapped =
                formalism::EffectView(this->copy_context().storage->repository.insert(*when_data, this->copy_context().storage->builder).first);
            urd::append(conjunction_data->effects, this->self().normalize_effect(wrapped));
        }
        const auto conjunction =
            formalism::EffectView(this->copy_context().storage->repository.insert(*conjunction_data, this->copy_context().storage->builder).first);
        return this->self().normalize_effect(conjunction);
    }
    if (const auto nested_forall = this->self().template as_effect<formalism::Effect<formalism::ForallTag>>(nested))
    {
        auto forall_data = formalism::checkout<formalism::Effect<formalism::ForallTag>>(this->copy_context().builder);
        for (auto parameter : nested_forall->get_parameters())
            forall_data->parameters.push_back(parameter.index());
        auto when_data = formalism::checkout<formalism::Effect<formalism::WhenTag>>(this->copy_context().builder);
        urd::store(when_data->condition, condition);
        urd::store(when_data->effect, nested_forall->get_effect());
        forall_data->effect = this->copy_context().storage->repository.insert(*when_data, this->copy_context().storage->builder).first.index();
        const auto wrapped = formalism::EffectView(this->copy_context().storage->repository.insert(*forall_data, this->copy_context().storage->builder).first);
        return this->self().normalize_effect(wrapped);
    }
    if (const auto exists = this->self().as_exists(condition))
    {
        auto forall_data = formalism::checkout<formalism::Effect<formalism::ForallTag>>(this->copy_context().builder);
        for (auto parameter : exists->get_parameters())
            forall_data->parameters.push_back(parameter.index());
        auto when_data = formalism::checkout<formalism::Effect<formalism::WhenTag>>(this->copy_context().builder);
        urd::store(when_data->condition, exists->get_condition());
        urd::store(when_data->effect, nested);
        forall_data->effect = this->copy_context().storage->repository.insert(*when_data, this->copy_context().storage->builder).first.index();
        const auto wrapped = formalism::EffectView(this->copy_context().storage->repository.insert(*forall_data, this->copy_context().storage->builder).first);
        return this->self().normalize_effect(wrapped);
    }

    auto when_data = formalism::checkout<formalism::Effect<formalism::WhenTag>>(this->copy_context().builder);
    urd::store(when_data->condition, condition);
    urd::store(when_data->effect, nested);
    return formalism::EffectView(this->copy_context().storage->repository.insert(*when_data, this->copy_context().storage->builder).first);
}

template<typename Derived>
formalism::EffectView ToEffectNormalFormTranslator<Derived>::normalize_effect_node(formalism::EffectView,
                                                                                   formalism::EntityView<formalism::Effect<formalism::OneOfTag>> node)
{
    auto data = formalism::checkout<formalism::Effect<formalism::OneOfTag>>(this->copy_context().builder);
    for (auto child : node.get_effects())
    {
        const auto normalized = this->self().normalize_effect(child);
        if (const auto nested = this->self().template as_effect<formalism::Effect<formalism::OneOfTag>>(normalized))
        {
            for (auto outcome : nested->get_effects())
                urd::append(data->effects, outcome);
        }
        else
        {
            urd::append(data->effects, normalized);
        }
    }
    // Inserting drops equal outcomes: a oneof is a set of outcomes.
    const auto result = this->copy_context().storage->repository.insert(*data, this->copy_context().storage->builder).first;
    if (result.get_effects().size() == 1)
        return result.get_effects()[0];
    return this->self().factor_shared_condition(formalism::EffectView(result));
}

template<typename Derived>
formalism::EffectView ToEffectNormalFormTranslator<Derived>::normalize_effect_node(formalism::EffectView,
                                                                                   formalism::EntityView<formalism::Effect<formalism::ProbabilisticTag>> node)
{
    // The parser's tolerance for probabilities that sum to more than 1.
    constexpr auto rounding_tolerance = 1e-9;

    auto outcomes = urd::Store<urd::sequence<std::pair<formalism::EffectView, double>>> {};
    const auto add_outcome = [&](formalism::EffectView effect, double probability)
    {
        if (probability <= 0.0)
            return;
        for (auto& [outcome, mass] : outcomes)
        {
            if (outcome == effect)
            {
                mass += probability;
                return;
            }
        }
        outcomes.emplace_back(effect, probability);
    };

    for (auto alternative : node.get_alternatives())
    {
        const auto normalized = this->self().normalize_effect(alternative.get_effect());
        if (const auto nested = this->self().template as_effect<formalism::Effect<formalism::ProbabilisticTag>>(normalized))
        {
            for (auto nested_alternative : nested->get_alternatives())
                add_outcome(nested_alternative.get_effect(), alternative.get_probability() * nested_alternative.get_probability());
        }
        else
        {
            add_outcome(normalized, alternative.get_probability());
        }
    }

    auto total = 0.0;
    for (const auto& [outcome, mass] : outcomes)
        total += mass;
    if (total < 1.0 - rounding_tolerance)
    {
        auto empty_data = formalism::checkout<formalism::Effect<formalism::AndTag>>(this->copy_context().builder);
        add_outcome(formalism::EffectView(this->copy_context().storage->repository.insert(*empty_data, this->copy_context().storage->builder).first),
                    1.0 - total);
    }

    if (outcomes.size() == 1)
        return outcomes.front().first;

    auto data = formalism::checkout<formalism::Effect<formalism::ProbabilisticTag>>(this->copy_context().builder);
    auto alternative_data = formalism::checkout<formalism::EffectProbabilisticAlternative>(this->copy_context().builder);
    for (const auto& [outcome, mass] : outcomes)
    {
        urd::clear_fields(*alternative_data);
        alternative_data->probability = mass;
        urd::store(alternative_data->effect, outcome);
        data->alternatives.emplace_back(
            this->copy_context().storage->repository.insert(*alternative_data, this->copy_context().storage->builder).first.index());
    }
    return this->self().factor_shared_condition(
        formalism::EffectView(this->copy_context().storage->repository.insert(*data, this->copy_context().storage->builder).first));
}

}  // namespace loki::semantic::detail

#endif
