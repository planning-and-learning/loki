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

#ifndef LOKI_SEMANTIC_TRANSLATOR_COPY_TRANSLATOR_FACADE_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_COPY_TRANSLATOR_FACADE_HPP_

#include "loki/semantic/errors.hpp"
#include "loki/semantic/translator/copy_translator_component.hpp"

#include <algorithm>
#include <limits>
#include <yggdrasil/containers/unordered_set.hpp>

namespace loki::semantic::detail
{

template<typename Derived>
class CopyTranslatorFacade : public CopyTranslatorComponent<Derived, CopyTranslatorFacade<Derived>>
{
public:
    explicit CopyTranslatorFacade(CopyContext& context) : CopyTranslatorComponent<Derived, CopyTranslatorFacade<Derived>>(context) {}

    template<typename T>
    void push_unique(ygg::IndexList<T>& list, ygg::UnorderedSet<formalism::EntityView<T>>& seen, formalism::EntityView<T> value);

    ygg::UnorderedSet<std::string> used_predicate_names() const;

    std::string next_generated_predicate_name(std::string_view prefix);

    std::pair<formalism::DomainView, bool> copy(formalism::DomainView domain);

    std::pair<formalism::TaskView, bool> copy(formalism::TaskView task);
};

template<typename Derived>
template<typename T>
void CopyTranslatorFacade<Derived>::push_unique(ygg::IndexList<T>& list, ygg::UnorderedSet<formalism::EntityView<T>>& seen, formalism::EntityView<T> value)
{
    if (seen.insert(value).second)
        list.push_back(value.get_index());
}

template<typename Derived>
ygg::UnorderedSet<std::string> CopyTranslatorFacade<Derived>::used_predicate_names() const
{
    auto names = this->m_context.used_predicate_names;
    if (this->m_context.storage->translated_domain)
    {
        for (auto predicate : this->m_context.storage->translated_domain->get_predicates())
            names.insert(std::string(predicate.get_name()));
    }
    return names;
}

template<typename Derived>
std::string CopyTranslatorFacade<Derived>::next_generated_predicate_name(std::string_view prefix)
{
    auto used = this->self().used_predicate_names();
    while (true)
    {
        auto name = std::string(prefix) + std::to_string(this->m_context.num_generated_axioms++);
        if (used.insert(name).second)
        {
            this->m_context.used_predicate_names.insert(name);
            return name;
        }
    }
}

template<typename Derived>
std::pair<formalism::DomainView, bool> CopyTranslatorFacade<Derived>::copy(formalism::DomainView domain)
{
    if (auto mapped = find_mapped(this->m_context.storage->domains, domain))
        return { *mapped, false };
    auto data = formalism::checkout<formalism::Domain>(this->m_context.builder);
    data->name = domain.get_data().name;
    this->self().template copy_list<formalism::Requirement>(domain.get_requirements(), data->requirements);
    this->self().template copy_list<formalism::Type>(domain.get_types(), data->types);
    this->self().template copy_list<formalism::Object>(domain.get_constants(), data->constants);
    this->self().template copy_list<formalism::Predicate>(domain.get_predicates(), data->predicates);
    this->self().template copy_list<formalism::FunctionSkeleton>(domain.get_functions(), data->functions);
    this->self().template copy_list<formalism::Action>(domain.get_actions(), data->actions);
    this->self().template copy_list<formalism::Axiom>(domain.get_axioms(), data->axioms);

    auto [copied_domain, copied_inserted] = formalism::insert(this->m_context.storage->repository, *data);
    this->m_context.storage->translated_domain = copied_domain;
    data->index = {};

    switch (this->m_context.phase)
    {
        case TranslationPhase::RemoveUniversalQuantifiers:
            this->self().append_generated_domain_objects(*data, copied_domain.get_requirements());
            if (!data->axioms.empty())
                this->self().ensure_derived_predicates_requirement(copied_domain.get_requirements(), data->requirements);
            break;
        case TranslationPhase::SplitDisjunctiveConditions:
            this->self().split_disjunctive_actions(copied_domain.get_actions(), data->actions);
            this->self().split_disjunctive_axioms(copied_domain.get_axioms(), data->axioms);
            break;
        case TranslationPhase::CompileConditionalEffects:
            this->self().compile_conditional_effect_actions(copied_domain.get_actions(), data->actions);
            this->self().strip_requirement(copied_domain.get_requirements(), formalism::RequirementKind::ConditionalEffects, data->requirements);
            break;
        case TranslationPhase::CompileTyping:
            this->self().compile_typing_to_domain(*data, copied_domain);
            break;
        case TranslationPhase::MaterializeEquality:
            this->self().add_equality_predicate_to_domain(*data, copied_domain);
            break;
        default:
            break;
    }

    auto [view, inserted] = formalism::insert(this->m_context.storage->repository, *data);
    this->m_context.storage->translated_domain = view;
    remember(this->m_context.storage->domains, domain, view);
    return { view, inserted || (copied_inserted && view == copied_domain) };
}

template<typename Derived>
std::pair<formalism::TaskView, bool> CopyTranslatorFacade<Derived>::copy(formalism::TaskView task)
{
    if (auto mapped = find_mapped(this->m_context.storage->tasks, task))
        return { *mapped, false };
    auto data = formalism::checkout<formalism::Task>(this->m_context.builder);
    data->name = task.get_data().name;
    data->domain = this->m_context.storage->translated_domain->get_index();
    this->self().template copy_list<formalism::Requirement>(task.get_requirements(), data->requirements);
    this->self().template copy_list<formalism::Object>(task.get_objects(), data->objects);
    this->self().template copy_list<formalism::Literal>(task.get_initial_literals(), data->initial_literals);
    this->self().template copy_list<formalism::InitialFunctionValue>(task.get_initial_function_values(), data->initial_function_values);

    if (const auto goal = task.get_goal())
    {
        if (this->m_context.phase == TranslationPhase::RenameQuantifiedVariables)
        {
            this->m_context.num_quantifications.clear();
            this->self().enter_variable_scope();
            const auto renamed_goal = this->self().rename_variables(goal.value());
            this->self().leave_variable_scope();
            const auto previous = this->m_context.renaming_enabled;
            this->m_context.renaming_enabled = false;
            data->goal = as_index(this->self().copy(renamed_goal));
            this->m_context.renaming_enabled = previous;
        }
        else
        {
            data->goal = as_index(this->self().copy(goal.value()));
        }
    }
    else
    {
        data->goal = {};
    }

    if (const auto metric = task.get_metric())
        data->metric = as_index(this->self().copy(metric.value()));
    else
        data->metric = {};
    this->self().template copy_list<formalism::Predicate>(task.get_predicates(), data->predicates);
    this->self().template copy_list<formalism::Axiom>(task.get_axioms(), data->axioms);

    auto [copied_task, copied_inserted] = formalism::insert(this->m_context.storage->repository, *data);
    data->index = {};

    switch (this->m_context.phase)
    {
        case TranslationPhase::RemoveUniversalQuantifiers:
        case TranslationPhase::SimplifyGoal:
        {
            if (this->m_context.phase == TranslationPhase::SimplifyGoal && data->goal)
                data->goal = as_index(this->self().simplify_goal_condition(copied_task.get_goal().value()));

            auto existing_predicates = ygg::UnorderedSet<formalism::PredicateView> {};
            for (auto predicate : copied_task.get_predicates())
                existing_predicates.insert(predicate);
            for (auto predicate : this->m_context.generated_predicates)
                if (existing_predicates.insert(predicate).second)
                    data->predicates.push_back(predicate.get_index());

            auto existing_axioms = ygg::UnorderedSet<formalism::AxiomView> {};
            for (auto axiom : copied_task.get_axioms())
                existing_axioms.insert(axiom);
            for (auto axiom : this->m_context.generated_axioms)
                if (existing_axioms.insert(axiom).second)
                    data->axioms.push_back(axiom.get_index());

            if (!data->axioms.empty())
                this->self().ensure_derived_predicates_requirement(copied_task.get_requirements(), data->requirements);
            break;
        }
        case TranslationPhase::SplitDisjunctiveConditions:
            this->self().split_disjunctive_axioms(copied_task.get_axioms(), data->axioms);
            break;
        case TranslationPhase::MaterializeEquality:
            this->self().materialize_equality(*data, task);
            break;
        case TranslationPhase::CompileTyping:
            this->self().initialize_type_literals(*data, task);
            break;
        default:
            break;
    }

    auto [view, inserted] = formalism::insert(this->m_context.storage->repository, *data);
    remember(this->m_context.storage->tasks, task, view);
    return { view, inserted || (copied_inserted && view == copied_task) };
}

}  // namespace loki::semantic::detail

#endif
