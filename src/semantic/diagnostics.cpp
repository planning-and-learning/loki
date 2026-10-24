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

#include "diagnostics.hpp"

namespace loki::semantic
{

DiagnosticContext::Scope::Scope(DiagnosticContext& diagnostics_, const parser::ErrorHandlerType& error_handler) :
    diagnostics(diagnostics_),
    previous(diagnostics_.active)
{ diagnostics.active = std::cref(error_handler); }

DiagnosticContext::Scope::~Scope() { diagnostics.active = previous; }

ParseError DiagnosticContext::parse_error(const parser::ErrorHandlerType& error_handler, const std::string& fallback, parser::Iterator position)
{
    auto error = ParseError(fallback);
    error.set_diagnostic(error_handler.diagnostic(fallback, position));
    return error;
}

}  // namespace loki::semantic
