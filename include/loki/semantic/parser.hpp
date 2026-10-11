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

#ifndef LOKI_SEMANTIC_PARSER_HPP_
#define LOKI_SEMANTIC_PARSER_HPP_

#include "loki/formalism/declarations.hpp"
#include "loki/semantic/options.hpp"

#include <filesystem>
#include <memory>
#include <string>

namespace loki::semantic
{
std::string read_file(const std::filesystem::path& path);

class Parser
{
public:
    explicit Parser(const std::string& domain_source, ParserOptions options = {});
    explicit Parser(const std::filesystem::path& domain_path, ParserOptions options = {});
    ~Parser();

    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;
    Parser(Parser&&) noexcept;
    Parser& operator=(Parser&&) noexcept;

    const formalism::Repository& repository() const noexcept;
    formalism::Repository& repository() noexcept;

    formalism::DomainView get_domain() const noexcept;

    formalism::TaskView parse_task(const std::string& source);
    formalism::TaskView parse_task(const std::filesystem::path& path);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
}  // namespace loki::semantic

#endif
