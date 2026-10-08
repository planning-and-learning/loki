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

CanonicalCopyTranslator::CanonicalCopyTranslator(std::shared_ptr<TranslationStorage> storage) : m_storage(std::move(storage)) {}

std::pair<formalism::DomainView, bool> CanonicalCopyTranslator::copy(formalism::DomainView domain)
{
    if (auto mapped = find_mapped(m_storage->domains, domain))
    {
        m_storage->translated_domain = *mapped;
        return { *mapped, false };
    }
    if (&domain.get_context() == &m_storage->repository)
    {
        m_storage->translated_domain = domain;
        remember(m_storage->domains, domain, domain);
        return { domain, false };
    }
    auto data = formalism::checkout<formalism::Domain>(m_builder);
    data->name = domain.get_data().name;
    copy_list(domain.get_requirements(), data->requirements);
    copy_list(domain.get_types(), data->types);
    copy_list(domain.get_constants(), data->constants);
    copy_list(domain.get_predicates(), data->predicates);
    copy_list(domain.get_functions(), data->functions);
    copy_list(domain.get_actions(), data->actions);
    copy_list(domain.get_axioms(), data->axioms);
    auto [view, inserted] = formalism::insert(m_storage->repository, *data);
    m_storage->translated_domain = view;
    remember(m_storage->domains, domain, view);
    return { view, inserted };
}

std::pair<formalism::TaskView, bool> CanonicalCopyTranslator::copy(formalism::TaskView task)
{
    if (auto mapped = find_mapped(m_storage->tasks, task))
        return { *mapped, false };
    if (&task.get_context() == &m_storage->repository)
    {
        remember(m_storage->tasks, task, task);
        return { task, false };
    }
    auto data = formalism::checkout<formalism::Task>(m_builder);
    data->name = task.get_data().name;
    data->domain = m_storage->translated_domain ? m_storage->translated_domain->get_index() : copy(task.get_domain()).first.get_index();
    copy_list(task.get_requirements(), data->requirements);
    copy_list(task.get_objects(), data->objects);
    copy_list(task.get_initial_literals(), data->initial_literals);
    copy_list(task.get_initial_function_values(), data->initial_function_values);
    if (const auto source_goal = task.get_goal())
        data->goal = as_index(copy(source_goal.value()));
    if (const auto source_metric = task.get_metric())
        data->metric = as_index(copy(source_metric.value()));
    copy_list(task.get_predicates(), data->predicates);
    copy_list(task.get_axioms(), data->axioms);
    auto [view, inserted] = formalism::insert(m_storage->repository, *data);
    remember(m_storage->tasks, task, view);
    return { view, inserted };
}

formalism::EffectLiteralView CanonicalCopyTranslator::copy(formalism::EffectLiteralView source)
{
    if (auto mapped = find_mapped(m_storage->effect_literals, source))
        return *mapped;
    auto data = formalism::checkout<formalism::EffectLiteral>(m_builder);
    data->literal = as_index(copy(source.get_literal()));
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->effect_literals, source, out);
    return out;
}

formalism::EffectAndView CanonicalCopyTranslator::copy(formalism::EffectAndView source)
{
    if (auto mapped = find_mapped(m_storage->effect_ands, source))
        return *mapped;
    auto data = formalism::checkout<formalism::EffectAnd>(m_builder);
    copy_list(source.get_effects(), data->effects);
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->effect_ands, source, out);
    return out;
}

formalism::EffectNumericView CanonicalCopyTranslator::copy(formalism::EffectNumericView source)
{
    if (auto mapped = find_mapped(m_storage->effect_numerics, source))
        return *mapped;
    auto data = formalism::checkout<formalism::EffectNumeric>(m_builder);
    data->op = source.get_data().op;
    data->function = as_index(copy(source.get_function()));
    data->expression = as_index(copy(source.get_expression()));
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->effect_numerics, source, out);
    return out;
}

formalism::EffectForallView CanonicalCopyTranslator::copy(formalism::EffectForallView source)
{
    if (auto mapped = find_mapped(m_storage->effect_foralls, source))
        return *mapped;
    auto data = formalism::checkout<formalism::EffectForall>(m_builder);
    copy_list(source.get_parameters(), data->parameters);
    data->effect = as_index(copy(source.get_effect()));
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->effect_foralls, source, out);
    return out;
}

