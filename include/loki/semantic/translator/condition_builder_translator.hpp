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

#ifndef LOKI_SEMANTIC_TRANSLATOR_CONDITION_BUILDER_TRANSLATOR_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_CONDITION_BUILDER_TRANSLATOR_HPP_

#include "loki/semantic/translator/copy_translator_component.hpp"

namespace loki::semantic::detail
{

template<typename Derived>
class ConditionBuilderTranslator : public CopyTranslatorComponent<Derived, ConditionBuilderTranslator<Derived>>
{
public:
    explicit ConditionBuilderTranslator(CopyContext& context) : CopyTranslatorComponent<Derived, ConditionBuilderTranslator<Derived>>(context) {}

    formalism::ConditionView wrap_condition(ygg::Data<formalism::Condition>::Variant value);
    template<typename T>
    formalism::ConditionView wrap_condition(formalism::EntityView<T> value);
    template<typename T>
    formalism::ConditionView wrap_condition(ygg::Index<T> value);
    std::optional<formalism::ConditionOrView> as_or(formalism::ConditionView condition) const;
    formalism::ConditionView flatten_condition(formalism::ConditionView condition);
    void append_conjunct(ygg::Data<formalism::ConditionAnd>& data, formalism::ConditionView condition);
    void append_disjunct(ygg::Data<formalism::ConditionOr>& data, formalism::ConditionView condition);
    formalism::ConditionView make_conjunction(ygg::Data<formalism::ConditionAnd>& data);
    formalism::ConditionView make_disjunction(ygg::Data<formalism::ConditionOr>& data);
};

template<typename Derived>
formalism::ConditionView ConditionBuilderTranslator<Derived>::wrap_condition(ygg::Data<formalism::Condition>::Variant value)
{
    auto data = formalism::checkout<formalism::Condition>(this->m_context.builder);
    data->variant = std::move(value);
    return formalism::insert(this->m_context.storage->repository, *data).first;
}

template<typename Derived>
template<typename T>
formalism::ConditionView ConditionBuilderTranslator<Derived>::wrap_condition(formalism::EntityView<T> value)
{
    auto data = formalism::checkout<formalism::Condition>(this->m_context.builder);
    data->variant = ygg::Data<formalism::Condition>::Variant(value.get_index());
    return formalism::insert(this->m_context.storage->repository, *data).first;
}

template<typename Derived>
template<typename T>
formalism::ConditionView ConditionBuilderTranslator<Derived>::wrap_condition(ygg::Index<T> value)
{
    return this->self().wrap_condition(ygg::Data<formalism::Condition>::Variant(value));
}

template<typename Derived>
std::optional<formalism::ConditionOrView> ConditionBuilderTranslator<Derived>::as_or(formalism::ConditionView condition) const
{
    const auto variant = condition.get_variant();
    if (variant.template is<ygg::Index<formalism::ConditionOr>>())
        return variant.template get<ygg::Index<formalism::ConditionOr>>();
    return std::nullopt;
}

template<typename Derived>
formalism::ConditionView ConditionBuilderTranslator<Derived>::flatten_condition(formalism::ConditionView condition)
{
    return ygg::visit(
        [&](const auto& node) -> formalism::ConditionView
        {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, formalism::ConditionAndView>)
            {
                auto data = formalism::checkout<formalism::ConditionAnd>(this->m_context.builder);
                for (auto child : node.get_conditions())
                {
                    const auto flat = this->self().flatten_condition(child);
                    ygg::visit(
                        [&](const auto& flat_node)
                        {
                            using FlatNode = std::decay_t<decltype(flat_node)>;
                            if constexpr (std::is_same_v<FlatNode, formalism::ConditionAndView>)
                            {
                                for (auto part : flat_node.get_conditions())
                                    data->conditions.push_back(part.get_index());
                            }
                            else
                            {
                                data->conditions.push_back(flat.get_index());
                            }
                        },
                        flat.get_variant());
                }
                return this->self().wrap_condition(formalism::insert(this->m_context.storage->repository, *data).first);
            }
            else if constexpr (std::is_same_v<Node, formalism::ConditionOrView>)
            {
                auto data = formalism::checkout<formalism::ConditionOr>(this->m_context.builder);
                for (auto child : node.get_conditions())
                {
                    const auto flat = this->self().flatten_condition(child);
                    ygg::visit(
                        [&](const auto& flat_node)
                        {
                            using FlatNode = std::decay_t<decltype(flat_node)>;
                            if constexpr (std::is_same_v<FlatNode, formalism::ConditionOrView>)
                            {
                                for (auto part : flat_node.get_conditions())
                                    data->conditions.push_back(part.get_index());
                            }
                            else
                            {
                                data->conditions.push_back(flat.get_index());
                            }
                        },
                        flat.get_variant());
                }
                return this->self().wrap_condition(formalism::insert(this->m_context.storage->repository, *data).first);
            }
            else if constexpr (std::is_same_v<Node, formalism::ConditionExistsView>)
            {
                const auto flat = this->self().flatten_condition(node.get_condition());
                return ygg::visit(
                    [&](const auto& flat_node) -> formalism::ConditionView
                    {
                        using FlatNode = std::decay_t<decltype(flat_node)>;
                        if constexpr (std::is_same_v<FlatNode, formalism::ConditionExistsView>)
                        {
                            auto data = formalism::checkout<formalism::ConditionExists>(this->m_context.builder);
                            for (auto parameter : node.get_parameters())
                                data->parameters.push_back(parameter.get_index());
                            for (auto parameter : flat_node.get_parameters())
                                data->parameters.push_back(parameter.get_index());
                            data->condition = flat_node.get_condition().get_index();
                            return this->self().wrap_condition(formalism::insert(this->m_context.storage->repository, *data).first);
                        }
                        else
                        {
                            auto data = formalism::checkout<formalism::ConditionExists>(this->m_context.builder);
                            for (auto parameter : node.get_parameters())
                                data->parameters.push_back(parameter.get_index());
                            data->condition = flat.get_index();
                            return this->self().wrap_condition(formalism::insert(this->m_context.storage->repository, *data).first);
                        }
                    },
                    flat.get_variant());
            }
            else if constexpr (std::is_same_v<Node, formalism::ConditionForallView>)
            {
                const auto flat = this->self().flatten_condition(node.get_condition());
                return ygg::visit(
                    [&](const auto& flat_node) -> formalism::ConditionView
                    {
                        using FlatNode = std::decay_t<decltype(flat_node)>;
                        if constexpr (std::is_same_v<FlatNode, formalism::ConditionForallView>)
                        {
                            auto data = formalism::checkout<formalism::ConditionForall>(this->m_context.builder);
                            for (auto parameter : node.get_parameters())
                                data->parameters.push_back(parameter.get_index());
                            for (auto parameter : flat_node.get_parameters())
                                data->parameters.push_back(parameter.get_index());
                            data->condition = flat_node.get_condition().get_index();
                            return this->self().wrap_condition(formalism::insert(this->m_context.storage->repository, *data).first);
                        }
                        else
                        {
                            auto data = formalism::checkout<formalism::ConditionForall>(this->m_context.builder);
                            for (auto parameter : node.get_parameters())
                                data->parameters.push_back(parameter.get_index());
                            data->condition = flat.get_index();
                            return this->self().wrap_condition(formalism::insert(this->m_context.storage->repository, *data).first);
                        }
                    },
                    flat.get_variant());
            }
            else
            {
                return condition;
            }
        },
        condition.get_variant());
}

