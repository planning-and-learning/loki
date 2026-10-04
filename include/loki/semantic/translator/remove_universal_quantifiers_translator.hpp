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

#ifndef LOKI_SEMANTIC_TRANSLATOR_REMOVE_UNIVERSAL_QUANTIFIERS_TRANSLATOR_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_REMOVE_UNIVERSAL_QUANTIFIERS_TRANSLATOR_HPP_

#include "loki/semantic/translator/copy_translator_component.hpp"

namespace loki::semantic::detail
{

template<typename Derived>
class RemoveUniversalQuantifiersTranslator : public CopyTranslatorComponent<Derived, RemoveUniversalQuantifiersTranslator<Derived>>
{
public:
    explicit RemoveUniversalQuantifiersTranslator(CopyContext& context) :
        CopyTranslatorComponent<Derived, RemoveUniversalQuantifiersTranslator<Derived>>(context)
    {
    }

    formalism::ConditionView make_generated_axiom_condition(formalism::ConditionView condition);
    formalism::ConditionView remove_universal_quantifiers(formalism::ConditionView condition);
    formalism::ConditionView remove_universal_quantifiers_node(formalism::ConditionAndView source);
    formalism::ConditionView remove_universal_quantifiers_node(formalism::ConditionOrView source);
    formalism::ConditionView remove_universal_quantifiers_node(formalism::ConditionNotView source);
    formalism::ConditionView remove_universal_quantifiers_node(formalism::ConditionImplyView source);
    formalism::ConditionView remove_universal_quantifiers_node(formalism::ConditionExistsView source);
    formalism::ConditionView remove_universal_quantifiers_node(formalism::ConditionForallView source);
    template<typename T>
    formalism::ConditionView remove_universal_quantifiers_node(formalism::EntityView<T> source);
};

template<typename Derived>
formalism::ConditionView RemoveUniversalQuantifiersTranslator<Derived>::make_generated_axiom_condition(formalism::ConditionView condition)
{
    const auto free_parameters = this->self().free_parameters_in_scope(condition);

    const auto key = GeneratedUniversalConditionKey { free_parameters, condition };
    if (auto it = this->m_generated_universal_conditions.find(key); it != this->m_generated_universal_conditions.end())
        return it->second;

    auto predicate_data = formalism::checkout<formalism::Predicate>(this->m_context.builder);
    auto atom_data = formalism::checkout<formalism::Atom>(this->m_context.builder);
    for (auto [parameter, variable] : free_parameters)
    {
        predicate_data->parameters.push_back(parameter.get_index());
        atom_data->terms.push_back(as_index(this->self().term_from_variable(variable)));
    }

    predicate_data->name = cista::offset::string(this->self().next_generated_predicate_name("loki-universal-"));
    const auto predicate = formalism::insert(this->m_storage->repository, *predicate_data).first;
    atom_data->predicate = predicate.get_index();
    const auto atom = formalism::insert(this->m_storage->repository, *atom_data).first.get_index();
    auto literal_data = formalism::checkout<formalism::Literal>(this->m_context.builder);
    literal_data->atom = atom;
    literal_data->m_polarity = true;
    const auto positive_head = formalism::insert(this->m_storage->repository, *literal_data).first.get_index();
    literal_data->clear();
    literal_data->atom = atom;
    literal_data->m_polarity = false;
    const auto negative_literal = formalism::insert(this->m_storage->repository, *literal_data).first.get_index();
    auto axiom_data = formalism::checkout<formalism::Axiom>(this->m_context.builder);
    for (auto parameter : predicate_data->parameters)
        axiom_data->parameters.push_back(parameter);
    axiom_data->original_arity = axiom_data->parameters.size();
    axiom_data->head = positive_head;
    axiom_data->condition = condition.get_index();
    const auto axiom = formalism::insert(this->m_storage->repository, *axiom_data).first;

    this->m_generated_predicates.push_back(predicate);
    this->m_generated_axioms.push_back(axiom);
    auto condition_data = formalism::checkout<formalism::ConditionLiteral>(this->m_context.builder);
    condition_data->literal = negative_literal;
    auto result = this->self().wrap_condition(formalism::insert(this->m_storage->repository, *condition_data).first);
    this->m_generated_universal_conditions.emplace(key, result);
    return result;
}

template<typename Derived>
formalism::ConditionView RemoveUniversalQuantifiersTranslator<Derived>::remove_universal_quantifiers(formalism::ConditionView condition)
{
    return ygg::visit([&](const auto& node) { return this->self().remove_universal_quantifiers_node(node); }, condition.get_variant());
}

template<typename Derived>
formalism::ConditionView RemoveUniversalQuantifiersTranslator<Derived>::remove_universal_quantifiers_node(formalism::ConditionAndView source)
{
    auto data = formalism::checkout<formalism::ConditionAnd>(this->m_context.builder);
    for (auto condition : source.get_conditions())
        this->self().append_conjunct(*data, this->self().remove_universal_quantifiers(condition));
    return this->self().make_conjunction(*data);
}

template<typename Derived>
formalism::ConditionView RemoveUniversalQuantifiersTranslator<Derived>::remove_universal_quantifiers_node(formalism::ConditionOrView source)
{
    auto data = formalism::checkout<formalism::ConditionOr>(this->m_context.builder);
    for (auto condition : source.get_conditions())
        this->self().append_disjunct(*data, this->self().remove_universal_quantifiers(condition));
    return this->self().make_disjunction(*data);
}

template<typename Derived>
formalism::ConditionView RemoveUniversalQuantifiersTranslator<Derived>::remove_universal_quantifiers_node(formalism::ConditionNotView source)
{
    const auto condition = as_index(this->self().remove_universal_quantifiers(source.get_condition()));
    auto data = formalism::checkout<formalism::ConditionNot>(this->m_context.builder);
    data->condition = condition;
    return this->self().wrap_condition(formalism::insert(this->m_storage->repository, *data).first);
}

template<typename Derived>
formalism::ConditionView RemoveUniversalQuantifiersTranslator<Derived>::remove_universal_quantifiers_node(formalism::ConditionImplyView source)
{
    // Sequence the recursions: both sides can generate loki-universal-* names, and argument
    // evaluation order is unspecified.
    const auto left = as_index(this->self().remove_universal_quantifiers(source.get_left()));
    const auto right = as_index(this->self().remove_universal_quantifiers(source.get_right()));
    auto data = formalism::checkout<formalism::ConditionImply>(this->m_context.builder);
    data->left = left;
    data->right = right;
    return this->self().wrap_condition(formalism::insert(this->m_storage->repository, *data).first);
}

template<typename Derived>
formalism::ConditionView RemoveUniversalQuantifiersTranslator<Derived>::remove_universal_quantifiers_node(formalism::ConditionExistsView source)
{
    this->self().increment_quantifications(source.get_parameters());
    auto parameter_views = this->self().copy_parameter_views(source.get_parameters());
    this->self().enter_scope(parameter_views);
    auto condition = as_index(this->self().remove_universal_quantifiers(source.get_condition()));
    this->self().leave_scope();
    auto data = formalism::checkout<formalism::ConditionExists>(this->m_context.builder);
    for (auto parameter : parameter_views)
        data->parameters.push_back(parameter.get_index());
    data->condition = condition;
    return this->self().flatten_condition(this->self().wrap_condition(formalism::insert(this->m_storage->repository, *data).first));
}

template<typename Derived>
formalism::ConditionView RemoveUniversalQuantifiersTranslator<Derived>::remove_universal_quantifiers_node(formalism::ConditionForallView source)
{
    this->self().increment_quantifications(source.get_parameters());
    auto parameter_views = this->self().copy_parameter_views(source.get_parameters());
    this->self().enter_scope(parameter_views);
    auto negated = as_index(this->self().negate_condition(source.get_condition()));
    this->self().leave_scope();
    auto data = formalism::checkout<formalism::ConditionExists>(this->m_context.builder);
    for (auto parameter : parameter_views)
        data->parameters.push_back(parameter.get_index());
    data->condition = negated;
    const auto exists_not = this->self().flatten_condition(this->self().wrap_condition(formalism::insert(this->m_storage->repository, *data).first));
    const auto translated_exists_not = this->self().remove_universal_quantifiers(exists_not);
    return this->self().make_generated_axiom_condition(translated_exists_not);
}

template<typename Derived>
template<typename T>
formalism::ConditionView RemoveUniversalQuantifiersTranslator<Derived>::remove_universal_quantifiers_node(formalism::EntityView<T> source)
{
    return this->self().wrap_condition(this->self().copy(source));
}

}  // namespace loki::semantic::detail

#endif
