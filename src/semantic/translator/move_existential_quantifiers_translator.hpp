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

#include "copy_translator_component.hpp"

#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

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

    [[nodiscard]] std::optional<formalism::EntityView<formalism::Condition<formalism::ExistsTag>>> as_exists(formalism::ConditionView condition) const;
    formalism::ConditionView move_existentials(formalism::ConditionView condition);
    formalism::ConditionView move_existentials_node(formalism::ConditionView condition, formalism::EntityView<formalism::Condition<formalism::AndTag>> node);
    formalism::ConditionView move_existentials_node(formalism::ConditionView condition, formalism::EntityView<formalism::Condition<formalism::ExistsTag>> node);
    formalism::ConditionView move_existentials_node(formalism::ConditionView condition, formalism::EntityView<formalism::Condition<formalism::OrTag>> node);
    template<typename T>
    formalism::ConditionView move_existentials_node(formalism::ConditionView condition, formalism::EntityView<T>);
    formalism::ConditionView lift_top_level_exists(urd::Store<urd::sequence<formalism::ParameterView>>& parameters,
                                                   formalism::ConditionView condition,
                                                   std::optional<formalism::EffectView> effect);

private:
    // Collects the names of all binders occurring in a subtree; lifted parameters must avoid
    // them so that no binder shadows an action parameter after the lift.
    template<typename T>
    void collect_binder_names(formalism::EntityView<T> node, urd::Store<urd::set<std::string>>& names) const;
    template<typename... Vs>
    void collect_binder_names(const std::variant<Vs...>& node, urd::Store<urd::set<std::string>>& names) const
    {
        std::visit([&](auto alternative) { collect_binder_names(alternative, names); }, node);
    }
    /// Below a condition or an effect, the parameters are those of its binders.
    void collect_binder_names(formalism::ParameterView parameter, urd::Store<urd::set<std::string>>& names) const
    { names.insert(std::string(parameter.get_variable().get_name())); }
    /// A declaration's parameters bind nothing in the subtree.
    void collect_binder_names(formalism::PredicateView, urd::Store<urd::set<std::string>>&) const {}
    void collect_binder_names(formalism::FunctionSkeletonView, urd::Store<urd::set<std::string>>&) const {}

    // Hoists the binders of an exists into parameters/claimed; a binder whose name is already
    // claimed by a sibling scope is alpha-renamed together with its bound occurrences. Fresh
    // names avoid the claimed names and the free variables of the body, so no capture is possible.
    formalism::ConditionView hoist_exists(formalism::EntityView<formalism::Condition<formalism::ExistsTag>> exists,
                                          urd::Store<urd::set<std::string>>& claimed,
                                          urd::Store<urd::sequence<formalism::ParameterView>>& parameters);
};

template<typename Derived>
std::optional<formalism::EntityView<formalism::Condition<formalism::ExistsTag>>>
MoveExistentialQuantifiersTranslator<Derived>::as_exists(formalism::ConditionView condition) const
{
    if (std::holds_alternative<formalism::EntityView<formalism::Condition<formalism::ExistsTag>>>(condition))
        return std::get<formalism::EntityView<formalism::Condition<formalism::ExistsTag>>>(condition);
    return std::nullopt;
}

template<typename Derived>
formalism::ConditionView MoveExistentialQuantifiersTranslator<Derived>::move_existentials(formalism::ConditionView condition)
{
    return std::visit([&](const auto& node) { return this->self().move_existentials_node(condition, node); }, condition);
}

template<typename Derived>
template<typename T>
void MoveExistentialQuantifiersTranslator<Derived>::collect_binder_names(formalism::EntityView<T> node, urd::Store<urd::set<std::string>>& names) const
{
    urd::for_each_child<formalism::SymbolRepositoryTypes, T>(node.data(), node.context(), [&](auto child) { collect_binder_names(child, names); });
}

