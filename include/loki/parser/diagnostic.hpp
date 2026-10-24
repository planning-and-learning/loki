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

#ifndef LOKI_PARSER_DIAGNOSTIC_HPP_
#define LOKI_PARSER_DIAGNOSTIC_HPP_

#include <string>
#include <vector>
#include <yggdrasil/diagnostics/x3.hpp>

namespace loki::parser
{
using Iterator = std::string::const_iterator;
using Position = boost::spirit::x3::position_tagged;
using PositionList = std::vector<Position>;

using ygg::diagnostics::ErrorHandler;
using ygg::diagnostics::format_error_at;
using ErrorHandlerType = ErrorHandler<Iterator>;

}  // namespace loki::parser

#endif
