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

#include "loki/formalism/repository.hpp"
#include "loki/semantic/translator/canonical_copy_translator.hpp"
#include "loki/semantic/translator/common.hpp"

#include <utility>

namespace loki::semantic::detail
{

template<typename T>
void compose_map(ViewMap<T>& out, const ViewMap<T>& source_to_middle, const ViewMap<T>& middle_to_target)
{
    ViewMap<T> composed;
    for (const auto& [source, middle] : source_to_middle)
    {
        if (auto it = middle_to_target.find(middle); it != middle_to_target.end())
            composed.emplace(source, it->second);
    }
    // Either input may alias out; replace it only after reading both maps.
    out = std::move(composed);
}

formalism::TypeView copy_type_view_for_metadata(TranslationStorage& target, formalism::Builder& builder, formalism::TypeView source)
{
    if (auto mapped = find_mapped(target.types, source))
        return *mapped;

    auto data = formalism::checkout<formalism::Type>(builder);
    data->name = source.get_name();
    for (auto base : source.get_bases())
        data->bases.push_back(copy_type_view_for_metadata(target, builder, base).get_index());

    auto out = formalism::insert(target.repository, *data).first;
    remember(target.types, source, out);
    return out;
}

void remap_object_type_metadata(TranslationStorage& target,
                                formalism::Builder& builder,
                                const TranslationStorage& source,
                                const ViewMap<formalism::Object>& object_map,
                                const ViewMap<formalism::Type>& type_map)
{
    target.object_type_views.clear();
    for (const auto& [source_object, type_views] : source.object_type_views)
    {
        auto object_it = object_map.find(source_object);
        if (object_it == object_map.end())
            continue;

        auto remapped = std::vector<formalism::TypeView> {};
        for (auto type : type_views)
        {
            if (auto type_it = type_map.find(type); type_it != type_map.end())
                remapped.push_back(type_it->second);
            else
                remapped.push_back(copy_type_view_for_metadata(target, builder, type));
        }
        target.object_type_views.emplace(object_it->second, std::move(remapped));
    }
}

std::shared_ptr<TranslationStorage> canonicalize_domain_storage(formalism::DomainView original_domain, const std::shared_ptr<TranslationStorage>& middle)
{
    auto canonical = std::make_shared<TranslationStorage>(middle->repository.get_index());
    const auto middle_domain = middle->domains.at(original_domain);
    auto context = CanonicalCopyTranslator(canonical);
    const auto canonical_domain = copy(middle_domain, context).first;

    const auto middle_objects = canonical->objects;
    const auto middle_types = canonical->types;
    auto builder = formalism::Builder {};
    remap_object_type_metadata(*canonical, builder, *middle, middle_objects, middle_types);

    canonical->translated_domain = canonical_domain;
    canonical->domains.clear();
    remember(canonical->domains, original_domain, canonical_domain);
    compose_map(canonical->requirements, middle->requirements, canonical->requirements);
    compose_map(canonical->types, middle->types, canonical->types);
    compose_map(canonical->objects, middle->objects, canonical->objects);
    compose_map(canonical->variables, middle->variables, canonical->variables);
    compose_map(canonical->parameters, middle->parameters, canonical->parameters);
    compose_map(canonical->predicates, middle->predicates, canonical->predicates);
    compose_map(canonical->functions, middle->functions, canonical->functions);
    compose_map(canonical->terms, middle->terms, canonical->terms);
    compose_map(canonical->atoms, middle->atoms, canonical->atoms);
    compose_map(canonical->literals, middle->literals, canonical->literals);
    compose_map(canonical->numbers, middle->numbers, canonical->numbers);
    compose_map(canonical->function_terms, middle->function_terms, canonical->function_terms);
    compose_map(canonical->unary_expressions, middle->unary_expressions, canonical->unary_expressions);
    compose_map(canonical->binary_expressions, middle->binary_expressions, canonical->binary_expressions);
    compose_map(canonical->multi_expressions, middle->multi_expressions, canonical->multi_expressions);
    compose_map(canonical->function_expressions, middle->function_expressions, canonical->function_expressions);
    compose_map(canonical->condition_literals, middle->condition_literals, canonical->condition_literals);
    compose_map(canonical->condition_ands, middle->condition_ands, canonical->condition_ands);
    compose_map(canonical->condition_ors, middle->condition_ors, canonical->condition_ors);
    compose_map(canonical->condition_nots, middle->condition_nots, canonical->condition_nots);
    compose_map(canonical->condition_implies, middle->condition_implies, canonical->condition_implies);
    compose_map(canonical->condition_exists, middle->condition_exists, canonical->condition_exists);
    compose_map(canonical->condition_foralls, middle->condition_foralls, canonical->condition_foralls);
    compose_map(canonical->condition_numeric_constraints, middle->condition_numeric_constraints, canonical->condition_numeric_constraints);
    compose_map(canonical->conditions, middle->conditions, canonical->conditions);
    compose_map(canonical->effect_literals, middle->effect_literals, canonical->effect_literals);
    compose_map(canonical->effect_ands, middle->effect_ands, canonical->effect_ands);
    compose_map(canonical->effect_numerics, middle->effect_numerics, canonical->effect_numerics);
    compose_map(canonical->effect_foralls, middle->effect_foralls, canonical->effect_foralls);
    compose_map(canonical->effect_whens, middle->effect_whens, canonical->effect_whens);
    compose_map(canonical->effect_one_ofs, middle->effect_one_ofs, canonical->effect_one_ofs);
    compose_map(canonical->effect_probabilistic_alternatives, middle->effect_probabilistic_alternatives, canonical->effect_probabilistic_alternatives);
    compose_map(canonical->effect_probabilistics, middle->effect_probabilistics, canonical->effect_probabilistics);
    compose_map(canonical->effects, middle->effects, canonical->effects);
    compose_map(canonical->actions, middle->actions, canonical->actions);
    compose_map(canonical->axioms, middle->axioms, canonical->axioms);
    compose_map(canonical->metrics, middle->metrics, canonical->metrics);
    compose_map(canonical->initial_function_values, middle->initial_function_values, canonical->initial_function_values);
    compose_map(canonical->tasks, middle->tasks, canonical->tasks);
    return canonical;
}

template<typename T>
void copy_map(ViewMap<T>& target, const ViewMap<T>& source)
{
    target = source;
}

void compose_storage_maps_from_previous(TranslationStorage& target, const TranslationStorage& previous)
{
    const auto precompose_objects = target.objects;
    const auto precompose_types = target.types;
    auto builder = formalism::Builder {};
    remap_object_type_metadata(target, builder, previous, precompose_objects, precompose_types);

    compose_map(target.requirements, previous.requirements, target.requirements);
    compose_map(target.types, previous.types, target.types);
    compose_map(target.objects, previous.objects, target.objects);
    compose_map(target.variables, previous.variables, target.variables);
    compose_map(target.parameters, previous.parameters, target.parameters);
    compose_map(target.predicates, previous.predicates, target.predicates);
    compose_map(target.functions, previous.functions, target.functions);
    compose_map(target.terms, previous.terms, target.terms);
    compose_map(target.atoms, previous.atoms, target.atoms);
    compose_map(target.literals, previous.literals, target.literals);
    compose_map(target.numbers, previous.numbers, target.numbers);
    compose_map(target.function_terms, previous.function_terms, target.function_terms);
    compose_map(target.unary_expressions, previous.unary_expressions, target.unary_expressions);
    compose_map(target.binary_expressions, previous.binary_expressions, target.binary_expressions);
    compose_map(target.multi_expressions, previous.multi_expressions, target.multi_expressions);
    compose_map(target.function_expressions, previous.function_expressions, target.function_expressions);
    compose_map(target.condition_literals, previous.condition_literals, target.condition_literals);
    compose_map(target.condition_ands, previous.condition_ands, target.condition_ands);
    compose_map(target.condition_ors, previous.condition_ors, target.condition_ors);
    compose_map(target.condition_nots, previous.condition_nots, target.condition_nots);
    compose_map(target.condition_implies, previous.condition_implies, target.condition_implies);
    compose_map(target.condition_exists, previous.condition_exists, target.condition_exists);
    compose_map(target.condition_foralls, previous.condition_foralls, target.condition_foralls);
    compose_map(target.condition_numeric_constraints, previous.condition_numeric_constraints, target.condition_numeric_constraints);
    compose_map(target.conditions, previous.conditions, target.conditions);
    compose_map(target.effect_literals, previous.effect_literals, target.effect_literals);
    compose_map(target.effect_ands, previous.effect_ands, target.effect_ands);
    compose_map(target.effect_numerics, previous.effect_numerics, target.effect_numerics);
    compose_map(target.effect_foralls, previous.effect_foralls, target.effect_foralls);
    compose_map(target.effect_whens, previous.effect_whens, target.effect_whens);
    compose_map(target.effect_one_ofs, previous.effect_one_ofs, target.effect_one_ofs);
    compose_map(target.effect_probabilistic_alternatives, previous.effect_probabilistic_alternatives, target.effect_probabilistic_alternatives);
    compose_map(target.effect_probabilistics, previous.effect_probabilistics, target.effect_probabilistics);
    compose_map(target.effects, previous.effects, target.effects);
    compose_map(target.actions, previous.actions, target.actions);
    compose_map(target.axioms, previous.axioms, target.axioms);
    compose_map(target.metrics, previous.metrics, target.metrics);
    compose_map(target.initial_function_values, previous.initial_function_values, target.initial_function_values);
    compose_map(target.domains, previous.domains, target.domains);
    compose_map(target.tasks, previous.tasks, target.tasks);
}

void inherit_domain_mappings(TranslationStorage& problem, const TranslationStorage& domain)
{
    copy_map(problem.domains, domain.domains);
    copy_map(problem.requirements, domain.requirements);
    copy_map(problem.types, domain.types);
    copy_map(problem.objects, domain.objects);
    copy_map(problem.predicates, domain.predicates);
    copy_map(problem.functions, domain.functions);
    problem.object_type_views = domain.object_type_views;
    problem.translated_domain = domain.translated_domain;
}

template<typename T>
void copy_identity_map(ViewMap<T>& target, const ViewMap<T>& source)
{
    for (const auto& [_, view] : source)
        target.emplace(view, view);
}

void inherit_domain_identity_mappings(TranslationStorage& problem, const TranslationStorage& domain)
{
    problem.domains.emplace(*domain.translated_domain, *domain.translated_domain);
    copy_identity_map(problem.requirements, domain.requirements);
    copy_identity_map(problem.types, domain.types);
    copy_identity_map(problem.objects, domain.objects);
    copy_identity_map(problem.predicates, domain.predicates);
    copy_identity_map(problem.functions, domain.functions);
    problem.object_type_views = domain.object_type_views;
    problem.translated_domain = domain.translated_domain;
}

std::shared_ptr<TranslationStorage>
canonicalize_problem_storage(formalism::TaskView middle_task,
                             const std::shared_ptr<TranslationStorage>& middle,
                             std::shared_ptr<const TranslationStorage> domain)
{
    auto canonical = std::make_shared<TranslationStorage>(middle->repository.get_index(), domain);
    inherit_domain_identity_mappings(*canonical, *domain);
    if (middle->translated_domain->get_index() == domain->translated_domain->get_index())
        remember(canonical->domains, *middle->translated_domain, *domain->translated_domain);
    auto context = CanonicalCopyTranslator(canonical);
    copy(middle_task, context);
    compose_storage_maps_from_previous(*canonical, *middle);
    return canonical;
}

}  // namespace loki::semantic::detail