formalism::EffectWhenView CanonicalCopyTranslator::copy(formalism::EffectWhenView source)
{
    if (auto mapped = find_mapped(m_storage->effect_whens, source))
        return *mapped;
    auto data = formalism::checkout<formalism::EffectWhen>(m_builder);
    data->condition = as_index(copy(source.get_condition()));
    data->effect = as_index(copy(source.get_effect()));
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->effect_whens, source, out);
    return out;
}

formalism::EffectOneOfView CanonicalCopyTranslator::copy(formalism::EffectOneOfView source)
{
    if (auto mapped = find_mapped(m_storage->effect_one_ofs, source))
        return *mapped;
    auto data = formalism::checkout<formalism::EffectOneOf>(m_builder);
    copy_list(source.get_effects(), data->effects);
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->effect_one_ofs, source, out);
    return out;
}

formalism::EffectProbabilisticAlternativeView CanonicalCopyTranslator::copy(formalism::EffectProbabilisticAlternativeView source)
{
    if (auto mapped = find_mapped(m_storage->effect_probabilistic_alternatives, source))
        return *mapped;
    auto data = formalism::checkout<formalism::EffectProbabilisticAlternative>(m_builder);
    data->probability = source.get_data().probability;
    data->effect = as_index(copy(source.get_effect()));
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->effect_probabilistic_alternatives, source, out);
    return out;
}

formalism::EffectProbabilisticView CanonicalCopyTranslator::copy(formalism::EffectProbabilisticView source)
{
    if (auto mapped = find_mapped(m_storage->effect_probabilistics, source))
        return *mapped;
    auto data = formalism::checkout<formalism::EffectProbabilistic>(m_builder);
    copy_list(source.get_alternatives(), data->alternatives);
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->effect_probabilistics, source, out);
    return out;
}

formalism::EffectView CanonicalCopyTranslator::copy(formalism::EffectView source)
{
    if (auto mapped = find_mapped(m_storage->effects, source))
        return *mapped;
    auto data = formalism::checkout<formalism::Effect>(m_builder);
    data->variant = ygg::visit([&](const auto& arg) -> ygg::Data<formalism::Effect>::Variant { return as_index(copy(arg)); }, source.get_variant());
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->effects, source, out);
    return out;
}

formalism::ActionView CanonicalCopyTranslator::copy(formalism::ActionView source)
{
    if (auto mapped = find_mapped(m_storage->actions, source))
        return *mapped;
    const auto& data = source.get_data();
    auto out_data = formalism::checkout<formalism::Action>(m_builder);
    out_data->name = data.name;
    out_data->original_name = data.original_name;
    out_data->original_arity = data.original_arity;
    if (const auto condition = source.get_precondition())
        out_data->precondition = as_index(copy(condition.value()));
    if (const auto effect_view = source.get_effect())
        out_data->effect = as_index(copy(effect_view.value()));
    copy_list(source.get_parameters(), out_data->parameters);
    auto out = formalism::insert(m_storage->repository, *out_data).first;
    remember(m_storage->actions, source, out);
    return out;
}

formalism::AxiomView CanonicalCopyTranslator::copy(formalism::AxiomView source)
{
    if (auto mapped = find_mapped(m_storage->axioms, source))
        return *mapped;
    auto data = formalism::checkout<formalism::Axiom>(m_builder);
    copy_list(source.get_parameters(), data->parameters);
    data->original_arity = source.get_data().original_arity;
    data->head = as_index(copy(source.get_head()));
    data->condition = as_index(copy(source.get_condition()));
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->axioms, source, out);
    return out;
}

formalism::MetricView CanonicalCopyTranslator::copy(formalism::MetricView source)
{
    if (auto mapped = find_mapped(m_storage->metrics, source))
        return *mapped;
    auto data = formalism::checkout<formalism::Metric>(m_builder);
    data->optimization_direction = source.get_optimization_direction();
    data->expression = as_index(copy(source.get_expression()));
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->metrics, source, out);
    return out;
}

formalism::InitialFunctionValueView CanonicalCopyTranslator::copy(formalism::InitialFunctionValueView source)
{
    if (auto mapped = find_mapped(m_storage->initial_function_values, source))
        return *mapped;
    auto data = formalism::checkout<formalism::InitialFunctionValue>(m_builder);
    data->function = as_index(copy(source.get_function()));
    data->value = as_index(copy(source.get_value()));
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->initial_function_values, source, out);
    return out;
}

}  // namespace loki::semantic::detail
