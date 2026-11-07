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
    formalism::ConditionView remove_universal_quantifiers_node(formalism::EntityView<formalism::Condition<formalism::AndTag>> source);
    formalism::ConditionView remove_universal_quantifiers_node(formalism::EntityView<formalism::Condition<formalism::OrTag>> source);
    formalism::ConditionView remove_universal_quantifiers_node(formalism::EntityView<formalism::Condition<formalism::NotTag>> source);
    formalism::ConditionView remove_universal_quantifiers_node(formalism::EntityView<formalism::Condition<formalism::ImplyTag>> source);
    formalism::ConditionView remove_universal_quantifiers_node(formalism::EntityView<formalism::Condition<formalism::ExistsTag>> source);
    formalism::ConditionView remove_universal_quantifiers_node(formalism::EntityView<formalism::Condition<formalism::ForallTag>> source);
    template<typename T>
    formalism::ConditionView remove_universal_quantifiers_node(formalism::EntityView<T> source);
};

template<typename Derived>
formalism::ConditionView RemoveUniversalQuantifiersTranslator<Derived>::make_generated_axiom_condition(formalism::ConditionView condition)
{
    const auto free_parameters = this->self().free_parameters_in_scope(condition);

    const auto key = GeneratedUniversalConditionKey { free_parameters, condition };
    if (auto it = this->copy_context().generated_universal_conditions.find(key); it != this->copy_context().generated_universal_conditions.end())
        return it->second;

    auto predicate_data = formalism::checkout<formalism::Predicate>(this->copy_context().builder);
    auto atom_data = formalism::checkout<formalism::Atom>(this->copy_context().builder);
    for (auto [parameter, variable] : free_parameters)
    {
        predicate_data->parameters.push_back(parameter.index());
        atom_data->terms.push_back(variable.index());
    }

    predicate_data->name = cista::offset::string(this->self().next_generated_predicate_name("loki-universal-"));
    const auto predicate = formalism::insert(this->copy_context().storage->repository, *predicate_data).first;
    atom_data->predicate = predicate.index();
    const auto atom = formalism::insert(this->copy_context().storage->repository, *atom_data).first.index();
    auto literal_data = formalism::checkout<formalism::Literal>(this->copy_context().builder);
    literal_data->atom = atom;
    literal_data->m_polarity = true;
    const auto positive_head = formalism::insert(this->copy_context().storage->repository, *literal_data).first.index();
    urd::clear_members(*literal_data);
    literal_data->atom = atom;
    literal_data->m_polarity = false;
    const auto negative_literal = formalism::insert(this->copy_context().storage->repository, *literal_data).first.index();
    auto axiom_data = formalism::checkout<formalism::Axiom>(this->copy_context().builder);
    for (auto parameter : predicate_data->parameters)
        axiom_data->parameters.push_back(parameter);
    axiom_data->original_arity = axiom_data->parameters.size();
    axiom_data->head = positive_head;
    axiom_data->condition = urd::to_stored(condition);
    const auto axiom = formalism::insert(this->copy_context().storage->repository, *axiom_data).first;

    this->copy_context().generated_predicates.push_back(predicate);
    this->copy_context().generated_axioms.push_back(axiom);
    auto condition_data = formalism::checkout<formalism::Condition<formalism::LiteralTag>>(this->copy_context().builder);
    condition_data->literal = negative_literal;
    auto result = formalism::ConditionView(formalism::insert(this->copy_context().storage->repository, *condition_data).first);
    this->copy_context().generated_universal_conditions.emplace(key, result);
    return result;
}

template<typename Derived>
formalism::ConditionView RemoveUniversalQuantifiersTranslator<Derived>::remove_universal_quantifiers(formalism::ConditionView condition)
{
    return std::visit([&](const auto& node) { return this->self().remove_universal_quantifiers_node(node); }, condition);
}

template<typename Derived>
formalism::ConditionView
RemoveUniversalQuantifiersTranslator<Derived>::remove_universal_quantifiers_node(formalism::EntityView<formalism::Condition<formalism::AndTag>> source)
{
    auto data = formalism::checkout<formalism::Condition<formalism::AndTag>>(this->copy_context().builder);
    for (auto condition : source.get_conditions())
        this->self().append_conjunct(*data, this->self().remove_universal_quantifiers(condition));
    return this->self().make_conjunction(*data);
}

