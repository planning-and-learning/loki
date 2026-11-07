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

    [[nodiscard]] std::optional<formalism::EntityView<formalism::Condition<formalism::OrTag>>> as_or(formalism::ConditionView condition) const;
    formalism::ConditionView flatten_condition(formalism::ConditionView condition);
    void append_conjunct(urd::data<formalism::Condition<formalism::AndTag>>& data, formalism::ConditionView condition);
    void append_disjunct(urd::data<formalism::Condition<formalism::OrTag>>& data, formalism::ConditionView condition);
    formalism::ConditionView make_conjunction(urd::data<formalism::Condition<formalism::AndTag>>& data);
    formalism::ConditionView make_disjunction(urd::data<formalism::Condition<formalism::OrTag>>& data);
};

template<typename Derived>
std::optional<formalism::EntityView<formalism::Condition<formalism::OrTag>>>
ConditionBuilderTranslator<Derived>::as_or(formalism::ConditionView condition) const
{
    if (std::holds_alternative<formalism::EntityView<formalism::Condition<formalism::OrTag>>>(condition))
        return std::get<formalism::EntityView<formalism::Condition<formalism::OrTag>>>(condition);
    return std::nullopt;
}

template<typename Derived>
formalism::ConditionView ConditionBuilderTranslator<Derived>::flatten_condition(formalism::ConditionView condition)
{
    return std::visit(
        [&]<typename Node>(const Node& node) -> formalism::ConditionView
        {
            if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::AndTag>>>)
            {
                auto data = formalism::checkout<formalism::Condition<formalism::AndTag>>(this->copy_context().builder);
                for (auto child : node.get_conditions())
                {
                    const auto flat = this->self().flatten_condition(child);
                    std::visit(
                        [&]<typename FlatNode>(const FlatNode& flat_node)
                        {
                            if constexpr (std::is_same_v<FlatNode, formalism::EntityView<formalism::Condition<formalism::AndTag>>>)
                            {
                                for (auto part : flat_node.get_conditions())
                                    data->conditions.push_back(urd::to_stored(part));
                            }
                            else
                            {
                                data->conditions.push_back(urd::to_stored(flat));
                            }
                        },
                        flat);
                }
                return formalism::ConditionView(formalism::insert(this->copy_context().storage->repository, *data).first);
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::OrTag>>>)
            {
                auto data = formalism::checkout<formalism::Condition<formalism::OrTag>>(this->copy_context().builder);
                for (auto child : node.get_conditions())
                {
                    const auto flat = this->self().flatten_condition(child);
                    std::visit(
                        [&]<typename FlatNode>(const FlatNode& flat_node)
                        {
                            if constexpr (std::is_same_v<FlatNode, formalism::EntityView<formalism::Condition<formalism::OrTag>>>)
                            {
                                for (auto part : flat_node.get_conditions())
                                    data->conditions.push_back(urd::to_stored(part));
                            }
                            else
                            {
                                data->conditions.push_back(urd::to_stored(flat));
                            }
                        },
                        flat);
                }
                return formalism::ConditionView(formalism::insert(this->copy_context().storage->repository, *data).first);
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::ExistsTag>>>)
            {
                const auto flat = this->self().flatten_condition(node.get_condition());
                return std::visit(
                    [&]<typename FlatNode>(const FlatNode& flat_node) -> formalism::ConditionView
                    {
                        if constexpr (std::is_same_v<FlatNode, formalism::EntityView<formalism::Condition<formalism::ExistsTag>>>)
                        {
                            auto data = formalism::checkout<formalism::Condition<formalism::ExistsTag>>(this->copy_context().builder);
                            for (auto parameter : node.get_parameters())
                                data->parameters.push_back(parameter.index());
                            for (auto parameter : flat_node.get_parameters())
                                data->parameters.push_back(parameter.index());
                            data->condition = urd::to_stored(flat_node.get_condition());
                            return formalism::ConditionView(formalism::insert(this->copy_context().storage->repository, *data).first);
                        }
                        else
                        {
                            auto data = formalism::checkout<formalism::Condition<formalism::ExistsTag>>(this->copy_context().builder);
                            for (auto parameter : node.get_parameters())
                                data->parameters.push_back(parameter.index());
                            data->condition = urd::to_stored(flat);
                            return formalism::ConditionView(formalism::insert(this->copy_context().storage->repository, *data).first);
                        }
                    },
                    flat);
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::ForallTag>>>)
            {
                const auto flat = this->self().flatten_condition(node.get_condition());
                return std::visit(
                    [&]<typename FlatNode>(const FlatNode& flat_node) -> formalism::ConditionView
                    {
                        if constexpr (std::is_same_v<FlatNode, formalism::EntityView<formalism::Condition<formalism::ForallTag>>>)
                        {
                            auto data = formalism::checkout<formalism::Condition<formalism::ForallTag>>(this->copy_context().builder);
                            for (auto parameter : node.get_parameters())
                                data->parameters.push_back(parameter.index());
                            for (auto parameter : flat_node.get_parameters())
                                data->parameters.push_back(parameter.index());
                            data->condition = urd::to_stored(flat_node.get_condition());
                            return formalism::ConditionView(formalism::insert(this->copy_context().storage->repository, *data).first);
                        }
                        else
                        {
                            auto data = formalism::checkout<formalism::Condition<formalism::ForallTag>>(this->copy_context().builder);
                            for (auto parameter : node.get_parameters())
                                data->parameters.push_back(parameter.index());
                            data->condition = urd::to_stored(flat);
                            return formalism::ConditionView(formalism::insert(this->copy_context().storage->repository, *data).first);
                        }
                    },
                    flat);
            }
            else
            {
                return condition;
            }
        },
        condition);
}

template<typename Derived>
void ConditionBuilderTranslator<Derived>::append_conjunct(urd::data<formalism::Condition<formalism::AndTag>>& data, formalism::ConditionView condition)
{
    std::visit(
        [&]<typename Node>(const Node& node)
        {
            if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::AndTag>>>)
            {
                for (auto part : node.get_conditions())
                    this->self().append_conjunct(data, part);
            }
            else
            {
                data.conditions.push_back(urd::to_stored(this->self().flatten_condition(condition)));
            }
        },
        condition);
}

template<typename Derived>
void ConditionBuilderTranslator<Derived>::append_disjunct(urd::data<formalism::Condition<formalism::OrTag>>& data, formalism::ConditionView condition)
{
    std::visit(
        [&]<typename Node>(const Node& node)
        {
            if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::OrTag>>>)
            {
                for (auto part : node.get_conditions())
                    this->self().append_disjunct(data, part);
            }
            else
            {
                data.conditions.push_back(urd::to_stored(this->self().flatten_condition(condition)));
            }
        },
        condition);
}

template<typename Derived>
formalism::ConditionView ConditionBuilderTranslator<Derived>::make_conjunction(urd::data<formalism::Condition<formalism::AndTag>>& data)
{ return formalism::ConditionView(formalism::insert(this->copy_context().storage->repository, data).first); }

template<typename Derived>
formalism::ConditionView ConditionBuilderTranslator<Derived>::make_disjunction(urd::data<formalism::Condition<formalism::OrTag>>& data)
{ return formalism::ConditionView(formalism::insert(this->copy_context().storage->repository, data).first); }

}  // namespace loki::semantic::detail

#endif
