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

#ifndef LOKI_SEMANTIC_TRANSLATOR_COPY_TRANSLATOR_COMPONENT_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_COPY_TRANSLATOR_COMPONENT_HPP_

#include "loki/formalism/repository.hpp"
#include "loki/semantic/translator/copy_context.hpp"

namespace loki::semantic::detail
{

template<typename Derived, typename Component>
class CopyTranslatorComponent
{
protected:
    explicit CopyTranslatorComponent(CopyContext& context) : m_context(context) {}

    Derived& self() noexcept { return static_cast<Derived&>(static_cast<Component&>(*this)); }
    const Derived& self() const noexcept { return static_cast<const Derived&>(static_cast<const Component&>(*this)); }

    CopyContext& m_context;
};

}  // namespace loki::semantic::detail

#endif
