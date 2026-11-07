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

template<typename T>
formalism::EntityView<T> CanonicalCopyTranslator::copy_child(formalism::EntityView<T> source)
{
    return memoized(m_storage->maps, source, [&] { return rebuild(m_storage->repository, source, [&](auto child) { return copy_child(child); }); });
}

formalism::DomainView CanonicalCopyTranslator::copy_child(formalism::DomainView source)
{ return m_storage->translated_domain ? m_storage->translated() : copy(source).first; }

std::pair<formalism::DomainView, bool> CanonicalCopyTranslator::copy(formalism::DomainView domain)
{
    if (auto mapped = find_mapped(m_storage->maps, domain))
    {
        m_storage->translated_domain = *mapped;
        return { *mapped, false };
    }
    if (&domain.context() == &m_storage->repository)
    {
        m_storage->translated_domain = domain;
        remember(m_storage->maps, domain, domain);
        return { domain, false };
    }
    auto data = urd::map_children<formalism::SymbolRepositoryTypes, formalism::Domain>(domain.data(),
                                                                                        domain.context(),
                                                                                        [&](auto child) { return urd::to_stored(copy_child(child)); });
    auto [view, inserted] = formalism::insert(m_storage->repository, data);
    m_storage->translated_domain = view;
    remember(m_storage->maps, domain, view);
    return { view, inserted };
}

std::pair<formalism::TaskView, bool> CanonicalCopyTranslator::copy(formalism::TaskView task)
{
    if (auto mapped = find_mapped(m_storage->maps, task))
        return { *mapped, false };
    if (&task.context() == &m_storage->repository)
    {
        remember(m_storage->maps, task, task);
        return { task, false };
    }
    auto data =
        urd::map_children<formalism::SymbolRepositoryTypes, formalism::Task>(task.data(), task.context(), [&](auto child) { return urd::to_stored(copy_child(child)); });
    auto [view, inserted] = formalism::insert(m_storage->repository, data);
    remember(m_storage->maps, task, view);
    return { view, inserted };
}

}  // namespace loki::semantic::detail