template<typename Derived>
formalism::ConditionView MoveExistentialQuantifiersTranslator<Derived>::hoist_exists(formalism::EntityView<formalism::Condition<formalism::ExistsTag>> exists,
                                                                                     urd::Store<urd::set<std::string>>& claimed,
                                                                                     urd::Store<urd::sequence<formalism::ParameterView>>& parameters)
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
        auto bound = urd::Store<urd::set<formalism::VariableView>> {};
        auto free = urd::Store<urd::set<formalism::VariableView>> {};
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
        auto variable_data = formalism::checkout<formalism::Variable>(this->copy_context().builder);
        variable_data->name = urd::flat_string(name);
        const auto fresh = formalism::insert(this->copy_context().storage->repository, *variable_data).first;
        this->self().enter_variable_scope();
        this->self().bind_variable(variable, fresh);
        condition = this->self().rename_variables(condition);
        this->self().leave_variable_scope();
        claimed.insert(name);
        auto parameter_data = formalism::checkout<formalism::Parameter>(this->copy_context().builder);
        parameter_data->variable = fresh.index();
        for (auto type : parameter.data().types)
            parameter_data->types.push_back(type);
        parameters.push_back(formalism::insert(this->copy_context().storage->repository, *parameter_data).first);
    }
    return condition;
}

template<typename Derived>
formalism::ConditionView
MoveExistentialQuantifiersTranslator<Derived>::move_existentials_node(formalism::ConditionView,
                                                                      formalism::EntityView<formalism::Condition<formalism::AndTag>> node)
{
    auto parameters = urd::Store<urd::sequence<formalism::ParameterView>> {};
    auto claimed = urd::Store<urd::set<std::string>> {};
    auto condition_data = formalism::checkout<formalism::Condition<formalism::AndTag>>(this->copy_context().builder);
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
    auto data = formalism::checkout<formalism::Condition<formalism::ExistsTag>>(this->copy_context().builder);
    for (auto parameter : parameters)
        data->parameters.push_back(parameter.index());
    data->condition = urd::to_stored(conjunction);
    return this->self().flatten_condition(formalism::ConditionView(formalism::insert(this->copy_context().storage->repository, *data).first));
}

template<typename Derived>
formalism::ConditionView
MoveExistentialQuantifiersTranslator<Derived>::move_existentials_node(formalism::ConditionView,
                                                                      formalism::EntityView<formalism::Condition<formalism::ExistsTag>> node)
{
    // No child types: the parameters are kept as they are, only the condition slot is mapped.
    auto data = urd::map_variant_children<boost::mp11::mp_list<>, formalism::Condition<formalism::ExistsTag>>(
        node.data(),
        node.context(),
        [&](formalism::ConditionView condition) { return this->self().move_existentials(condition); });
    return this->self().flatten_condition(formalism::ConditionView(formalism::insert(this->copy_context().storage->repository, data).first));
}

template<typename Derived>
formalism::ConditionView
MoveExistentialQuantifiersTranslator<Derived>::move_existentials_node(formalism::ConditionView,
                                                                      formalism::EntityView<formalism::Condition<formalism::OrTag>> node)
{
    auto data = formalism::checkout<formalism::Condition<formalism::OrTag>>(this->copy_context().builder);
    for (auto child : node.get_conditions())
        this->self().append_disjunct(*data, this->self().move_existentials(child));
    return this->self().make_disjunction(*data);
}

template<typename Derived>
template<typename T>
formalism::ConditionView MoveExistentialQuantifiersTranslator<Derived>::move_existentials_node(formalism::ConditionView condition, formalism::EntityView<T>)
{ return condition; }

template<typename Derived>
formalism::ConditionView MoveExistentialQuantifiersTranslator<Derived>::lift_top_level_exists(urd::Store<urd::sequence<formalism::ParameterView>>& parameters,
                                                                                              formalism::ConditionView condition,
                                                                                              std::optional<formalism::EffectView> effect)
{
    auto moved = this->self().move_existentials(condition);
    if (const auto exists = this->self().as_exists(moved))
    {
        auto claimed = urd::Store<urd::set<std::string>> {};
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
