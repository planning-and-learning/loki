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

#ifndef LOKI_SEMANTIC_TRANSLATOR_MOVE_EXISTENTIAL_QUANTIFIERS_TRANSLATOR_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_MOVE_EXISTENTIAL_QUANTIFIERS_TRANSLATOR_HPP_

#include "loki/semantic/translator/copy_translator_component.hpp"

namespace loki::semantic::detail
{

template<typename Derived>
class MoveExistentialQuantifiersTranslator : public CopyTranslatorComponent<Derived, MoveExistentialQuantifiersTranslator<Derived>>
{
public:
    explicit MoveExistentialQuantifiersTranslator(CopyContext& context) :
        CopyTranslatorComponent<Derived, MoveExistentialQuantifiersTranslator<Derived>>(context)
    {
    }

    std::optional<formalism::EntityView<formalism::Condition<formalism::ExistsTag>>> as_exists(formalism::ConditionView condition) const;
    formalism::ConditionView move_existentials(formalism::ConditionView condition);
    formalism::ConditionView move_existentials_node(formalism::ConditionView condition, formalism::EntityView<formalism::Condition<formalism::AndTag>> node);
    formalism::ConditionView move_existentials_node(formalism::ConditionView condition, formalism::EntityView<formalism::Condition<formalism::ExistsTag>> node);
    formalism::ConditionView move_existentials_node(formalism::ConditionView condition, formalism::EntityView<formalism::Condition<formalism::OrTag>> node);
    template<typename T>
    formalism::ConditionView move_existentials_node(formalism::ConditionView condition, formalism::EntityView<T>);
    formalism::ConditionView
    lift_top_level_exists(std::vector<formalism::ParameterView>& parameters, formalism::ConditionView condition, std::optional<formalism::EffectView> effect);

private:
    // Collects the names of all binders occurring in a subtree; lifted parameters must avoid
    // them so that no binder shadows an action parameter after the lift.
    void collect_binder_names(formalism::ConditionView condition, ygg::UnorderedSet<std::string>& names) const;
    void collect_binder_names(formalism::EffectView effect, ygg::UnorderedSet<std::string>& names) const;

    // Hoists the binders of an exists into parameters/claimed; a binder whose name is already
    // claimed by a sibling scope is alpha-renamed together with its bound occurrences. Fresh
    // names avoid the claimed names and the free variables of the body, so no capture is possible.
    formalism::ConditionView
    hoist_exists(formalism::EntityView<formalism::Condition<formalism::ExistsTag>> exists, ygg::UnorderedSet<std::string>& claimed, std::vector<formalism::ParameterView>& parameters);
};

template<typename Derived>
std::optional<formalism::EntityView<formalism::Condition<formalism::ExistsTag>>> MoveExistentialQuantifiersTranslator<Derived>::as_exists(formalism::ConditionView condition) const
{
    const auto variant = condition.get_variant();
    if (variant.template is<ygg::Index<formalism::Condition<formalism::ExistsTag>>>())
        return variant.template get<ygg::Index<formalism::Condition<formalism::ExistsTag>>>();
    return std::nullopt;
}

template<typename Derived>
formalism::ConditionView MoveExistentialQuantifiersTranslator<Derived>::move_existentials(formalism::ConditionView condition)
{
    return ygg::visit([&](const auto& node) { return this->self().move_existentials_node(condition, node); }, condition.get_variant());
}

template<typename Derived>
void MoveExistentialQuantifiersTranslator<Derived>::collect_binder_names(formalism::ConditionView condition, ygg::UnorderedSet<std::string>& names) const
{
    ygg::visit(
        [&](const auto& node)
        {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::ExistsTag>>> || std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::ForallTag>>>)
            {
                for (auto parameter : node.get_parameters())
                    names.insert(std::string(parameter.get_variable().get_name()));
                collect_binder_names(node.get_condition(), names);
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::AndTag>>> || std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::OrTag>>>)
            {
                for (auto child : node.get_conditions())
                    collect_binder_names(child, names);
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::NotTag>>>)
            {
                collect_binder_names(node.get_condition(), names);
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::ImplyTag>>>)
            {
                collect_binder_names(node.get_left(), names);
                collect_binder_names(node.get_right(), names);
            }
        },
        condition.get_variant());
}

template<typename Derived>
void MoveExistentialQuantifiersTranslator<Derived>::collect_binder_names(formalism::EffectView effect, ygg::UnorderedSet<std::string>& names) const
{
    ygg::visit(
        [&](const auto& node)
        {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::ForallTag>>>)
            {
                for (auto parameter : node.get_parameters())
                    names.insert(std::string(parameter.get_variable().get_name()));
                collect_binder_names(node.get_effect(), names);
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::AndTag>>> || std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::OneOfTag>>>)
            {
                for (auto child : node.get_effects())
                    collect_binder_names(child, names);
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::WhenTag>>>)
            {
                collect_binder_names(node.get_condition(), names);
                collect_binder_names(node.get_effect(), names);
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Effect<formalism::ProbabilisticTag>>>)
            {
                for (auto alternative : node.get_alternatives())
                    collect_binder_names(alternative.get_effect(), names);
            }
        },
        effect.get_variant());
}

