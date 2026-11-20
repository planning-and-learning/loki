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

#include "copy_translator_component.hpp"

#include <urd/elements/flat.hpp>
#include <urd/repr/maps/unordered_flat_map.hpp>
#include <urd/repr/values/std_set.hpp>
#include <urd/repr/values/unordered_flat_set.hpp>
#include <urd/store.hpp>

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

    void split_disjunctive_actions(formalism::EntityListView<formalism::Action> actions,
                                   urd::built_t<urd::flat_vector<urd::index<formalism::Action>>, formalism::Repository>& result);
    void split_disjunctive_axioms(formalism::EntityListView<formalism::Axiom> axioms,
                                  urd::built_t<urd::flat_vector<urd::index<formalism::Axiom>>, formalism::Repository>& result);
};

template<typename Derived>
void SplitDisjunctiveConditionsTranslator<Derived>::split_disjunctive_actions(
    formalism::EntityListView<formalism::Action> actions,
    urd::built_t<urd::flat_vector<urd::index<formalism::Action>>, formalism::Repository>& result)
{
    result.clear();
    auto seen = urd::Store<urd::set<formalism::ActionView>> {};
    for (auto action : actions)
    {
        if (const auto precondition_view = action.get_precondition())
        {
            const auto precondition = this->self().flatten_condition(precondition_view.value());
            if (const auto condition_or = this->self().as_or(precondition))
            {
                for (auto part : condition_or->get_conditions())
                {
                    auto action_data = formalism::checkout<formalism::Action>(this->copy_context().builder);
                    action_data->name = action.get_name();
                    action_data->original_name = action.get_original_name();
                    for (auto parameter : action.get_parameters())
                        action_data->parameters.push_back(parameter.index());
                    action_data->original_arity = action.get_original_arity();
                    urd::store(action_data->precondition, part);
                    urd::store(action_data->effect, action.get_effect());
                    this->self().push_unique(result,
                                             seen,
                                             this->copy_context().storage->repository.insert(*action_data, this->copy_context().storage->builder).first);
                }
                continue;
            }
        }
        this->self().push_unique(result, seen, action);
    }
}

template<typename Derived>
void SplitDisjunctiveConditionsTranslator<Derived>::split_disjunctive_axioms(
    formalism::EntityListView<formalism::Axiom> axioms,
    urd::built_t<urd::flat_vector<urd::index<formalism::Axiom>>, formalism::Repository>& result)
{
    result.clear();
    auto seen = urd::Store<urd::set<formalism::AxiomView>> {};
    for (auto axiom : axioms)
    {
        const auto condition = this->self().flatten_condition(axiom.get_condition());
        if (const auto condition_or = this->self().as_or(condition))
        {
            for (auto part : condition_or->get_conditions())
            {
                auto axiom_data = formalism::checkout<formalism::Axiom>(this->copy_context().builder);
                for (auto parameter : axiom.get_parameters())
                    axiom_data->parameters.push_back(parameter.index());
                axiom_data->original_arity = axiom.get_original_arity();
                axiom_data->head = axiom.get_head().index();
                urd::store(axiom_data->condition, part);
                this->self().push_unique(result,
                                         seen,
                                         this->copy_context().storage->repository.insert(*axiom_data, this->copy_context().storage->builder).first);
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