template<typename Derived>
void ConditionBuilderTranslator<Derived>::append_conjunct(ygg::Data<formalism::ConditionAnd>& data, formalism::ConditionView condition)
{
    ygg::visit(
        [&](const auto& node)
        {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, formalism::ConditionAndView>)
            {
                for (auto part : node.get_conditions())
                    this->self().append_conjunct(data, part);
            }
            else
            {
                data.conditions.push_back(this->self().flatten_condition(condition).get_index());
            }
        },
        condition.get_variant());
}

template<typename Derived>
void ConditionBuilderTranslator<Derived>::append_disjunct(ygg::Data<formalism::ConditionOr>& data, formalism::ConditionView condition)
{
    ygg::visit(
        [&](const auto& node)
        {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, formalism::ConditionOrView>)
            {
                for (auto part : node.get_conditions())
                    this->self().append_disjunct(data, part);
            }
            else
            {
                data.conditions.push_back(this->self().flatten_condition(condition).get_index());
            }
        },
        condition.get_variant());
}

template<typename Derived>
formalism::ConditionView ConditionBuilderTranslator<Derived>::make_conjunction(ygg::Data<formalism::ConditionAnd>& data)
{
    return this->self().wrap_condition(formalism::insert(this->m_context.storage->repository, data).first);
}

template<typename Derived>
formalism::ConditionView ConditionBuilderTranslator<Derived>::make_disjunction(ygg::Data<formalism::ConditionOr>& data)
{
    return this->self().wrap_condition(formalism::insert(this->m_context.storage->repository, data).first);
}

}  // namespace loki::semantic::detail

#endif
