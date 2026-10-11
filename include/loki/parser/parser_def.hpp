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

#ifndef LOKI_PARSER_PARSER_DEF_HPP_
#define LOKI_PARSER_PARSER_DEF_HPP_

#include "loki/parser/ast_adapted.hpp"
#include "loki/parser/parser.hpp"

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>
#include <string>
#include <vector>

namespace loki::parser::rules
{
using boost::spirit::x3::attr;
using boost::spirit::x3::char_;
using boost::spirit::x3::double_;
using boost::spirit::x3::lexeme;
using boost::spirit::x3::lit;
using boost::spirit::x3::no_case;
using boost::spirit::x3::string;
using ygg::diagnostics::context;

identifier_type const identifier = "identifier";
type_expression_type const type_expression = "type_expression";
typed_name_list_type const typed_name_list = "typed_name_list";
typed_variable_list_type const typed_variable_list = "typed_variable_list";
term_type const term = "term";
atom_type const atom = "atom";
literal_type const literal = "literal";
function_term_type const function_term = "function_term";
function_expression_type const function_expression = "function_expression";
condition_type const condition = "condition";
effect_type const effect = "effect";
predicate_declaration_type const predicate_declaration = "predicate_declaration";
function_declaration_type const function_declaration = "function_declaration";
action_type const action = "action";
axiom_type const axiom = "axiom";
metric_type const metric = "metric";
initial_function_value_type const initial_function_value = "initial_function_value";
domain_type const domain = "domain";
task_type const task = "task";
file_type const file = "file";

namespace detail
{
struct VariableIdentifierClass : ErrorHandlerBase
{
};
struct TypeReferenceClass : ErrorHandlerBase
{
};
struct EitherTypeClass : ErrorHandlerBase
{
};
struct FunctionExpressionNumberClass : ErrorHandlerBase
{
};
struct FunctionExpressionFunctionClass : ErrorHandlerBase
{
};
struct FunctionExpressionUnaryClass : ErrorHandlerBase
{
};
struct FunctionExpressionBinaryClass : ErrorHandlerBase
{
};
struct FunctionExpressionMultiClass : ErrorHandlerBase
{
};
struct ConditionLiteralClass : ErrorHandlerBase
{
};
struct ConditionAndClass : ErrorHandlerBase
{
};
struct ConditionOrClass : ErrorHandlerBase
{
};
struct ConditionNotClass : ErrorHandlerBase
{
};
struct ConditionImplyClass : ErrorHandlerBase
{
};
struct ConditionExistsClass : ErrorHandlerBase
{
};
struct ConditionForallClass : ErrorHandlerBase
{
};
struct ConditionNumericConstraintClass : ErrorHandlerBase
{
};
struct EffectLiteralClass : ErrorHandlerBase
{
};
struct EffectAndClass : ErrorHandlerBase
{
};
struct EffectNumericClass : ErrorHandlerBase
{
};
struct EffectForallClass : ErrorHandlerBase
{
};
struct EffectWhenClass : ErrorHandlerBase
{
};
struct EffectOneOfClass : ErrorHandlerBase
{
};
struct EffectProbabilisticClass : ErrorHandlerBase
{
};
struct ProbabilisticEffectAlternativeClass : ErrorHandlerBase
{
};
struct RequirementSectionClass : ErrorHandlerBase
{
};
struct TypeSectionClass : ErrorHandlerBase
{
};
struct ConstantSectionClass : ErrorHandlerBase
{
};
struct PredicateSectionClass : ErrorHandlerBase
{
};
struct FunctionSectionClass : ErrorHandlerBase
{
};
struct ObjectSectionClass : ErrorHandlerBase
{
};
struct InitialSectionClass : ErrorHandlerBase
{
};

boost::spirit::x3::rule<VariableIdentifierClass, ast::Identifier> const variable_identifier = "variable_identifier";
boost::spirit::x3::rule<TypeReferenceClass, ast::TypeReference> const type_reference = "type_reference";
boost::spirit::x3::rule<EitherTypeClass, ast::EitherType> const either_type = "either_type";
boost::spirit::x3::rule<FunctionExpressionNumberClass, ast::FunctionExpressionNumber> const function_expression_number = "function_expression_number";
boost::spirit::x3::rule<FunctionExpressionFunctionClass, ast::FunctionExpressionFunction> const function_expression_function = "function_expression_function";
boost::spirit::x3::rule<FunctionExpressionUnaryClass, ast::FunctionExpressionUnary> const function_expression_unary = "function_expression_unary";
boost::spirit::x3::rule<FunctionExpressionBinaryClass, ast::FunctionExpressionBinary> const function_expression_binary = "function_expression_binary";
boost::spirit::x3::rule<FunctionExpressionMultiClass, ast::FunctionExpressionMulti> const function_expression_multi = "function_expression_multi";
boost::spirit::x3::rule<ConditionLiteralClass, ast::ConditionLiteral> const condition_literal = "condition_literal";
boost::spirit::x3::rule<ConditionAndClass, ast::ConditionAnd> const condition_and = "condition_and";
boost::spirit::x3::rule<ConditionOrClass, ast::ConditionOr> const condition_or = "condition_or";
boost::spirit::x3::rule<ConditionNotClass, ast::ConditionNot> const condition_not = "condition_not";
boost::spirit::x3::rule<ConditionImplyClass, ast::ConditionImply> const condition_imply = "condition_imply";
boost::spirit::x3::rule<ConditionExistsClass, ast::ConditionExists> const condition_exists = "condition_exists";
boost::spirit::x3::rule<ConditionForallClass, ast::ConditionForall> const condition_forall = "condition_forall";
boost::spirit::x3::rule<ConditionNumericConstraintClass, ast::ConditionNumericConstraint> const condition_numeric_constraint = "condition_numeric_constraint";
boost::spirit::x3::rule<EffectLiteralClass, ast::EffectLiteral> const effect_literal = "effect_literal";
boost::spirit::x3::rule<EffectAndClass, ast::EffectAnd> const effect_and = "effect_and";
boost::spirit::x3::rule<EffectNumericClass, ast::EffectNumeric> const effect_numeric = "effect_numeric";
boost::spirit::x3::rule<EffectForallClass, ast::EffectForall> const effect_forall = "effect_forall";
boost::spirit::x3::rule<EffectWhenClass, ast::EffectWhen> const effect_when = "effect_when";
boost::spirit::x3::rule<EffectOneOfClass, ast::EffectOneOf> const effect_one_of = "effect_one_of";
boost::spirit::x3::rule<EffectProbabilisticClass, ast::EffectProbabilistic> const effect_probabilistic = "effect_probabilistic";
boost::spirit::x3::rule<ProbabilisticEffectAlternativeClass, ast::ProbabilisticEffectAlternative> const probabilistic_effect_alternative =
    "probabilistic_effect_alternative";
boost::spirit::x3::rule<RequirementSectionClass, std::vector<ast::Requirement>> const requirement_section = "requirement_section";
boost::spirit::x3::rule<TypeSectionClass, std::vector<ast::TypedGroup>> const type_section = "type_section";
boost::spirit::x3::rule<ConstantSectionClass, std::vector<ast::TypedGroup>> const constant_section = "constant_section";
boost::spirit::x3::rule<PredicateSectionClass, std::vector<ast::PredicateDeclaration>> const predicate_section = "predicate_section";
boost::spirit::x3::rule<FunctionSectionClass, std::vector<ast::FunctionDeclaration>> const function_section = "function_section";
boost::spirit::x3::rule<ObjectSectionClass, std::vector<ast::TypedGroup>> const object_section = "object_section";
boost::spirit::x3::rule<InitialSectionClass, std::vector<ast::InitialElement>> const initial_section = "init section";

inline const auto symbol_character = char_ - '(' - ')' - ';' - boost::spirit::x3::ascii::space;
inline const auto symbol = lexeme[+symbol_character];
inline const auto keyword = [](char const* text)
{ return boost::spirit::x3::rule<class KeywordClass> { text } = lexeme[no_case[lit(text)] >> !symbol_character]; };
inline const auto operator_symbol =
    lexeme[(string("scale-down") | string("scale-up") | string("increase") | string("decrease") | string("assign") | string("minus")) >> !symbol_character
           | char_("+-*/=<>!") >> -char_('=')];
}  // namespace detail

using detail::condition_and;
using detail::condition_exists;
using detail::condition_forall;
using detail::condition_imply;
using detail::condition_literal;
using detail::condition_not;
using detail::condition_numeric_constraint;
using detail::condition_or;
using detail::constant_section;
using detail::effect_and;
using detail::effect_forall;
using detail::effect_literal;
using detail::effect_numeric;
using detail::effect_one_of;
using detail::effect_probabilistic;
using detail::effect_when;
using detail::either_type;
using detail::function_expression_binary;
using detail::function_expression_function;
using detail::function_expression_multi;
using detail::function_expression_number;
using detail::function_expression_unary;
using detail::function_section;
using detail::initial_section;
using detail::object_section;
using detail::predicate_section;
using detail::probabilistic_effect_alternative;
using detail::requirement_section;
using detail::type_reference;
using detail::type_section;
using detail::variable_identifier;

auto const identifier_def = detail::symbol;
auto const type_expression_def = detail::type_reference | detail::either_type;
struct TypedNameGroupClass : boost::spirit::x3::annotate_on_success
{
};
struct TypedVariableGroupClass : boost::spirit::x3::annotate_on_success
{
};
auto const typed_name_group = boost::spirit::x3::rule<TypedNameGroupClass, ast::TypedGroup> { "typed name group" } = +(!lit('-') >> identifier)
                                                                                                                     >> -(lit('-') >> type_expression);
auto const typed_variable_group = boost::spirit::x3::rule<TypedVariableGroupClass, ast::TypedGroup> {
    "typed variable group"
} = +(!lit('-') >> detail::variable_identifier) >> -(lit('-') >> type_expression);
auto const typed_name_list_def = *typed_name_group;
auto const typed_variable_list_def = *typed_variable_group;
auto const term_def = (detail::variable_identifier >> attr(true)) | (identifier >> attr(false));
auto const atom_def = context("atom")['(' >> identifier >> *term > ')'];
auto const literal_def = (context("negated literal")['(' >> detail::keyword("not") > atom > ')'] >> attr(false)) | (atom >> attr(true));
auto const function_term_def = context("function term")['(' >> identifier >> *term > ')'];
auto const function_expression_def = detail::function_expression_number | detail::function_expression_binary | detail::function_expression_unary
                                     | detail::function_expression_multi | detail::function_expression_function;
auto const condition_empty = boost::spirit::x3::rule<class ConditionEmptyClass, ast::Condition> { "condition_empty" } = lit('(') >> lit(')')
                                                                                                                        >> attr(ast::ConditionAnd {});
auto const condition_def = condition_empty | detail::condition_and | detail::condition_or | detail::condition_not | detail::condition_imply
                           | detail::condition_exists | detail::condition_forall | detail::condition_numeric_constraint | detail::condition_literal;
auto const effect_def = detail::effect_and | detail::effect_forall | detail::effect_when | detail::effect_one_of | detail::effect_probabilistic
                        | detail::effect_numeric | detail::effect_literal;
auto const predicate_declaration_def = context("predicate declaration")['(' >> identifier >> typed_variable_list > ')'];
auto const function_declaration_def = context("function declaration")['(' >> identifier >> typed_variable_list > ')'] >> -(lit('-') >> type_expression);
auto const action_def = context(
    "action")[('(' >> detail::keyword(":action") > identifier > detail::keyword(":parameters") > context("action parameters")['(' >> typed_variable_list > ')'])
              >> -(detail::keyword(":precondition") > condition) >> -(detail::keyword(":effect") > effect) > ')'];
auto const axiom_def = context("derived predicate")['(' >> detail::keyword(":derived") > predicate_declaration > condition > ')'];
auto const metric_def = context("metric")['(' >> detail::keyword(":metric") > identifier > function_expression > ')'];
// '=' also introduces object equality; a function term distinguishes a numeric initial value.
auto const initial_function_value_def = context("initial function value")['(' >> detail::keyword("=") >> function_term > function_expression > ')'];
auto const domain_def = context(
    "domain definition")['(' >> detail::keyword("define") >> context("domain header")['(' >> detail::keyword("domain") > identifier > ')']
                         >> (detail::requirement_section | attr(std::vector<ast::Requirement> {}))
                         >> (detail::type_section | attr(std::vector<ast::TypedGroup> {})) >> (detail::constant_section | attr(std::vector<ast::TypedGroup> {}))
                         >> (detail::predicate_section | attr(std::vector<ast::PredicateDeclaration> {}))
                         >> (detail::function_section | attr(std::vector<ast::FunctionDeclaration> {})) >> *(axiom | action) > ')'];
auto const task_def =
    context("problem definition")[('(' >> detail::keyword("define") >> context("problem header")['(' >> detail::keyword("problem") > identifier > ')']
                                   > context("problem domain")['(' >> detail::keyword(":domain") > identifier > ')'])
                                  >> (detail::requirement_section | attr(std::vector<ast::Requirement> {}))
                                  >> (detail::object_section | attr(std::vector<ast::TypedGroup> {})) >> boost::spirit::x3::expect[detail::initial_section]
                                  >> -context("goal")['(' >> detail::keyword(":goal") > condition > ')'] >> -metric >> *axiom > ')'];
auto const file_def = domain | task;

BOOST_SPIRIT_DEFINE(identifier,
                    type_expression,
                    typed_name_list,
                    typed_variable_list,
                    term,
                    atom,
                    literal,
                    function_term,
                    function_expression,
                    condition,
                    effect,
                    predicate_declaration,
                    function_declaration,
                    action,
                    axiom,
                    metric,
                    initial_function_value,
                    domain,
                    task,
                    file)

namespace detail
{
auto const variable_identifier_def = boost::spirit::x3::raw['?' >> identifier];
auto const type_reference_def = identifier;
auto const type_expressions = boost::spirit::x3::rule<class TypeExpressionsClass, std::vector<ast::TypeExpression>> { "one or more type expressions" } =
    +type_expression;
auto const either_type_def = context("either type")['(' >> keyword("either") > type_expressions > ')'];
auto const function_expression_number_def = double_;
auto const function_expression_function_def = function_term;
auto const function_expression_unary_def =
    context("unary expression")['(' >> (lexeme[string("minus") >> !symbol_character] | string("-")) > function_expression > ')'];
// Binary '-' is tried first; only a parsed second operand distinguishes it from unary '-'.
auto const function_expression_binary_def = context("binary expression")[('(' >> string("-") >> function_expression >> function_expression > ')')
                                                                         | ('(' >> string("/") > function_expression > function_expression > ')')];
auto const function_expression_multi_def = context("arithmetic expression")['(' >> (string("+") | string("*")) >> *function_expression > ')'];
auto const condition_literal_def = literal;
auto const condition_and_def = context("and condition")['(' >> keyword("and") >> *condition > ')'];
auto const condition_or_def = context("or condition")['(' >> keyword("or") >> *condition > ')'];
auto const condition_not_def = context("not condition")['(' >> keyword("not") > condition > ')'];
auto const condition_imply_def = context("imply condition")['(' >> keyword("imply") > condition > condition > ')'];
auto const condition_exists_def =
    context("exists condition")['(' >> keyword("exists") > context("exists parameters")['(' >> typed_variable_list > ')'] > condition > ')'];
auto const condition_forall_def =
    context("forall condition")['(' >> keyword("forall") > context("forall parameters")['(' >> typed_variable_list > ')'] > condition > ')'];
// A numeric first operand distinguishes numeric '=' from an ordinary equality literal.
auto const condition_numeric_constraint_def = context("numeric constraint")['(' >> operator_symbol >> function_expression > function_expression > ')'];
auto const effect_literal_def = literal;
auto const effect_and_def = context("and effect")['(' >> keyword("and") >> *effect > ')'];
auto const effect_numeric_def = context("numeric effect")['(' >> operator_symbol > function_term > function_expression > ')'];
auto const effect_forall_def =
    context("forall effect")['(' >> keyword("forall") > context("forall parameters")['(' >> typed_variable_list > ')'] > effect > ')'];
auto const effect_when_def = context("when effect")['(' >> keyword("when") > condition > effect > ')'];
auto const effect_one_of_def = context("oneof effect")['(' >> keyword("oneof") >> *effect > ')'];
auto const probabilistic_effect_alternative_def = double_ >> effect;
auto const effect_probabilistic_def = context("probabilistic effect")['(' >> keyword("probabilistic") >> *probabilistic_effect_alternative > ')'];
struct RequirementNameClass : boost::spirit::x3::annotate_on_success
{
};
auto const requirement_name = boost::spirit::x3::rule<RequirementNameClass, ast::Identifier> { "requirement name" } =
    +(symbol_character - ':') >> !symbol_character;
auto const requirement_section_def = context("requirements section")['(' >> keyword(":requirements") >> *lexeme[lit(':') > requirement_name] > ')'];
auto const type_section_def = context("types section")['(' >> keyword(":types") > typed_name_list > ')'];
auto const constant_section_def = context("constants section")['(' >> keyword(":constants") > typed_name_list > ')'];
auto const predicate_section_def = context("predicates section")['(' >> keyword(":predicates") >> *predicate_declaration > ')'];
auto const function_section_def = context("functions section")['(' >> keyword(":functions") >> *function_declaration > ')'];
auto const object_section_def = context("objects section")['(' >> keyword(":objects") > typed_name_list > ')'];
auto const initial_section_def = context("init section")['(' >> keyword(":init") >> *((initial_function_value | literal)) > ')'];

BOOST_SPIRIT_DEFINE(variable_identifier,
                    type_reference,
                    either_type,
                    function_expression_number,
                    function_expression_function,
                    function_expression_unary,
                    function_expression_binary,
                    function_expression_multi,
                    condition_literal,
                    condition_and,
                    condition_or,
                    condition_not,
                    condition_imply,
                    condition_exists,
                    condition_forall,
                    condition_numeric_constraint,
                    effect_literal,
                    effect_and,
                    effect_numeric,
                    effect_forall,
                    effect_when,
                    effect_one_of,
                    effect_probabilistic,
                    probabilistic_effect_alternative,
                    requirement_section,
                    type_section,
                    constant_section,
                    predicate_section,
                    function_section,
                    object_section,
                    initial_section)
}  // namespace detail
}  // namespace loki::parser::rules

#endif
