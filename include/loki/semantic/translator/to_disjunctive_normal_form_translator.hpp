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

#ifndef LOKI_SEMANTIC_TRANSLATOR_TO_DISJUNCTIVE_NORMAL_FORM_TRANSLATOR_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_TO_DISJUNCTIVE_NORMAL_FORM_TRANSLATOR_HPP_

#include "loki/semantic/translator/copy_translator_component.hpp"

namespace loki::semantic::detail
{

template<typename Derived>
class ToDisjunctiveNormalFormTranslator : public CopyTranslatorComponent<Derived, ToDisjunctiveNormalFormTranslator<Derived>>
{
public:
    explicit ToDisjunctiveNormalFormTranslator(CopyContext& context) : CopyTranslatorComponent<Derived, ToDisjunctiveNormalFormTranslator<Derived>>(context) {}

    formalism::ConditionView to_dnf(formalism::ConditionView condition);
    formalism::ConditionView to_dnf_node(formalism::ConditionView, formalism::ConditionOrView node);
    formalism::ConditionView to_dnf_node(formalism::ConditionView, formalism::ConditionAndView node);
    formalism::ConditionView to_dnf_node(formalism::ConditionView, formalism::ConditionExistsView node);
    formalism::ConditionView to_dnf_node(formalism::ConditionView, formalism::ConditionForallView node);
    template<typename T>
    formalism::ConditionView to_dnf_node(formalism::ConditionView condition, T);
};

template<typename Derived>
formalism::ConditionView ToDisjunctiveNormalFormTranslator<Derived>::to_dnf(formalism::ConditionView condition)
{
    return ygg::visit([&](const auto& node) { return this->self().to_dnf_node(condition, node); }, condition.get_variant());
}

template<typename Derived>
formalism::ConditionView ToDisjunctiveNormalFormTranslator<Derived>::to_dnf_node(formalism::ConditionView, formalism::ConditionOrView node)
{
    auto data = formalism::checkout<formalism::ConditionOr>(this->m_context.builder);
    for (auto child : node.get_conditions())
        this->self().append_disjunct(*data, this->self().to_dnf(child));
    return this->self().make_disjunction(*data);
}

template<typename Derived>
formalism::ConditionView ToDisjunctiveNormalFormTranslator<Derived>::to_dnf_node(formalism::ConditionView, formalism::ConditionAndView node)
{
    auto combinations = std::vector<std::vector<formalism::ConditionView>> { {} };
    for (auto child : node.get_conditions())
    {
        const auto dnf = this->self().to_dnf(child);
        auto alternatives = std::vector<formalism::ConditionView> {};
        if (const auto child_or = this->self().as_or(dnf))
        {
            for (auto nested : child_or->get_conditions())
                alternatives.push_back(nested);
        }
        else
        {
            alternatives.push_back(dnf);
        }
        auto next = std::vector<std::vector<formalism::ConditionView>> {};
        for (const auto& combination : combinations)
        {
            for (auto alternative : alternatives)
            {
                auto extended = combination;
                extended.push_back(alternative);
                next.push_back(std::move(extended));
            }
        }
        combinations = std::move(next);
    }
    if (combinations.size() == 1)
    {
        auto data = formalism::checkout<formalism::ConditionAnd>(this->m_context.builder);
        for (auto condition : combinations.front())
            this->self().append_conjunct(*data, condition);
        return this->self().make_conjunction(*data);
    }
    auto data = formalism::checkout<formalism::ConditionOr>(this->m_context.builder);
    for (const auto& combination : combinations)
    {
        auto conjunction_data = formalism::checkout<formalism::ConditionAnd>(this->m_context.builder);
        for (auto condition : combination)
            this->self().append_conjunct(*conjunction_data, condition);
        this->self().append_disjunct(*data, this->self().make_conjunction(*conjunction_data));
    }
    return this->self().make_disjunction(*data);
}

template<typename Derived>
formalism::ConditionView ToDisjunctiveNormalFormTranslator<Derived>::to_dnf_node(formalism::ConditionView, formalism::ConditionExistsView node)
{
    const auto& data = node.get_data();
    const auto child = this->self().to_dnf(node.get_condition());
    if (const auto child_or = this->self().as_or(child))
    {
        auto condition_data = formalism::checkout<formalism::ConditionOr>(this->m_context.builder);
        for (auto nested : child_or->get_conditions())
        {
            auto result = formalism::checkout<formalism::ConditionExists>(this->m_context.builder);
            for (auto parameter : data.parameters)
                result->parameters.push_back(parameter);
            result->condition = nested.get_index();
            const auto exists = this->self().wrap_condition(formalism::insert(this->m_storage->repository, *result).first);
            this->self().append_disjunct(*condition_data, exists);
        }
        return this->self().to_dnf(this->self().make_disjunction(*condition_data));
    }
    auto result = formalism::checkout<formalism::ConditionExists>(this->m_context.builder);
    for (auto parameter : data.parameters)
        result->parameters.push_back(parameter);
    result->condition = child.get_index();
    const auto exists = this->self().wrap_condition(formalism::insert(this->m_storage->repository, *result).first);
    return this->self().flatten_condition(exists);
}

template<typename Derived>
formalism::ConditionView ToDisjunctiveNormalFormTranslator<Derived>::to_dnf_node(formalism::ConditionView, formalism::ConditionForallView node)
{
    const auto& data = node.get_data();
    const auto child = this->self().to_dnf(node.get_condition());
    if (const auto child_or = this->self().as_or(child))
    {
        auto condition_data = formalism::checkout<formalism::ConditionOr>(this->m_context.builder);
        for (auto nested : child_or->get_conditions())
        {
            auto result = formalism::checkout<formalism::ConditionForall>(this->m_context.builder);
            for (auto parameter : data.parameters)
                result->parameters.push_back(parameter);
            result->condition = nested.get_index();
            const auto forall = this->self().wrap_condition(formalism::insert(this->m_storage->repository, *result).first);
            this->self().append_disjunct(*condition_data, forall);
        }
        return this->self().to_dnf(this->self().make_disjunction(*condition_data));
    }
    auto result = formalism::checkout<formalism::ConditionForall>(this->m_context.builder);
    for (auto parameter : data.parameters)
        result->parameters.push_back(parameter);
    result->condition = child.get_index();
    const auto forall = this->self().wrap_condition(formalism::insert(this->m_storage->repository, *result).first);
    return this->self().flatten_condition(forall);
}

template<typename Derived>
template<typename T>
formalism::ConditionView ToDisjunctiveNormalFormTranslator<Derived>::to_dnf_node(formalism::ConditionView condition, T)
{
    return condition;
}

}  // namespace loki::semantic::detail

#endif
