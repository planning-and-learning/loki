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

#ifndef LOKI_SEMANTIC_TRANSLATOR_SIMPLIFY_GOAL_TRANSLATOR_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_SIMPLIFY_GOAL_TRANSLATOR_HPP_

#include "copy_translator_component.hpp"

namespace loki::semantic::detail
{

template<typename Derived>
class SimplifyGoalTranslator : public CopyTranslatorComponent<Derived, SimplifyGoalTranslator<Derived>>
{
public:
    explicit SimplifyGoalTranslator(CopyContext& context) : CopyTranslatorComponent<Derived, SimplifyGoalTranslator<Derived>>(context) {}

    formalism::ConditionView make_generated_goal_condition(formalism::ConditionVariant condition);
    formalism::ConditionView simplify_goal_condition(formalism::ConditionView condition);
    formalism::ConditionView simplify_goal_condition_node(formalism::ConditionView condition,
                                                          formalism::EntityView<formalism::Condition<formalism::LiteralTag>>);
    formalism::ConditionView simplify_goal_condition_node(formalism::ConditionView condition,
                                                          formalism::EntityView<formalism::Condition<formalism::NumericConstraintTag>>);
    formalism::ConditionView simplify_goal_condition_node(formalism::ConditionView, formalism::EntityView<formalism::Condition<formalism::AndTag>> node);
    template<typename T>
    formalism::ConditionView simplify_goal_condition_node(formalism::ConditionView condition, T);
};

template<typename Derived>
formalism::ConditionView SimplifyGoalTranslator<Derived>::make_generated_goal_condition(formalism::ConditionVariant condition)
{
    const auto name = urd::flat_string(this->self().next_generated_predicate_name("loki-goal-"));
    auto predicate_data = formalism::checkout<formalism::Predicate>(this->copy_context().builder);
    predicate_data->name = name;
    const auto predicate = this->copy_context().storage->repository.insert(*predicate_data, this->copy_context().storage->builder).first;
    auto atom_data = formalism::checkout<formalism::Atom>(this->copy_context().builder);
    atom_data->predicate = predicate.index();
    const auto atom = this->copy_context().storage->repository.insert(*atom_data, this->copy_context().storage->builder).first.index();
    auto literal_data = formalism::checkout<formalism::Literal>(this->copy_context().builder);
    literal_data->atom = atom;
    literal_data->polarity = true;
    const auto literal = this->copy_context().storage->repository.insert(*literal_data, this->copy_context().storage->builder).first.index();
    auto axiom_data = formalism::checkout<formalism::Axiom>(this->copy_context().builder);
    axiom_data->original_arity = 0;
    axiom_data->head = literal;
    urd::store(axiom_data->condition, condition);
    const auto axiom = this->copy_context().storage->repository.insert(*axiom_data, this->copy_context().storage->builder).first;
    urd::append(this->copy_context().generated_predicates, predicate);
    urd::append(this->copy_context().generated_axioms, axiom);
    auto condition_data = formalism::checkout<formalism::Condition<formalism::LiteralTag>>(this->copy_context().builder);
    condition_data->literal = literal;
    return formalism::ConditionView(this->copy_context().storage->repository.insert(*condition_data, this->copy_context().storage->builder).first);
}

template<typename Derived>
formalism::ConditionView SimplifyGoalTranslator<Derived>::simplify_goal_condition(formalism::ConditionView condition)
{
    return std::visit([&](const auto& node) { return this->self().simplify_goal_condition_node(condition, node); }, condition);
}

template<typename Derived>
formalism::ConditionView SimplifyGoalTranslator<Derived>::simplify_goal_condition_node(formalism::ConditionView condition,
                                                                                       formalism::EntityView<formalism::Condition<formalism::LiteralTag>>)
{ return condition; }

template<typename Derived>
formalism::ConditionView
SimplifyGoalTranslator<Derived>::simplify_goal_condition_node(formalism::ConditionView condition,
                                                              formalism::EntityView<formalism::Condition<formalism::NumericConstraintTag>>)
{ return condition; }

template<typename Derived>
formalism::ConditionView SimplifyGoalTranslator<Derived>::simplify_goal_condition_node(formalism::ConditionView,
                                                                                       formalism::EntityView<formalism::Condition<formalism::AndTag>> node)
{
    auto data = formalism::checkout<formalism::Condition<formalism::AndTag>>(this->copy_context().builder);
    for (auto child : node.get_conditions())
        this->self().append_conjunct(*data, this->self().simplify_goal_condition(child));
    return this->self().make_conjunction(*data);
}

template<typename Derived>
template<typename T>
formalism::ConditionView SimplifyGoalTranslator<Derived>::simplify_goal_condition_node(formalism::ConditionView condition, T)
{ return this->self().make_generated_goal_condition(urd::as_member<formalism::ConditionVariant>(condition)); }

}  // namespace loki::semantic::detail

#endif
