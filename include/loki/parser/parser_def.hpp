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

// X3 grammars combine sequences (>>) with expectations (>) by design; clang's request for parentheses around
// every such sequence would not change any grammar.
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-shift-op-parentheses"
#endif

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

constexpr identifier_type identifier = "identifier";
constexpr type_expression_type type_expression = "type_expression";
constexpr typed_name_list_type typed_name_list = "typed_name_list";
constexpr typed_variable_list_type typed_variable_list = "typed_variable_list";
constexpr term_type term = "term";
constexpr atom_type atom = "atom";
constexpr literal_type literal = "literal";
constexpr function_term_type function_term = "function_term";
constexpr function_expression_type function_expression = "function_expression";
constexpr condition_type condition = "condition";
constexpr effect_type effect = "effect";
constexpr predicate_declaration_type predicate_declaration = "predicate_declaration";
constexpr function_declaration_type function_declaration = "function_declaration";
constexpr action_type action = "action";
constexpr axiom_type axiom = "axiom";
constexpr metric_type metric = "metric";
constexpr initial_function_value_type initial_function_value = "initial_function_value";
constexpr domain_type domain = "domain";
constexpr task_type task = "task";
constexpr file_type file = "file";

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

constexpr boost::spirit::x3::rule<VariableIdentifierClass, ast::Identifier> variable_identifier = "variable_identifier";
constexpr boost::spirit::x3::rule<TypeReferenceClass, ast::TypeReference> type_reference = "type_reference";
constexpr boost::spirit::x3::rule<EitherTypeClass, ast::EitherType> either_type = "either_type";
constexpr boost::spirit::x3::rule<FunctionExpressionNumberClass, ast::FunctionExpressionNumber> function_expression_number = "function_expression_number";
constexpr boost::spirit::x3::rule<FunctionExpressionFunctionClass, ast::FunctionExpressionFunction> function_expression_function =
    "function_expression_function";
constexpr boost::spirit::x3::rule<FunctionExpressionUnaryClass, ast::FunctionExpressionUnary> function_expression_unary = "function_expression_unary";
constexpr boost::spirit::x3::rule<FunctionExpressionBinaryClass, ast::FunctionExpressionBinary> function_expression_binary = "function_expression_binary";
constexpr boost::spirit::x3::rule<FunctionExpressionMultiClass, ast::FunctionExpressionMulti> function_expression_multi = "function_expression_multi";
constexpr boost::spirit::x3::rule<ConditionLiteralClass, ast::ConditionLiteral> condition_literal = "condition_literal";
constexpr boost::spirit::x3::rule<ConditionAndClass, ast::ConditionAnd> condition_and = "condition_and";
constexpr boost::spirit::x3::rule<ConditionOrClass, ast::ConditionOr> condition_or = "condition_or";
constexpr boost::spirit::x3::rule<ConditionNotClass, ast::ConditionNot> condition_not = "condition_not";
constexpr boost::spirit::x3::rule<ConditionImplyClass, ast::ConditionImply> condition_imply = "condition_imply";
constexpr boost::spirit::x3::rule<ConditionExistsClass, ast::ConditionExists> condition_exists = "condition_exists";
constexpr boost::spirit::x3::rule<ConditionForallClass, ast::ConditionForall> condition_forall = "condition_forall";
constexpr boost::spirit::x3::rule<ConditionNumericConstraintClass, ast::ConditionNumericConstraint> condition_numeric_constraint =
    "condition_numeric_constraint";
constexpr boost::spirit::x3::rule<EffectLiteralClass, ast::EffectLiteral> effect_literal = "effect_literal";
constexpr boost::spirit::x3::rule<EffectAndClass, ast::EffectAnd> effect_and = "effect_and";
constexpr boost::spirit::x3::rule<EffectNumericClass, ast::EffectNumeric> effect_numeric = "effect_numeric";
constexpr boost::spirit::x3::rule<EffectForallClass, ast::EffectForall> effect_forall = "effect_forall";
constexpr boost::spirit::x3::rule<EffectWhenClass, ast::EffectWhen> effect_when = "effect_when";
constexpr boost::spirit::x3::rule<EffectOneOfClass, ast::EffectOneOf> effect_one_of = "effect_one_of";
constexpr boost::spirit::x3::rule<EffectProbabilisticClass, ast::EffectProbabilistic> effect_probabilistic = "effect_probabilistic";
constexpr boost::spirit::x3::rule<ProbabilisticEffectAlternativeClass, ast::ProbabilisticEffectAlternative> probabilistic_effect_alternative =
    "probabilistic_effect_alternative";
constexpr boost::spirit::x3::rule<RequirementSectionClass, std::vector<ast::Requirement>> requirement_section = "requirement_section";
constexpr boost::spirit::x3::rule<TypeSectionClass, std::vector<ast::TypedGroup>> type_section = "type_section";
constexpr boost::spirit::x3::rule<ConstantSectionClass, std::vector<ast::TypedGroup>> constant_section = "constant_section";
constexpr boost::spirit::x3::rule<PredicateSectionClass, std::vector<ast::PredicateDeclaration>> predicate_section = "predicate_section";
constexpr boost::spirit::x3::rule<FunctionSectionClass, std::vector<ast::FunctionDeclaration>> function_section = "function_section";
constexpr boost::spirit::x3::rule<ObjectSectionClass, std::vector<ast::TypedGroup>> object_section = "object_section";
constexpr boost::spirit::x3::rule<InitialSectionClass, std::vector<ast::InitialElement>> initial_section = "init section";