template<typename Derived>
formalism::ConditionView MoveExistentialQuantifiersTranslator<Derived>::hoist_exists(formalism::EntityView<formalism::Condition<formalism::ExistsTag>> exists,
                                                                                     ygg::UnorderedSet<std::string>& claimed,
                                                                                     std::vector<formalism::ParameterView>& parameters)
{
    auto condition = exists.get_condition();
    for (auto parameter : exists.get_parameters())
    {
        const auto variable = parameter.get_variable();
        if (claimed.insert(std::string(variable.get_name())).second)
        {
            parameters.push_back(parameter);
            continue;
        }

        auto avoid = claimed;
        auto bound = ygg::UnorderedSet<formalism::VariableView> {};
        auto free = ygg::UnorderedSet<formalism::VariableView> {};
        this->self().collect_free_variables(condition, bound, free);
        for (auto free_variable : free)
            avoid.insert(std::string(free_variable.get_name()));

        auto name = std::string {};
        for (auto k = std::size_t { 0 };; ++k)
        {
            name = std::string(variable.get_name()) + "_" + std::to_string(k);
            if (!avoid.contains(name))
                break;
        }
        auto variable_data = formalism::checkout<formalism::Variable>(this->m_context.builder);
        variable_data->name = cista::offset::string(name);
        const auto fresh = formalism::insert(this->m_context.storage->repository, *variable_data).first;
        this->self().enter_variable_scope();
        this->m_context.variable_bindings.back().emplace(variable, fresh);
        condition = this->self().rename_variables(condition);
        this->self().leave_variable_scope();
        claimed.insert(name);
        auto parameter_data = formalism::checkout<formalism::Parameter>(this->m_context.builder);
        parameter_data->variable = fresh.get_index();
        for (auto type : parameter.get_data().types)
            parameter_data->types.push_back(type);
        parameters.push_back(formalism::insert(this->m_context.storage->repository, *parameter_data).first);
    }
    return condition;
}

template<typename Derived>
formalism::ConditionView MoveExistentialQuantifiersTranslator<Derived>::move_existentials_node(formalism::ConditionView, formalism::EntityView<formalism::Condition<formalism::AndTag>> node)
{
    auto parameters = std::vector<formalism::ParameterView> {};
    auto claimed = ygg::UnorderedSet<std::string> {};
    auto condition_data = formalism::checkout<formalism::Condition<formalism::AndTag>>(this->m_context.builder);
    for (auto child : node.get_conditions())
    {
        const auto moved = this->self().move_existentials(child);
        if (const auto exists = this->self().as_exists(moved))
        {
            this->self().append_conjunct(*condition_data, this->self().hoist_exists(*exists, claimed, parameters));
        }
        else
        {
            this->self().append_conjunct(*condition_data, moved);
        }
    }
    auto conjunction = this->self().make_conjunction(*condition_data);
    if (parameters.empty())
        return conjunction;
    auto data = formalism::checkout<formalism::Condition<formalism::ExistsTag>>(this->m_context.builder);
    for (auto parameter : parameters)
        data->parameters.push_back(parameter.get_index());
    data->condition = conjunction.get_index();
    return this->self().flatten_condition(this->self().wrap_condition(formalism::insert(this->m_context.storage->repository, *data).first));
}

template<typename Derived>
formalism::ConditionView MoveExistentialQuantifiersTranslator<Derived>::move_existentials_node(formalism::ConditionView, formalism::EntityView<formalism::Condition<formalism::ExistsTag>> node)
{
    const auto& data = node.get_data();
    const auto condition = this->self().move_existentials(node.get_condition());
    auto result = formalism::checkout<formalism::Condition<formalism::ExistsTag>>(this->m_context.builder);
    for (auto parameter : data.parameters)
        result->parameters.push_back(parameter);
    result->condition = condition.get_index();
    return this->self().flatten_condition(this->self().wrap_condition(formalism::insert(this->m_context.storage->repository, *result).first));
}

template<typename Derived>
formalism::ConditionView MoveExistentialQuantifiersTranslator<Derived>::move_existentials_node(formalism::ConditionView, formalism::EntityView<formalism::Condition<formalism::OrTag>> node)
{
    auto data = formalism::checkout<formalism::Condition<formalism::OrTag>>(this->m_context.builder);
    for (auto child : node.get_conditions())
        this->self().append_disjunct(*data, this->self().move_existentials(child));
    return this->self().make_disjunction(*data);
}

template<typename Derived>
template<typename T>
formalism::ConditionView MoveExistentialQuantifiersTranslator<Derived>::move_existentials_node(formalism::ConditionView condition, formalism::EntityView<T>)
{
    return condition;
}

template<typename Derived>
formalism::ConditionView MoveExistentialQuantifiersTranslator<Derived>::lift_top_level_exists(std::vector<formalism::ParameterView>& parameters,
                                                                                              formalism::ConditionView condition,
                                                                                              std::optional<formalism::EffectView> effect)
{
    auto moved = this->self().move_existentials(condition);
    if (const auto exists = this->self().as_exists(moved))
    {
        auto claimed = ygg::UnorderedSet<std::string> {};
        for (auto parameter : parameters)
            claimed.insert(std::string(parameter.get_variable().get_name()));
        // Lifted parameters must not shadow-clash with binders that stay behind.
        collect_binder_names(exists->get_condition(), claimed);
        if (effect)
            collect_binder_names(*effect, claimed);

        moved = this->self().hoist_exists(*exists, claimed, parameters);
    }
    return this->self().flatten_condition(moved);
}

}  // namespace loki::semantic::detail

#endif
