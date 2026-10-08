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

formalism::RequirementView CanonicalCopyTranslator::copy(formalism::RequirementView source)
{
    if (auto mapped = find_mapped(m_storage->requirements, source))
        return *mapped;
    auto data = formalism::checkout<formalism::Requirement>(m_builder);
    data->kind = source.get_kind();
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->requirements, source, out);
    return out;
}

formalism::TypeView CanonicalCopyTranslator::copy(formalism::TypeView source)
{
    if (auto mapped = find_mapped(m_storage->types, source))
        return *mapped;
    auto data = formalism::checkout<formalism::Type>(m_builder);
    data->name = source.get_name();
    copy_list(source.get_bases(), data->bases);
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->types, source, out);
    return out;
}

formalism::ObjectView CanonicalCopyTranslator::copy(formalism::ObjectView source)
{
    if (auto mapped = find_mapped(m_storage->objects, source))
        return *mapped;
    auto data = formalism::checkout<formalism::Object>(m_builder);
    data->name = source.get_name();
    copy_list(source.get_types(), data->types);
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->objects, source, out);
    return out;
}

formalism::VariableView CanonicalCopyTranslator::copy(formalism::VariableView source)
{
    if (auto mapped = find_mapped(m_storage->variables, source))
        return *mapped;
    auto data = formalism::checkout<formalism::Variable>(m_builder);
    data->name = source.get_name();
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->variables, source, out);
    return out;
}

formalism::ParameterView CanonicalCopyTranslator::copy(formalism::ParameterView source)
{
    if (auto mapped = find_mapped(m_storage->parameters, source))
        return *mapped;
    auto data = formalism::checkout<formalism::Parameter>(m_builder);
    data->variable = as_index(copy(source.get_variable()));
    copy_list(source.get_types(), data->types);
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->parameters, source, out);
    return out;
}

formalism::PredicateView CanonicalCopyTranslator::copy(formalism::PredicateView source)
{
    if (auto mapped = find_mapped(m_storage->predicates, source))
        return *mapped;
    auto data = formalism::checkout<formalism::Predicate>(m_builder);
    data->name = source.get_name();
    copy_list(source.get_parameters(), data->parameters);
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->predicates, source, out);
    return out;
}

formalism::FunctionSkeletonView CanonicalCopyTranslator::copy(formalism::FunctionSkeletonView source)
{
    if (auto mapped = find_mapped(m_storage->functions, source))
        return *mapped;
    auto data = formalism::checkout<formalism::FunctionSkeleton>(m_builder);
    data->name = source.get_name();
    copy_list(source.get_parameters(), data->parameters);
    data->type = as_index(copy(source.get_type()));
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->functions, source, out);
    return out;
}

formalism::TermView CanonicalCopyTranslator::copy(formalism::TermView source)
{
    if (auto mapped = find_mapped(m_storage->terms, source))
        return *mapped;
    auto data = formalism::checkout<formalism::Term>(m_builder);
    data->variant = ygg::visit([&](const auto& arg) -> ygg::Data<formalism::Term>::Variant { return as_index(copy(arg)); }, source.get_variant());
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->terms, source, out);
    return out;
}

formalism::AtomView CanonicalCopyTranslator::copy(formalism::AtomView source)
{
    if (auto mapped = find_mapped(m_storage->atoms, source))
        return *mapped;
    auto data = formalism::checkout<formalism::Atom>(m_builder);
    data->predicate = as_index(copy(source.get_predicate()));
    copy_list(source.get_terms(), data->terms);
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->atoms, source, out);
    return out;
}

formalism::LiteralView CanonicalCopyTranslator::copy(formalism::LiteralView source)
{
    if (auto mapped = find_mapped(m_storage->literals, source))
        return *mapped;
    auto data = formalism::checkout<formalism::Literal>(m_builder);
    data->atom = as_index(copy(source.get_atom()));
    data->m_polarity = source.get_polarity();
    auto out = formalism::insert(m_storage->repository, *data).first;
    remember(m_storage->literals, source, out);
    return out;
}

}  // namespace loki::semantic::detail