inline constexpr auto symbol_character = char_ - '(' - ')' - ';' - boost::spirit::x3::ascii::space;
inline constexpr auto symbol = lexeme[+symbol_character];
inline constexpr auto keyword = [](char const* text)
{ return boost::spirit::x3::rule<class KeywordClass> { text } = lexeme[no_case[lit(text)] >> !symbol_character]; };
inline constexpr auto operator_symbol =
    lexeme[(string("scale-down") | string("scale-up") | string("increase") | string("decrease") | string("assign") | string("minus")) >> !symbol_character
           | (char_('+') | char_('-') | char_('*') | char_('/') | char_('=') | char_('<') | char_('>') | char_('!')) >> -char_('=')];
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

constexpr auto identifier_def = detail::symbol;
constexpr auto type_expression_def = detail::type_reference | detail::either_type;
struct TypedNameGroupClass : boost::spirit::x3::annotate_on_success
{
};
struct TypedVariableGroupClass : boost::spirit::x3::annotate_on_success
{
};
constexpr auto typed_name_group = boost::spirit::x3::rule<TypedNameGroupClass, ast::TypedGroup> { "typed name group" } = +(!lit('-') >> identifier)
                                                                                                                         >> -(lit('-') >> type_expression);
constexpr auto typed_variable_group = boost::spirit::x3::rule<TypedVariableGroupClass, ast::TypedGroup> {
    "typed variable group"
} = +(!lit('-') >> detail::variable_identifier) >> -(lit('-') >> type_expression);
constexpr auto typed_name_list_def = *typed_name_group;
constexpr auto typed_variable_list_def = *typed_variable_group;
constexpr auto term_def = (detail::variable_identifier >> attr(true)) | (identifier >> attr(false));
constexpr auto atom_def = context("atom")['(' >> identifier >> *term > ')'];
constexpr auto literal_def = (context("negated literal")['(' >> detail::keyword("not") > atom > ')'] >> attr(false)) | (atom >> attr(true));
constexpr auto function_term_def = context("function term")['(' >> identifier >> *term > ')'];
constexpr auto function_expression_def = detail::function_expression_number | detail::function_expression_binary | detail::function_expression_unary
                                         | detail::function_expression_multi | detail::function_expression_function;
constexpr auto condition_empty = boost::spirit::x3::rule<class ConditionEmptyClass, ast::Condition> { "condition_empty" } = lit('(') >> lit(')')
                                                                                                                            >> attr(ast::ConditionAnd {});
constexpr auto condition_def = condition_empty | detail::condition_and | detail::condition_or | detail::condition_not | detail::condition_imply
                               | detail::condition_exists | detail::condition_forall | detail::condition_numeric_constraint | detail::condition_literal;
constexpr auto effect_def = detail::effect_and | detail::effect_forall | detail::effect_when | detail::effect_one_of | detail::effect_probabilistic
                            | detail::effect_numeric | detail::effect_literal;
constexpr auto predicate_declaration_def = context("predicate declaration")['(' >> identifier >> typed_variable_list > ')'];
constexpr auto function_declaration_def = context("function declaration")['(' >> identifier >> typed_variable_list > ')'] >> -(lit('-') >> type_expression);
constexpr auto action_def = context(
    "action")[('(' >> detail::keyword(":action") > identifier > detail::keyword(":parameters") > context("action parameters")['(' >> typed_variable_list > ')'])
              >> -(detail::keyword(":precondition") > condition) >> -(detail::keyword(":effect") > effect) > ')'];
constexpr auto axiom_def = context("derived predicate")['(' >> detail::keyword(":derived") > predicate_declaration > condition > ')'];
constexpr auto metric_def = context("metric")['(' >> detail::keyword(":metric") > identifier > function_expression > ')'];
// '=' also introduces object equality; a function term distinguishes a numeric initial value.
constexpr auto initial_function_value_def = context("initial function value")['(' >> detail::keyword("=") >> function_term > function_expression > ')'];
constexpr auto domain_def = context(
    "domain definition")['(' >> detail::keyword("define") >> context("domain header")['(' >> detail::keyword("domain") > identifier > ')']
                         >> (detail::requirement_section | attr(std::vector<ast::Requirement> {}))
                         >> (detail::type_section | attr(std::vector<ast::TypedGroup> {})) >> (detail::constant_section | attr(std::vector<ast::TypedGroup> {}))
                         >> (detail::predicate_section | attr(std::vector<ast::PredicateDeclaration> {}))
                         >> (detail::function_section | attr(std::vector<ast::FunctionDeclaration> {})) >> *(axiom | action) > ')'];
