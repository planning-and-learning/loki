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

#include "loki/semantic/parser.hpp"

#include "builder.hpp"
#include "context.hpp"
#include "diagnostics.hpp"
#include "loki/ast/ast.hpp"
#include "loki/semantic/errors.hpp"
#include "loki/semantic/options.hpp"
#include "loki/semantic/translator/canonical_copy_translator.hpp"
#include "loki/semantic/translator/common.hpp"
#include "mappings.hpp"
#include "parser_runtime.hpp"

#include <fstream>
#include <sstream>
#include <utility>

namespace loki::semantic
{

std::string read_file(const fs::path& path)
{
    std::ifstream in(path);
    if (!in)
        throw ParseError("Could not open PDDL file: " + path.string());
    std::ostringstream out;
    out << in.rdbuf();
    return out.str();
}

// Orchestrates the pipeline: source text -> syntactic AST -> formalism entities
// (AstBuilder) -> canonicalized storage (CanonicalCopyTranslator).
struct Parser::Impl
{
    Impl(std::string domain_source, std::string source_name, ParserOptions options);

    const formalism::Repository& repository() const noexcept;
    formalism::Repository& repository() noexcept;
    formalism::DomainView get_domain() const noexcept;
    formalism::TaskView parse_task(const std::string& source);
    formalism::TaskView parse_task(const fs::path& path);

private:
    ParserOptions m_options;
    DiagnosticContext m_diagnostics;
    DomainContext m_domain_context;
    std::vector<std::shared_ptr<detail::TranslationStorage>> m_task_storages;

    formalism::Repository& repo() noexcept;
    const formalism::Repository& repo() const noexcept;

    formalism::DomainView parse_domain_ast(const ast::Domain& domain);
    formalism::TaskView parse_task_source(std::string source, const std::string& source_name);
    formalism::TaskView parse_task_ast(const ast::Task& task);

