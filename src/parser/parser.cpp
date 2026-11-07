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

#include "loki/parser/parser_def.hpp"

namespace loki::parser::rules
{

BOOST_SPIRIT_INSTANTIATE(identifier_type, Iterator, Context)
BOOST_SPIRIT_INSTANTIATE(type_expression_type, Iterator, Context)
BOOST_SPIRIT_INSTANTIATE(typed_name_list_type, Iterator, Context)
BOOST_SPIRIT_INSTANTIATE(typed_variable_list_type, Iterator, Context)
BOOST_SPIRIT_INSTANTIATE(term_type, Iterator, Context)
BOOST_SPIRIT_INSTANTIATE(atom_type, Iterator, Context)
BOOST_SPIRIT_INSTANTIATE(literal_type, Iterator, Context)
BOOST_SPIRIT_INSTANTIATE(function_term_type, Iterator, Context)
BOOST_SPIRIT_INSTANTIATE(function_expression_type, Iterator, Context)
BOOST_SPIRIT_INSTANTIATE(condition_type, Iterator, Context)
BOOST_SPIRIT_INSTANTIATE(effect_type, Iterator, Context)
BOOST_SPIRIT_INSTANTIATE(predicate_declaration_type, Iterator, Context)
BOOST_SPIRIT_INSTANTIATE(function_declaration_type, Iterator, Context)
BOOST_SPIRIT_INSTANTIATE(action_type, Iterator, Context)
BOOST_SPIRIT_INSTANTIATE(axiom_type, Iterator, Context)
BOOST_SPIRIT_INSTANTIATE(metric_type, Iterator, Context)
BOOST_SPIRIT_INSTANTIATE(initial_function_value_type, Iterator, Context)
BOOST_SPIRIT_INSTANTIATE(domain_type, Iterator, Context)
BOOST_SPIRIT_INSTANTIATE(task_type, Iterator, Context)
BOOST_SPIRIT_INSTANTIATE(file_type, Iterator, Context)

}  // namespace loki::parser::rules

namespace loki::parser
{

rules::identifier_type const& identifier() { return rules::identifier; }
rules::type_expression_type const& type_expression() { return rules::type_expression; }
rules::typed_name_list_type const& typed_name_list() { return rules::typed_name_list; }
rules::typed_variable_list_type const& typed_variable_list() { return rules::typed_variable_list; }
rules::term_type const& term() { return rules::term; }
rules::atom_type const& atom() { return rules::atom; }
rules::literal_type const& literal() { return rules::literal; }
rules::function_term_type const& function_term() { return rules::function_term; }
rules::function_expression_type const& function_expression() { return rules::function_expression; }
rules::condition_type const& condition() { return rules::condition; }
rules::effect_type const& effect() { return rules::effect; }
rules::predicate_declaration_type const& predicate_declaration() { return rules::predicate_declaration; }
rules::function_declaration_type const& function_declaration() { return rules::function_declaration; }
rules::action_type const& action() { return rules::action; }
rules::axiom_type const& axiom() { return rules::axiom; }
rules::metric_type const& metric() { return rules::metric; }
rules::initial_function_value_type const& initial_function_value() { return rules::initial_function_value; }
rules::domain_type const& domain() { return rules::domain; }
rules::task_type const& task() { return rules::task; }
rules::file_type const& file() { return rules::file; }

}  // namespace loki::parser
