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

#ifndef LOKI_PARSER_CONFIG_HPP_
#define LOKI_PARSER_CONFIG_HPP_

#include "loki/parser/diagnostic.hpp"

#include <yggdrasil/diagnostics/x3.hpp>

namespace loki::parser
{
using ErrorHandlerTag = ygg::diagnostics::ErrorHandlerTag;
using Context = ygg::diagnostics::ParserContext<ErrorHandlerType>;
}

#endif
