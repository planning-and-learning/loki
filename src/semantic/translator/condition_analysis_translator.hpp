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

#ifndef LOKI_SEMANTIC_TRANSLATOR_CONDITION_ANALYSIS_TRANSLATOR_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_CONDITION_ANALYSIS_TRANSLATOR_HPP_

#include "copy_translator_component.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

#include <algorithm>
#include <concepts>
#include <urd/elements/children.hpp>
#include <variant>
#include <urd/elements/less.hpp>

namespace loki::semantic::detail
{

using VariableSet = urd::Store<urd::set<formalism::VariableView>>;

template<typename Derived>
class ConditionAnalysisTranslator : public CopyTranslatorComponent<Derived, ConditionAnalysisTranslator<Derived>>
{
public:
    explicit ConditionAnalysisTranslator(CopyContext& context) : CopyTranslatorComponent<Derived, ConditionAnalysisTranslator<Derived>>(context) {}

    /// Adds the variables of node not in bound to free: those of its children.
    template<typename T>
    void collect_free_variables(formalism::EntityView<T> node, VariableSet& bound, VariableSet& free) const;
    template<typename... Vs>
    void collect_free_variables(const std::variant<Vs...>& node, VariableSet& bound, VariableSet& free) const;
    void collect_free_variables(formalism::VariableView variable, VariableSet& bound, VariableSet& free) const;
    /// A quantifier binds its parameters' variables in its condition.
    template<typename Tag>
        requires std::same_as<Tag, formalism::ExistsTag> || std::same_as<Tag, formalism::ForallTag>
    void collect_free_variables(formalism::EntityView<formalism::Condition<Tag>> node, VariableSet& bound, VariableSet& free) const;
    /// Declarations and constants hold none of the node's variables.
    void collect_free_variables(formalism::PredicateView, VariableSet&, VariableSet&) const {}
    void collect_free_variables(formalism::FunctionSkeletonView, VariableSet&, VariableSet&) const {}
    void collect_free_variables(formalism::ObjectView, VariableSet&, VariableSet&) const {}

    [[nodiscard]] urd::Store<urd::sequence<std::pair<formalism::ParameterView, formalism::VariableView>>> free_parameters_in_scope(formalism::ConditionView condition) const;
};

template<typename Derived>
template<typename T>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::EntityView<T> node, VariableSet& bound, VariableSet& free) const
{
    urd::for_each_child<formalism::SymbolRepositoryTypes, T>(node.data(),
                                                             node.context(),
                                                             [&](auto child) { this->self().collect_free_variables(child, bound, free); });
}

template<typename Derived>
template<typename... Vs>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(const std::variant<Vs...>& node, VariableSet& bound, VariableSet& free) const
{ std::visit([&](auto alternative) { this->self().collect_free_variables(alternative, bound, free); }, node); }

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::VariableView variable, VariableSet& bound, VariableSet& free) const
{
    if (!bound.contains(variable))
        free.insert(variable);
}

template<typename Derived>
template<typename Tag>
    requires std::same_as<Tag, formalism::ExistsTag> || std::same_as<Tag, formalism::ForallTag>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::EntityView<formalism::Condition<Tag>> node, VariableSet& bound, VariableSet& free) const
{
    for (auto parameter : node.get_parameters())
        bound.insert(parameter.get_variable());
    this->self().collect_free_variables(node.get_condition(), bound, free);
    for (auto parameter : node.get_parameters())
        bound.erase(parameter.get_variable());
}

template<typename Derived>
urd::Store<urd::sequence<std::pair<formalism::ParameterView, formalism::VariableView>>>
ConditionAnalysisTranslator<Derived>::free_parameters_in_scope(formalism::ConditionView condition) const
{
    auto bound = urd::Store<urd::set<formalism::VariableView>> {};
    auto free = urd::Store<urd::set<formalism::VariableView>> {};
    this->self().collect_free_variables(condition, bound, free);

    auto result = urd::Store<urd::sequence<std::pair<formalism::ParameterView, formalism::VariableView>>> {};
    for (const auto& scope : this->copy_context().active_parameters)
    {
        for (const auto& parameter : scope)
        {
            if (free.contains(parameter.second))
                result.push_back(parameter);
        }
    }
    std::sort(result.begin(), result.end());
    return result;
}

}  // namespace loki::semantic::detail

#endif
