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

#ifndef LOKI_SEMANTIC_TRANSLATOR_CANONICAL_COPY_TRANSLATOR_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_CANONICAL_COPY_TRANSLATOR_HPP_

#include "loki/formalism/repository.hpp"

#include <memory>
#include <utility>

namespace loki::semantic::detail
{

struct TranslationStorage;

class CanonicalCopyTranslator
{
public:
    explicit CanonicalCopyTranslator(std::shared_ptr<TranslationStorage> storage);

    std::pair<formalism::DomainView, bool> copy(formalism::DomainView domain);
    std::pair<formalism::TaskView, bool> copy(formalism::TaskView task);

private:
    std::shared_ptr<TranslationStorage> m_storage;

    /// The copy of every entity below the domain and the task: memoized, children copied first.
    template<typename T>
    formalism::EntityView<T> copy_child(formalism::EntityView<T> source);
    /// A task's domain: the translated domain once there is one.
    formalism::DomainView copy_child(formalism::DomainView source);
};

inline auto copy(formalism::DomainView source, CanonicalCopyTranslator& context) { return context.copy(source); }
inline auto copy(formalism::TaskView source, CanonicalCopyTranslator& context) { return context.copy(source); }

}  // namespace loki::semantic::detail

#endif
