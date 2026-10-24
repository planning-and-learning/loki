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

#ifndef LOKI_SEMANTIC_DIAGNOSTICS_HPP_
#define LOKI_SEMANTIC_DIAGNOSTICS_HPP_

#include "loki/parser/diagnostic.hpp"
#include "loki/semantic/errors.hpp"

#include <functional>
#include <optional>
#include <string>

namespace loki::semantic
{

struct DiagnosticContext
{
    std::optional<std::reference_wrapper<const parser::ErrorHandlerType>> active;

    struct Scope
    {
        DiagnosticContext& diagnostics;
        std::optional<std::reference_wrapper<const parser::ErrorHandlerType>> previous;

        Scope(DiagnosticContext& diagnostics, const parser::ErrorHandlerType& error_handler);
        ~Scope();

        Scope(const Scope&) = delete;
        Scope& operator=(const Scope&) = delete;
        Scope(Scope&&) = delete;
        Scope& operator=(Scope&&) = delete;
    };

    static ParseError parse_error(const parser::ErrorHandlerType& error_handler, const std::string& fallback, parser::Iterator position);

    template<typename Node, typename Error>
    [[noreturn]] void throw_at(const Node& node, Error error) const
    {
        if (active)
            error.set_diagnostic(active->get().make_diagnostic(node, error.message()));
        throw error;
    }
};

}  // namespace loki::semantic

#endif