    void canonicalize_domain(formalism::DomainView domain);
    formalism::TaskView canonicalize_task(formalism::TaskView task, const std::shared_ptr<detail::TranslationStorage>& domain_storage);
    void reset_domain();
};

Parser::Parser(const std::string& domain_source, ParserOptions options) : m_impl(std::make_unique<Impl>(domain_source, "", options)) {}

Parser::Parser(const fs::path& domain_path, ParserOptions options) : m_impl(std::make_unique<Impl>(read_file(domain_path), domain_path.string(), options)) {}

Parser::~Parser() = default;
Parser::Parser(Parser&&) noexcept = default;
Parser& Parser::operator=(Parser&&) noexcept = default;

const formalism::Repository& Parser::repository() const noexcept { return m_impl->repository(); }
formalism::Repository& Parser::repository() noexcept { return m_impl->repository(); }
formalism::DomainView Parser::get_domain() const noexcept { return m_impl->get_domain(); }
formalism::TaskView Parser::parse_task(const std::string& source) { return m_impl->parse_task(source); }
formalism::TaskView Parser::parse_task(const fs::path& path) { return m_impl->parse_task(path); }

Parser::Impl::Impl(std::string domain_source, std::string source_name, ParserOptions options) :
    m_options(options),
    m_domain_context(std::make_shared<detail::TranslationStorage>(0))
{
    domain_source = lowercase(std::move(domain_source));
    auto first = domain_source.cbegin();
    auto syntax_errors = std::ostringstream {};
    parser::ErrorHandlerType error_handler(first, domain_source.cend(), syntax_errors, std::move(source_name));
    ast::Domain domain_ast;
    if (!parser::detail::parse_domain_full(first, domain_source.cend(), domain_ast, error_handler))
        throw DiagnosticContext::parse_error(error_handler, "Could not parse PDDL domain.", first);
    auto scope = DiagnosticContext::Scope { m_diagnostics, error_handler };
    parse_domain_ast(domain_ast);
}

const formalism::Repository& Parser::Impl::repository() const noexcept { return repo(); }
formalism::Repository& Parser::Impl::repository() noexcept { return repo(); }
formalism::DomainView Parser::Impl::get_domain() const noexcept { return *m_domain_context.domain; }
formalism::TaskView Parser::Impl::parse_task(const std::string& source) { return parse_task_source(source, ""); }

formalism::TaskView Parser::Impl::parse_task(const fs::path& path) { return parse_task_source(read_file(path), path.string()); }

formalism::Repository& Parser::Impl::repo() noexcept { return m_domain_context.storage->repository; }
const formalism::Repository& Parser::Impl::repo() const noexcept { return m_domain_context.storage->repository; }

formalism::TaskView Parser::Impl::parse_task_source(std::string source, const std::string& source_name)
{
    source = lowercase(std::move(source));
    auto first = source.cbegin();
    auto syntax_errors = std::ostringstream {};
    parser::ErrorHandlerType error_handler(first, source.cend(), syntax_errors, source_name);
    ast::Task task_ast;
    if (!parser::detail::parse_task_full(first, source.cend(), task_ast, error_handler))
        throw DiagnosticContext::parse_error(error_handler, "Could not parse PDDL task.", first);
    auto scope = DiagnosticContext::Scope { m_diagnostics, error_handler };
    return parse_task_ast(task_ast);
}

formalism::DomainView Parser::Impl::parse_domain_ast(const ast::Domain& domain)
{
    reset_domain();

    auto parse_context = ParseContext {};
    auto view = AstBuilder(m_options, m_diagnostics, repo(), m_domain_context, parse_context).build_domain(domain);
    const auto declared_requirements = m_domain_context.requirement_kinds;
    const auto declared_action_costs = m_domain_context.action_costs;
    const auto declared_numeric_fluents = m_domain_context.numeric_fluents;
    canonicalize_domain(view);
    m_domain_context.requirement_kinds = declared_requirements;
    m_domain_context.action_costs = declared_action_costs;
    m_domain_context.numeric_fluents = declared_numeric_fluents;
    m_domain_context.domain_name = domain.name.text;
    return get_domain();
}

formalism::TaskView Parser::Impl::parse_task_ast(const ast::Task& task)
{
    const auto domain_storage = m_domain_context.storage;
    auto parse_storage = std::make_shared<detail::TranslationStorage>(m_task_storages.size() + 1, domain_storage);
    detail::inherit_domain_identity_mappings(*parse_storage, *domain_storage);

    auto task_domain_context = m_domain_context;
    auto parse_context = ParseContext {};
    parse_context.active_requirements = m_domain_context.requirement_kinds;
    parse_context.active_action_costs = m_domain_context.action_costs;
    parse_context.active_numeric_fluents = m_domain_context.numeric_fluents;
    parse_context.declared_objects = m_domain_context.declared_objects;
    auto view = AstBuilder(m_options, m_diagnostics, parse_storage->repository, task_domain_context, parse_context).build_task(task);
    return canonicalize_task(view, domain_storage);
}

void Parser::Impl::canonicalize_domain(formalism::DomainView domain)
{
    auto canonical = std::make_shared<detail::TranslationStorage>(0);
    auto context = detail::CanonicalCopyTranslator(canonical);
    auto copied = detail::copy(domain, context).first;
    m_domain_context.storage = std::move(canonical);
    m_task_storages.clear();
    m_domain_context.domain = copied;
    rebuild_domain_symbols(m_domain_context, repo());
}

formalism::TaskView Parser::Impl::canonicalize_task(formalism::TaskView task, const std::shared_ptr<detail::TranslationStorage>& domain_storage)
{
    auto canonical = std::make_shared<detail::TranslationStorage>(m_task_storages.size() + 1, domain_storage);
    detail::inherit_domain_identity_mappings(*canonical, *domain_storage);
    auto context = detail::CanonicalCopyTranslator(canonical);
    auto copied = detail::copy(task, context).first;
    m_task_storages.push_back(canonical);
    return copied;
}

void Parser::Impl::reset_domain()
{
    m_domain_context = DomainContext(std::make_shared<detail::TranslationStorage>(0));
    m_task_storages.clear();
}

}  // namespace loki::semantic