constexpr auto task_def =
    context("problem definition")[('(' >> detail::keyword("define") >> context("problem header")['(' >> detail::keyword("problem") > identifier > ')']
                                   > context("problem domain")['(' >> detail::keyword(":domain") > identifier > ')'])
                                  >> (detail::requirement_section | attr(std::vector<ast::Requirement> {}))
                                  >> (detail::type_section | attr(std::vector<ast::TypedGroup> {}))
                                  >> (detail::predicate_section | attr(std::vector<ast::PredicateDeclaration> {}))
                                  >> (detail::object_section | attr(std::vector<ast::TypedGroup> {})) >> boost::spirit::x3::expect[detail::initial_section]
                                  >> -context("goal")['(' >> detail::keyword(":goal") > condition > ')'] >> -metric >> *axiom > ')'];
constexpr auto file_def = domain | task;

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
constexpr auto variable_identifier_def = boost::spirit::x3::raw['?' >> identifier];
constexpr auto type_reference_def = identifier;
constexpr auto type_expressions = boost::spirit::x3::rule<class TypeExpressionsClass, std::vector<ast::TypeExpression>> { "one or more type expressions" } =
    +type_expression;
constexpr auto either_type_def = context("either type")['(' >> keyword("either") > type_expressions > ')'];
constexpr auto function_expression_number_def = double_;
constexpr auto function_expression_function_def = function_term;
constexpr auto function_expression_unary_def =
    context("unary expression")['(' >> (lexeme[string("minus") >> !symbol_character] | string("-")) > function_expression > ')'];
// Binary '-' is tried first; only a parsed second operand distinguishes it from unary '-'.
constexpr auto function_expression_binary_def = context("binary expression")[('(' >> string("-") >> function_expression >> function_expression > ')')
                                                                             | ('(' >> string("/") > function_expression > function_expression > ')')];
constexpr auto function_expression_multi_def = context("arithmetic expression")['(' >> (string("+") | string("*")) >> *function_expression > ')'];
constexpr auto condition_literal_def = literal;
constexpr auto condition_and_def = context("and condition")['(' >> keyword("and") >> *condition > ')'];
constexpr auto condition_or_def = context("or condition")['(' >> keyword("or") >> *condition > ')'];
constexpr auto condition_not_def = context("not condition")['(' >> keyword("not") > condition > ')'];
constexpr auto condition_imply_def = context("imply condition")['(' >> keyword("imply") > condition > condition > ')'];
constexpr auto condition_exists_def =
    context("exists condition")['(' >> keyword("exists") > context("exists parameters")['(' >> typed_variable_list > ')'] > condition > ')'];
constexpr auto condition_forall_def =
    context("forall condition")['(' >> keyword("forall") > context("forall parameters")['(' >> typed_variable_list > ')'] > condition > ')'];
// A numeric first operand distinguishes numeric '=' from an ordinary equality literal.
constexpr auto condition_numeric_constraint_def = context("numeric constraint")['(' >> operator_symbol >> function_expression > function_expression > ')'];
constexpr auto effect_literal_def = literal;
constexpr auto effect_and_def = context("and effect")['(' >> keyword("and") >> *effect > ')'];
constexpr auto effect_numeric_def = context("numeric effect")['(' >> operator_symbol > function_term > function_expression > ')'];
constexpr auto effect_forall_def =
    context("forall effect")['(' >> keyword("forall") > context("forall parameters")['(' >> typed_variable_list > ')'] > effect > ')'];
constexpr auto effect_when_def = context("when effect")['(' >> keyword("when") > condition > effect > ')'];
constexpr auto effect_one_of_def = context("oneof effect")['(' >> keyword("oneof") >> *effect > ')'];
constexpr auto probabilistic_effect_alternative_def = double_ >> effect;
constexpr auto effect_probabilistic_def = context("probabilistic effect")['(' >> keyword("probabilistic") >> *probabilistic_effect_alternative > ')'];
struct RequirementNameClass : boost::spirit::x3::annotate_on_success
{
};
constexpr auto requirement_name = boost::spirit::x3::rule<RequirementNameClass, ast::Identifier> { "requirement name" } =
    +(symbol_character - ':') >> !symbol_character;
constexpr auto requirement_section_def = context("requirements section")['(' >> keyword(":requirements") >> *lexeme[lit(':') > requirement_name] > ')'];
constexpr auto type_section_def = context("types section")['(' >> keyword(":types") > typed_name_list > ')'];
constexpr auto constant_section_def = context("constants section")['(' >> keyword(":constants") > typed_name_list > ')'];
constexpr auto predicate_section_def = context("predicates section")['(' >> keyword(":predicates") >> *predicate_declaration > ')'];
constexpr auto function_section_def = context("functions section")['(' >> keyword(":functions") >> *function_declaration > ')'];
constexpr auto object_section_def = context("objects section")['(' >> keyword(":objects") > typed_name_list > ')'];
constexpr auto initial_section_def = context("init section")['(' >> keyword(":init") >> *((initial_function_value | literal)) > ')'];

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

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif
