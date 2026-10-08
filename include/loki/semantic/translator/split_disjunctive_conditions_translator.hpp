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

#ifndef LOKI_SEMANTIC_TRANSLATOR_SPLIT_DISJUNCTIVE_CONDITIONS_TRANSLATOR_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_SPLIT_DISJUNCTIVE_CONDITIONS_TRANSLATOR_HPP_

#include "loki/semantic/translator/copy_translator_component.hpp"

#include <yggdrasil/containers/unordered_set.hpp>

namespace loki::semantic::detail
{

template<typename Derived>
class SplitDisjunctiveConditionsTranslator : public CopyTranslatorComponent<Derived, SplitDisjunctiveConditionsTranslator<Derived>>
{
public:
    explicit SplitDisjunctiveConditionsTranslator(CopyContext& context) :
        CopyTranslatorComponent<Derived, SplitDisjunctiveConditionsTranslator<Derived>>(context)
    {
    }

    void split_disjunctive_actions(formalism::EntityListView<formalism::Action> actions, ygg::IndexList<formalism::Action>& result);
    void split_disjunctive_axioms(formalism::EntityListView<formalism::Axiom> axioms, ygg::IndexList<formalism::Axiom>& result);
};

template<typename Derived>
void SplitDisjunctiveConditionsTranslator<Derived>::split_disjunctive_actions(formalism::EntityListView<formalism::Action> actions,
                                                                              ygg::IndexList<formalism::Action>& result)
{
    result.clear();
    auto seen = ygg::UnorderedSet<formalism::ActionView> {};
    for (auto action : actions)
    {
        const auto& data = action.get_data();
        if (const auto precondition_view = action.get_precondition())
        {
            const auto precondition = this->self().flatten_condition(precondition_view.value());
            if (const auto condition_or = this->self().as_or(precondition))
            {
                for (auto part : condition_or->get_conditions())
                {
                    auto action_data = formalism::checkout<formalism::Action>(this->m_context.builder);
                    action_data->name = data.name;
                    action_data->original_name = data.original_name;
                    for (auto parameter : data.parameters)
                        action_data->parameters.push_back(parameter);
                    action_data->original_arity = data.original_arity;
                    action_data->precondition = part.get_index();
                    action_data->effect = data.effect;
                    this->self().push_unique(result, seen, formalism::insert(this->m_context.storage->repository, *action_data).first);
                }
                continue;
            }
        }
        this->self().push_unique(result, seen, action);
    }
}

template<typename Derived>
void SplitDisjunctiveConditionsTranslator<Derived>::split_disjunctive_axioms(formalism::EntityListView<formalism::Axiom> axioms,
                                                                             ygg::IndexList<formalism::Axiom>& result)
{
    result.clear();
    auto seen = ygg::UnorderedSet<formalism::AxiomView> {};
    for (auto axiom : axioms)
    {
        const auto& data = axiom.get_data();
        const auto condition = this->self().flatten_condition(axiom.get_condition());
        if (const auto condition_or = this->self().as_or(condition))
        {
            for (auto part : condition_or->get_conditions())
            {
                auto axiom_data = formalism::checkout<formalism::Axiom>(this->m_context.builder);
                for (auto parameter : data.parameters)
                    axiom_data->parameters.push_back(parameter);
                axiom_data->original_arity = data.original_arity;
                axiom_data->head = data.head;
                axiom_data->condition = part.get_index();
                this->self().push_unique(result, seen, formalism::insert(this->m_context.storage->repository, *axiom_data).first);
            }
        }
        else
        {
            this->self().push_unique(result, seen, axiom);
        }
    }
}

}  // namespace loki::semantic::detail

#endif
