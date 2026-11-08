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

#ifndef LOKI_SEMANTIC_TRANSLATOR_BASIC_COPY_TRANSLATOR_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_BASIC_COPY_TRANSLATOR_HPP_

#include "copy_translator_component.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

namespace loki::semantic::detail
{

template<typename Derived>
class BasicCopyTranslator : public CopyTranslatorComponent<Derived, BasicCopyTranslator<Derived>>
{
public:
    explicit BasicCopyTranslator(CopyContext& context) : CopyTranslatorComponent<Derived, BasicCopyTranslator<Derived>>(context) {}

    void increment_quantifications(formalism::EntityListView<formalism::Parameter> parameters);

    template<typename T>
    void copy_list(formalism::EntityListView<T> source, urd::flat_vector<urd::index<T>>& result);

    urd::Store<urd::sequence<formalism::ParameterView>> copy_parameter_views(formalism::EntityListView<formalism::Parameter> source);

    void enter_scope(const urd::Store<urd::sequence<formalism::ParameterView>>& parameters);

    void leave_scope();

    void append_generated_domain_objects(urd::data<formalism::Domain>& data);

    void set_requirements(const urd::Store<urd::sequence<formalism::RequirementKind>>& kinds, urd::flat_vector<urd::index<formalism::Requirement>>& result);

    /// The entities without type-specific logic: their children copied through copy, a variant slot whole (so that
    /// the phase hooks on ConditionView, EffectView and FunctionExpressionView apply), the record rebuilt.
    template<typename T>
    formalism::EntityView<T> copy(formalism::EntityView<T> source);

    formalism::RequirementView copy(formalism::RequirementView source);

    formalism::TypeView copy(formalism::TypeView source);

    formalism::ObjectView copy(formalism::ObjectView source);

    formalism::VariableView copy(formalism::VariableView source);

    formalism::PredicateView copy(formalism::PredicateView source);

    formalism::FunctionSkeletonView copy(formalism::FunctionSkeletonView source);

    formalism::TermView copy(formalism::TermView source);

    formalism::EntityView<formalism::FunctionExpression<formalism::NumberTag>>
    copy(formalism::EntityView<formalism::FunctionExpression<formalism::NumberTag>> source);

