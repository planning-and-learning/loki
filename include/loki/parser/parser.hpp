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
#include "loki/parser/error_handler.hpp"

#include <boost/spirit/home/x3.hpp>

namespace loki::parser
{
namespace rules
{
struct IdentifierClass : ErrorHandlerBase
{
};
struct TypeExpressionClass : ErrorHandlerBase
{
};
struct TypedNameListClass : ErrorHandlerBase
{
};
struct TypedVariableListClass : ErrorHandlerBase
{
};
struct TermClass : ErrorHandlerBase
{
};
struct AtomClass : ErrorHandlerBase
{
};
struct LiteralClass : ErrorHandlerBase
{
};
struct FunctionTermClass : ErrorHandlerBase
{
};
struct FunctionExpressionClass : ErrorHandlerBase
{
};
struct ConditionClass : ErrorHandlerBase
{
};
struct EffectClass : ErrorHandlerBase
{
};
struct PredicateDeclarationClass : ErrorHandlerBase
{
};
struct FunctionDeclarationClass : ErrorHandlerBase
{
};
struct ActionClass : ErrorHandlerBase
{
};
struct AxiomClass : ErrorHandlerBase
{
};
struct MetricClass : ErrorHandlerBase
{
};
struct InitialFunctionValueClass : ErrorHandlerBase
{
};
struct DomainClass : ErrorHandlerBase
{
};
struct TaskClass : ErrorHandlerBase
{
};
struct FileClass : ErrorHandlerBase
{
};

using identifier_type = boost::spirit::x3::rule<IdentifierClass, ast::Identifier>;
using type_expression_type = boost::spirit::x3::rule<TypeExpressionClass, ast::TypeExpression>;
using typed_name_list_type = boost::spirit::x3::rule<TypedNameListClass, std::vector<ast::TypedGroup>>;
using typed_variable_list_type = boost::spirit::x3::rule<TypedVariableListClass, std::vector<ast::TypedGroup>>;
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

template<typename Iterator, typename Parser, typename Node>
bool parse_ast(Iterator& first, Iterator last, const Parser& parser, Node& out, ErrorHandler<Iterator>& error_handler)
{
    error_handler.clear_error();
    out = Node {};
    auto wrapped = boost::spirit::x3::with<ErrorHandlerTag>(std::ref(error_handler))[parser];
    return phrase_parse(first, last, wrapped, ygg::diagnostics::comment_skipper(), out) && !error_handler.last_error();
}

template<typename Parser, typename Node>
bool parse_ast(const std::string& source, const Parser& parser, Node& out, ErrorHandlerType& error_handler)
{
    auto first = source.begin();
    return parse_ast(first, source.end(), parser, out, error_handler);
}

template<typename Iterator, typename Parser, typename Node>
bool parse_full(Iterator& first, Iterator last, const Parser& parser, Node& out, ErrorHandler<Iterator>& error_handler)
{
    const auto success = parse_ast(first, last, parser, out, error_handler);
    if (success && first != last)
        error_handler.record_error(first, "Unexpected trailing input.");
    return success && first == last;
}

template<typename Parser, typename Node>
bool parse_full(const std::string& source, const Parser& parser, Node& out, ErrorHandlerType& error_handler)
{
    auto first = source.begin();
    return parse_full(first, source.end(), parser, out, error_handler);
}

inline bool parse_domain(const std::string& source, ast::Domain& out, ErrorHandlerType& error_handler)
{ return parse_full(source, domain(), out, error_handler); }

inline bool parse_task(const std::string& source, ast::Task& out, ErrorHandlerType& error_handler) { return parse_full(source, task(), out, error_handler); }

inline bool parse_file(const std::string& source, ast::File& out, ErrorHandlerType& error_handler) { return parse_full(source, file(), out, error_handler); }

}

#endif
