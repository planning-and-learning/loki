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

#ifndef LOKI_SEMANTIC_TRANSLATOR_RENAME_QUANTIFIED_VARIABLES_TRANSLATOR_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_RENAME_QUANTIFIED_VARIABLES_TRANSLATOR_HPP_

#include "copy_translator_component.hpp"

#include <variant>

namespace loki::semantic::detail
{

template<typename Derived>
class RenameQuantifiedVariablesTranslator : public CopyTranslatorComponent<Derived, RenameQuantifiedVariablesTranslator<Derived>>
{
public:
    explicit RenameQuantifiedVariablesTranslator(CopyContext& context) : CopyTranslatorComponent<Derived, RenameQuantifiedVariablesTranslator<Derived>>(context)
    {
    }

    void enter_variable_scope();
    void leave_variable_scope();
    /// Binds source to target in the innermost scope, unless that scope bound source already.
    void bind_variable(formalism::VariableView source, formalism::VariableView target);

    /// Every entity below an action, an axiom or a goal: its children renamed in a scope of its own, so that the
    /// parameters of a binder (its first children) bind their variables for its other children only.
    template<typename T>
    formalism::EntityView<T> rename_variables(formalism::EntityView<T> source);
    template<typename... Vs>
    std::variant<Vs...> rename_variables(const std::variant<Vs...>& source);
    /// A bound variable is renamed to its binding's target, a free one copied.
    formalism::VariableView rename_variables(formalism::VariableView source);
    /// A parameter binds its variable to a fresh one, in the scope of the binder it belongs to.
    formalism::ParameterView rename_variables(formalism::ParameterView source);
    /// Declarations and constants hold no quantified variables: copied.
    formalism::PredicateView rename_variables(formalism::PredicateView source) { return this->self().copy(source); }
    formalism::FunctionSkeletonView rename_variables(formalism::FunctionSkeletonView source) { return this->self().copy(source); }
    formalism::TypeView rename_variables(formalism::TypeView source) { return this->self().copy(source); }
    formalism::ObjectView rename_variables(formalism::ObjectView source) { return this->self().copy(source); }
    formalism::EntityView<formalism::FunctionExpression<formalism::NumberTag>>
    rename_variables(formalism::EntityView<formalism::FunctionExpression<formalism::NumberTag>> source)
    { return this->self().copy(source); }

    formalism::ActionView rename_action_variables(formalism::ActionView source);
    formalism::AxiomView rename_axiom_variables(formalism::AxiomView source);

private:
    formalism::VariableView fresh_variable(formalism::VariableView source);
};

template<typename Derived>
void RenameQuantifiedVariablesTranslator<Derived>::enter_variable_scope()
{
    this->copy_context().variable_bindings.push_scope();
    this->copy_context().shadowed_variable_names.push_scope();
}

template<typename Derived>
void RenameQuantifiedVariablesTranslator<Derived>::leave_variable_scope()
{
    this->copy_context().variable_bindings.pop_scope();
    this->copy_context().shadowed_variable_names.pop_scope();
}

template<typename Derived>
void RenameQuantifiedVariablesTranslator<Derived>::bind_variable(formalism::VariableView source, formalism::VariableView target)
{
    auto& bindings = this->copy_context().variable_bindings;
    if (bindings.bound_in_scope(source))
        return;
    if (const auto outer = bindings.find(source); outer != bindings.end())
        this->copy_context().shadowed_variable_names.insert_or_assign(std::string(outer->second.get_name()), outer->second);
    bindings.insert_or_assign(source, target);
}

template<typename Derived>
formalism::VariableView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::VariableView source)
{
    const auto& bindings = this->copy_context().variable_bindings;
    if (const auto mapped = bindings.find(source); mapped != bindings.end())
        return mapped->second;
    return this->self().copy(source);
}

template<typename Derived>
formalism::VariableView RenameQuantifiedVariablesTranslator<Derived>::fresh_variable(formalism::VariableView source)
{
    // Pure capture avoidance: keep the name unless this binding shadows an enclosing one.
    // Sibling scopes may share names; the existential-lifting phase uniquifies where scopes
    // actually merge, which keeps renaming idempotent across repeated translations.
    if (!this->copy_context().variable_bindings.contains(source))
        return this->self().copy(source);

    // Pick the first suffix that neither an enclosing binding nor an earlier rename in this
    // action uses; earlier renames may later be lifted into the action parameters, so reusing
    // one would shadow it on the next translation and break idempotence.
    auto taken = this->copy_context().renamed_variable_names;
    for (const auto& [_, target] : this->copy_context().variable_bindings)
        taken.insert(std::string(target.get_name()));
    for (const auto& [name, _] : this->copy_context().shadowed_variable_names)
        taken.insert(name);
    for (auto k = std::size_t { 0 };; ++k)
    {
        auto name = std::string(source.get_name()) + "_" + std::to_string(k);
        if (!taken.contains(name))
        {
            this->copy_context().renamed_variable_names.insert(name);
            auto data = formalism::checkout<formalism::Variable>(this->copy_context().builder);
            data->name = urd::flat_string(name);
            return formalism::insert(this->copy_context().storage->repository, *data).first;
        }
    }
}

template<typename Derived>
formalism::ParameterView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::ParameterView source)
{
    const auto variable = this->self().fresh_variable(source.get_variable());
    this->self().bind_variable(source.get_variable(), variable);
    auto data = formalism::checkout<formalism::Parameter>(this->copy_context().builder);
    data->variable = variable.index();
    this->self().template copy_list<formalism::Type>(source.get_types(), data->types);
    return formalism::insert(this->copy_context().storage->repository, *data).first;
}

template<typename Derived>
template<typename T>
formalism::EntityView<T> RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::EntityView<T> source)
{
    this->self().enter_variable_scope();
    const auto target = rebuild(this->copy_context().storage->repository, source, [&](auto child) { return this->self().rename_variables(child); });
    this->self().leave_variable_scope();
    return target;
}

template<typename Derived>
template<typename... Vs>
std::variant<Vs...> RenameQuantifiedVariablesTranslator<Derived>::rename_variables(const std::variant<Vs...>& source)
{ return std::visit([&](auto alternative) -> std::variant<Vs...> { return this->self().rename_variables(alternative); }, source); }

template<typename Derived>
formalism::ActionView RenameQuantifiedVariablesTranslator<Derived>::rename_action_variables(formalism::ActionView source)
{
    this->copy_context().num_quantifications.clear();
    this->copy_context().renamed_variable_names.clear();
    return this->self().rename_variables(source);
}

template<typename Derived>
formalism::AxiomView RenameQuantifiedVariablesTranslator<Derived>::rename_axiom_variables(formalism::AxiomView source)
{
    this->copy_context().num_quantifications.clear();
    this->copy_context().renamed_variable_names.clear();
    return this->self().rename_variables(source);
}

}  // namespace loki::semantic::detail

#endif
