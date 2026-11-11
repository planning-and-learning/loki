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

#ifndef LOKI_PARSER_PARSER_HPP_
#define LOKI_PARSER_PARSER_HPP_

#include "loki/ast/ast.hpp"
#include "loki/parser/config.hpp"

#include <boost/spirit/home/x3.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

namespace loki::parser
{
namespace rules
{
struct IdentifierClass : ygg::diagnostics::ErrorHandlerBase
{
};
struct TypeExpressionClass : ygg::diagnostics::ErrorHandlerBase
{
};
struct TypedNameListClass : ygg::diagnostics::ErrorHandlerBase
{
};
struct TypedVariableListClass : ygg::diagnostics::ErrorHandlerBase
{
};
struct TermClass : ygg::diagnostics::ErrorHandlerBase
{
};
struct AtomClass : ygg::diagnostics::ErrorHandlerBase
{
};
struct LiteralClass : ygg::diagnostics::ErrorHandlerBase
{
};
struct FunctionTermClass : ygg::diagnostics::ErrorHandlerBase
{
};
struct FunctionExpressionClass : ygg::diagnostics::ErrorHandlerBase
{
};
struct ConditionClass : ygg::diagnostics::ErrorHandlerBase
{
};
struct EffectClass : ygg::diagnostics::ErrorHandlerBase
{
};
struct PredicateDeclarationClass : ygg::diagnostics::ErrorHandlerBase
{
};
struct FunctionDeclarationClass : ygg::diagnostics::ErrorHandlerBase
{
};
struct ActionClass : ygg::diagnostics::ErrorHandlerBase
{
};
struct AxiomClass : ygg::diagnostics::ErrorHandlerBase
{
};
struct MetricClass : ygg::diagnostics::ErrorHandlerBase
{
};
struct InitialFunctionValueClass : ygg::diagnostics::ErrorHandlerBase
{
};
struct DomainClass : ygg::diagnostics::ErrorHandlerBase
{
};
struct TaskClass : ygg::diagnostics::ErrorHandlerBase
{
};
struct FileClass : ygg::diagnostics::ErrorHandlerBase
{
};

using identifier_type = boost::spirit::x3::rule<IdentifierClass, ast::Identifier>;
using type_expression_type = boost::spirit::x3::rule<TypeExpressionClass, ast::TypeExpression>;
using typed_name_list_type = boost::spirit::x3::rule<TypedNameListClass, urd::Store<urd::sequence<ast::TypedGroup>>>;
using typed_variable_list_type = boost::spirit::x3::rule<TypedVariableListClass, urd::Store<urd::sequence<ast::TypedGroup>>>;
using term_type = boost::spirit::x3::rule<TermClass, ast::Term>;
using atom_type = boost::spirit::x3::rule<AtomClass, ast::Atom>;
using literal_type = boost::spirit::x3::rule<LiteralClass, ast::Literal>;
using function_term_type = boost::spirit::x3::rule<FunctionTermClass, ast::FunctionTerm>;
using function_expression_type = boost::spirit::x3::rule<FunctionExpressionClass, ast::FunctionExpression>;
using condition_type = boost::spirit::x3::rule<ConditionClass, ast::Condition>;
using effect_type = boost::spirit::x3::rule<EffectClass, ast::Effect>;
using predicate_declaration_type = boost::spirit::x3::rule<PredicateDeclarationClass, ast::PredicateDeclaration>;
using function_declaration_type = boost::spirit::x3::rule<FunctionDeclarationClass, ast::FunctionDeclaration>;
using action_type = boost::spirit::x3::rule<ActionClass, ast::Action>;
using axiom_type = boost::spirit::x3::rule<AxiomClass, ast::Axiom>;
using metric_type = boost::spirit::x3::rule<MetricClass, ast::Metric>;
using initial_function_value_type = boost::spirit::x3::rule<InitialFunctionValueClass, ast::InitialFunctionValue>;
using domain_type = boost::spirit::x3::rule<DomainClass, ast::Domain>;
using task_type = boost::spirit::x3::rule<TaskClass, ast::Task>;
using file_type = boost::spirit::x3::rule<FileClass, ast::File>;

BOOST_SPIRIT_DECLARE(identifier_type)
BOOST_SPIRIT_DECLARE(type_expression_type)
BOOST_SPIRIT_DECLARE(typed_name_list_type)
BOOST_SPIRIT_DECLARE(typed_variable_list_type)
BOOST_SPIRIT_DECLARE(term_type)
BOOST_SPIRIT_DECLARE(atom_type)
BOOST_SPIRIT_DECLARE(literal_type)
BOOST_SPIRIT_DECLARE(function_term_type)
BOOST_SPIRIT_DECLARE(function_expression_type)
BOOST_SPIRIT_DECLARE(condition_type)
BOOST_SPIRIT_DECLARE(effect_type)
BOOST_SPIRIT_DECLARE(predicate_declaration_type)
BOOST_SPIRIT_DECLARE(function_declaration_type)
BOOST_SPIRIT_DECLARE(action_type)
BOOST_SPIRIT_DECLARE(axiom_type)
BOOST_SPIRIT_DECLARE(metric_type)
BOOST_SPIRIT_DECLARE(initial_function_value_type)
BOOST_SPIRIT_DECLARE(domain_type)
BOOST_SPIRIT_DECLARE(task_type)
BOOST_SPIRIT_DECLARE(file_type)
}

rules::identifier_type const& identifier();
rules::type_expression_type const& type_expression();
rules::typed_name_list_type const& typed_name_list();
rules::typed_variable_list_type const& typed_variable_list();
rules::term_type const& term();
rules::atom_type const& atom();
rules::literal_type const& literal();
rules::function_term_type const& function_term();
rules::function_expression_type const& function_expression();
rules::condition_type const& condition();
rules::effect_type const& effect();
rules::predicate_declaration_type const& predicate_declaration();
rules::function_declaration_type const& function_declaration();
rules::action_type const& action();
rules::axiom_type const& axiom();
rules::metric_type const& metric();
rules::initial_function_value_type const& initial_function_value();
rules::domain_type const& domain();
rules::task_type const& task();
rules::file_type const& file();

inline bool parse_domain(const std::string& source, ast::Domain& out, ErrorHandlerType& error_handler)
{ return ygg::diagnostics::parse_full(source, domain(), out, error_handler); }

inline bool parse_task(const std::string& source, ast::Task& out, ErrorHandlerType& error_handler)
{ return ygg::diagnostics::parse_full(source, task(), out, error_handler); }

inline bool parse_file(const std::string& source, ast::File& out, ErrorHandlerType& error_handler)
{ return ygg::diagnostics::parse_full(source, file(), out, error_handler); }

}

#endif