    formalism::FunctionExpressionView copy(formalism::FunctionExpressionView source);
};

template<typename Derived>
void BasicCopyTranslator<Derived>::increment_quantifications(formalism::EntityListView<formalism::Parameter> parameters)
{
    for (auto parameter : parameters)
    {
        const auto variable = parameter.get_variable();
        auto& counts = this->copy_context().num_quantifications;
        if (counts.contains(variable))
            ++counts.at(variable);
        else
            counts.emplace(variable, 0);
    }
}

template<typename Derived>
template<typename T>
void BasicCopyTranslator<Derived>::copy_list(formalism::EntityListView<T> source, urd::flat_vector<urd::index<T>>& result)
{
    for (auto view : source)
        result.push_back(urd::to_stored(this->self().copy(view)));
}

template<typename Derived>
urd::Store<urd::sequence<formalism::ParameterView>> BasicCopyTranslator<Derived>::copy_parameter_views(formalism::EntityListView<formalism::Parameter> source)
{
    auto result = urd::Store<urd::sequence<formalism::ParameterView>> {};
    for (auto parameter : source)
        result.push_back(this->self().copy(parameter));
    return result;
}

template<typename Derived>
void BasicCopyTranslator<Derived>::enter_scope(const urd::Store<urd::sequence<formalism::ParameterView>>& parameters)
{
    auto scope = urd::Store<urd::sequence<ScopedFreeParameter>> {};
    for (auto parameter : parameters)
        scope.emplace_back(parameter, parameter.get_variable());
    this->copy_context().active_parameters.push_back(std::move(scope));
}

template<typename Derived>
void BasicCopyTranslator<Derived>::leave_scope()
{
    this->copy_context().active_parameters.pop_back();
}

template<typename Derived>
void BasicCopyTranslator<Derived>::append_generated_domain_objects(urd::data<formalism::Domain>& data)
{
    if (this->copy_context().generated_predicates.empty() && this->copy_context().generated_axioms.empty())
        return;

    for (auto predicate : this->copy_context().generated_predicates)
        data.predicates.push_back(predicate.index());
    for (auto axiom : this->copy_context().generated_axioms)
        data.axioms.push_back(axiom.index());
}

template<typename Derived>
void BasicCopyTranslator<Derived>::set_requirements(const urd::Store<urd::sequence<formalism::RequirementKind>>& kinds, urd::flat_vector<urd::index<formalism::Requirement>>& result)
{
    result.clear();
    auto data = formalism::checkout<formalism::Requirement>(this->copy_context().builder);
    for (const auto kind : kinds)
    {
        data->kind = kind;
        result.push_back(formalism::insert(this->copy_context().storage->repository, *data).first.index());
    }
}

template<typename Derived>
template<typename T>
formalism::EntityView<T> BasicCopyTranslator<Derived>::copy(formalism::EntityView<T> source)
{
    auto data = urd::map_variant_children<formalism::SymbolRepositoryTypes, T>(source.data(), source.context(), [&](auto child) { return this->self().copy(child); });
    return formalism::insert(this->copy_context().storage->repository, data).first;
}

template<typename Derived>
formalism::RequirementView BasicCopyTranslator<Derived>::copy(formalism::RequirementView source)
{
    // Memoized: the translation maps record these copies.
    return memoized(this->copy_context().storage->maps, source, [&] { return this->template copy<formalism::Requirement>(source); });
}

template<typename Derived>
formalism::TypeView BasicCopyTranslator<Derived>::copy(formalism::TypeView source)
{
    // Memoized: the translation maps record these copies.
    return memoized(this->copy_context().storage->maps, source, [&] { return this->template copy<formalism::Type>(source); });
}

template<typename Derived>
formalism::ObjectView BasicCopyTranslator<Derived>::copy(formalism::ObjectView source)
{
    if (auto mapped = find_mapped(this->copy_context().storage->maps, source))
        return *mapped;
    auto source_types = urd::Store<urd::sequence<formalism::TypeView>> {};
    for (auto type : source.get_types())
        source_types.push_back(type);
    auto data = formalism::checkout<formalism::Object>(this->copy_context().builder);
    data->name = source.get_name();
    if (!this->self().compiles_typing_now())
        this->self().copy_type_hierarchy(source.get_types(), data->types);
    auto out = formalism::insert(this->copy_context().storage->repository, *data).first;
    this->copy_context().storage->object_type_views[out] = std::move(source_types);
    remember(this->copy_context().storage->maps, source, out);
    return out;
}

template<typename Derived>
formalism::VariableView BasicCopyTranslator<Derived>::copy(formalism::VariableView source)
{
    auto name = std::string(source.get_name());
    if (this->copy_context().phase == TranslationPhase::RenameQuantifiedVariables && this->copy_context().renaming_enabled)
    {
        if (auto it = this->copy_context().num_quantifications.find(source); it != this->copy_context().num_quantifications.end())
            name += "_" + std::to_string(it->second);
    }
    auto data = formalism::checkout<formalism::Variable>(this->copy_context().builder);
    data->name = urd::flat_string(name);
    return formalism::insert(this->copy_context().storage->repository, *data).first;
}

template<typename Derived>
formalism::PredicateView BasicCopyTranslator<Derived>::copy(formalism::PredicateView source)
{
    if (auto mapped = find_mapped(this->copy_context().storage->maps, source))
        return *mapped;
    const auto previous = this->copy_context().renaming_enabled;
    this->copy_context().renaming_enabled = false;
    auto data = formalism::checkout<formalism::Predicate>(this->copy_context().builder);
    data->name = source.get_name();
    if (this->self().compiles_typing_now())
        this->self().copy_parameters_without_types(source.get_parameters(), data->parameters);
    else
        for (auto parameter : source.get_parameters())
            data->parameters.push_back(urd::to_stored(this->self().copy(parameter)));
    auto out = formalism::insert(this->copy_context().storage->repository, *data).first;
    this->copy_context().used_predicate_names.insert(std::string(source.get_name()));
    this->copy_context().renaming_enabled = previous;
    remember(this->copy_context().storage->maps, source, out);
    return out;
}

template<typename Derived>
formalism::FunctionSkeletonView BasicCopyTranslator<Derived>::copy(formalism::FunctionSkeletonView source)
{
    if (auto mapped = find_mapped(this->copy_context().storage->maps, source))
        return *mapped;
    const auto previous = this->copy_context().renaming_enabled;
    this->copy_context().renaming_enabled = false;
    auto data = formalism::checkout<formalism::FunctionSkeleton>(this->copy_context().builder);
    data->name = source.get_name();
    if (this->self().compiles_typing_now())
        this->self().copy_parameters_without_types(source.get_parameters(), data->parameters);
    else
        for (auto parameter : source.get_parameters())
            data->parameters.push_back(urd::to_stored(this->self().copy(parameter)));
    data->type = urd::to_stored(this->self().copy(source.get_type()));
    auto out = formalism::insert(this->copy_context().storage->repository, *data).first;
    this->copy_context().renaming_enabled = previous;
    remember(this->copy_context().storage->maps, source, out);
    return out;
}

template<typename Derived>
formalism::TermView BasicCopyTranslator<Derived>::copy(formalism::TermView source)
{
    return std::visit([&](const auto& arg) -> formalism::TermView { return this->self().copy(arg); }, source);
}

template<typename Derived>
formalism::EntityView<formalism::FunctionExpression<formalism::NumberTag>>
BasicCopyTranslator<Derived>::copy(formalism::EntityView<formalism::FunctionExpression<formalism::NumberTag>> source)
{
    // Memoized: the translation maps record these copies.
    return memoized(this->copy_context().storage->maps, source, [&] { return this->template copy<formalism::FunctionExpression<formalism::NumberTag>>(source); });
}

template<typename Derived>
formalism::FunctionExpressionView BasicCopyTranslator<Derived>::copy(formalism::FunctionExpressionView source)
{
    if (this->copy_context().phase == TranslationPhase::NormalizeArithmeticExpressions)
        return this->self().normalize_arithmetic_expression(source);

    return std::visit([&](const auto& arg) -> formalism::FunctionExpressionView { return this->self().copy(arg); }, source);
}

}  // namespace loki::semantic::detail

#endif