template<typename Derived>
formalism::ConditionView
RemoveUniversalQuantifiersTranslator<Derived>::remove_universal_quantifiers_node(formalism::EntityView<formalism::Condition<formalism::OrTag>> source)
{
    auto data = formalism::checkout<formalism::Condition<formalism::OrTag>>(this->copy_context().builder);
    for (auto condition : source.get_conditions())
        this->self().append_disjunct(*data, this->self().remove_universal_quantifiers(condition));
    return this->self().make_disjunction(*data);
}

template<typename Derived>
formalism::ConditionView
RemoveUniversalQuantifiersTranslator<Derived>::remove_universal_quantifiers_node(formalism::EntityView<formalism::Condition<formalism::NotTag>> source)
{
    const auto condition = urd::to_stored(this->self().remove_universal_quantifiers(source.get_condition()));
    auto data = formalism::checkout<formalism::Condition<formalism::NotTag>>(this->copy_context().builder);
    data->condition = condition;
    return formalism::ConditionView(formalism::insert(this->copy_context().storage->repository, *data).first);
}

template<typename Derived>
formalism::ConditionView
RemoveUniversalQuantifiersTranslator<Derived>::remove_universal_quantifiers_node(formalism::EntityView<formalism::Condition<formalism::ImplyTag>> source)
{
    // Sequence the recursions: both sides can generate loki-universal-* names, and argument
    // evaluation order is unspecified.
    const auto left = urd::to_stored(this->self().remove_universal_quantifiers(source.get_left()));
    const auto right = urd::to_stored(this->self().remove_universal_quantifiers(source.get_right()));
    auto data = formalism::checkout<formalism::Condition<formalism::ImplyTag>>(this->copy_context().builder);
    data->left = left;
    data->right = right;
    return formalism::ConditionView(formalism::insert(this->copy_context().storage->repository, *data).first);
}

template<typename Derived>
formalism::ConditionView
RemoveUniversalQuantifiersTranslator<Derived>::remove_universal_quantifiers_node(formalism::EntityView<formalism::Condition<formalism::ExistsTag>> source)
{
    this->self().increment_quantifications(source.get_parameters());
    auto parameter_views = this->self().copy_parameter_views(source.get_parameters());
    this->self().enter_scope(parameter_views);
    auto condition = urd::to_stored(this->self().remove_universal_quantifiers(source.get_condition()));
    this->self().leave_scope();
    auto data = formalism::checkout<formalism::Condition<formalism::ExistsTag>>(this->copy_context().builder);
    for (auto parameter : parameter_views)
        data->parameters.push_back(parameter.index());
    data->condition = condition;
    return this->self().flatten_condition(formalism::ConditionView(formalism::insert(this->copy_context().storage->repository, *data).first));
}

template<typename Derived>
formalism::ConditionView
RemoveUniversalQuantifiersTranslator<Derived>::remove_universal_quantifiers_node(formalism::EntityView<formalism::Condition<formalism::ForallTag>> source)
{
    this->self().increment_quantifications(source.get_parameters());
    auto parameter_views = this->self().copy_parameter_views(source.get_parameters());
    this->self().enter_scope(parameter_views);
    auto negated = urd::to_stored(this->self().negate_condition(source.get_condition()));
    this->self().leave_scope();
    auto data = formalism::checkout<formalism::Condition<formalism::ExistsTag>>(this->copy_context().builder);
    for (auto parameter : parameter_views)
        data->parameters.push_back(parameter.index());
    data->condition = negated;
    const auto exists_not =
        this->self().flatten_condition(formalism::ConditionView(formalism::insert(this->copy_context().storage->repository, *data).first));
    const auto translated_exists_not = this->self().remove_universal_quantifiers(exists_not);
    return this->self().make_generated_axiom_condition(translated_exists_not);
}

template<typename Derived>
template<typename T>
formalism::ConditionView RemoveUniversalQuantifiersTranslator<Derived>::remove_universal_quantifiers_node(formalism::EntityView<T> source)
{ return formalism::ConditionView(this->self().copy(source)); }

}  // namespace loki::semantic::detail

#endif
