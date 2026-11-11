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

#include "loki/formalism/detail/repository_instantiations.hpp"
#include "loki/formalism/repository.hpp"
#include "translator/canonical_copy_translator.hpp"
#include "translator/common.hpp"

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

template<typename T>
std::pair<formalism::EntityView<T>, bool> CanonicalCopyTranslator::copy_root(formalism::EntityView<T> source)
{
    if (auto mapped = find_mapped(m_storage->maps, source))
        return { *mapped, false };
    if (&source.context() == &m_storage->repository)
    {
        remember(m_storage->maps, source, source);
        return { source, false };
    }
    auto data =
        urd::map_children<formalism::SymbolRepositoryTypes, T>(source.data(), source.context(), [&](auto child) { return urd::to_stored(copy_child(child)); });
    auto [view, inserted] = formalism::insert(m_storage->repository, data);
    remember(m_storage->maps, source, view);
    return { view, inserted };
}

std::pair<formalism::DomainView, bool> CanonicalCopyTranslator::copy(formalism::DomainView domain)
{
    const auto result = copy_root(domain);
    m_storage->translated_domain = result.first;
    return result;
}

std::pair<formalism::TaskView, bool> CanonicalCopyTranslator::copy(formalism::TaskView task) { return copy_root(task); }

}  // namespace loki::semantic::detail
