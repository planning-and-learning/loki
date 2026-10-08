/*
 * Copyright (C) 2026 Dominik Drexler
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

#ifndef LOKI_SEMANTIC_TRANSLATOR_NORMALIZE_ARITHMETIC_EXPRESSIONS_TRANSLATOR_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_NORMALIZE_ARITHMETIC_EXPRESSIONS_TRANSLATOR_HPP_

#include "loki/semantic/translator/copy_translator_component.hpp"

#include <array>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace loki::semantic::detail
{

template<typename Derived>
class NormalizeArithmeticExpressionsTranslator : public CopyTranslatorComponent<Derived, NormalizeArithmeticExpressionsTranslator<Derived>>
{
public:
    explicit NormalizeArithmeticExpressionsTranslator(CopyContext& context) :
        CopyTranslatorComponent<Derived, NormalizeArithmeticExpressionsTranslator<Derived>>(context)
    {
    }

    formalism::FunctionExpressionView normalize_arithmetic_expression(formalism::FunctionExpressionView source);

private:
    template<typename T>
    formalism::FunctionExpressionView wrap(T value);

    formalism::FunctionExpressionView normalize_node(formalism::FunctionExpressionNumberView source);
    formalism::FunctionExpressionView normalize_node(formalism::FunctionTermView source);
    formalism::FunctionExpressionView normalize_node(formalism::UnaryFunctionExpressionView source);
    formalism::FunctionExpressionView normalize_node(formalism::BinaryFunctionExpressionView source);
    formalism::FunctionExpressionView normalize_node(formalism::MultiFunctionExpressionView source);

    formalism::FunctionExpressionView make_number(double value);
    template<typename Range>
    formalism::FunctionExpressionView normalize_associative(formalism::MultiArithmeticOperator op, const Range& source_operands);
    void append_source_operand(formalism::MultiArithmeticOperator op,
                               formalism::FunctionExpressionView source,
                               ygg::IndexList<formalism::FunctionExpression>& operands,
                               std::optional<formalism::FunctionExpressionView>& first_operand);
    void append_normalized_operand(formalism::MultiArithmeticOperator op,
                                   formalism::FunctionExpressionView source,
                                   ygg::IndexList<formalism::FunctionExpression>& operands,
                                   std::optional<formalism::FunctionExpressionView>& first_operand);
    bool is_unit(formalism::MultiArithmeticOperator op, formalism::FunctionExpressionView expression) const;
};

template<typename Derived>
template<typename T>
formalism::FunctionExpressionView NormalizeArithmeticExpressionsTranslator<Derived>::wrap(T value)
{
    auto data = formalism::checkout<formalism::FunctionExpression>(this->m_context.builder);
    data->variant = ygg::Data<formalism::FunctionExpression>::Variant(value.get_index());
    return formalism::insert(this->m_context.storage->repository, *data).first;
}

template<typename Derived>
formalism::FunctionExpressionView NormalizeArithmeticExpressionsTranslator<Derived>::normalize_arithmetic_expression(formalism::FunctionExpressionView source)
{
    if (const auto mapped = find_mapped(this->m_context.storage->function_expressions, source))
        return *mapped;
    const auto normalized = ygg::visit([this](const auto& node) { return this->normalize_node(node); }, source.get_variant());
    remember(this->m_context.storage->function_expressions, source, normalized);
    return normalized;
}

template<typename Derived>
formalism::FunctionExpressionView NormalizeArithmeticExpressionsTranslator<Derived>::normalize_node(formalism::FunctionExpressionNumberView source)
{
    return wrap(this->self().copy(source));
}

template<typename Derived>
formalism::FunctionExpressionView NormalizeArithmeticExpressionsTranslator<Derived>::normalize_node(formalism::FunctionTermView source)
{
    return wrap(this->self().copy(source));
}

template<typename Derived>
formalism::FunctionExpressionView NormalizeArithmeticExpressionsTranslator<Derived>::normalize_node(formalism::UnaryFunctionExpressionView source)
{
    const auto expression = normalize_arithmetic_expression(source.get_expression());
    auto data = formalism::checkout<formalism::UnaryFunctionExpression>(this->m_context.builder);
    data->op = source.get_operator();
    data->expression = expression.get_index();
    return wrap(formalism::insert(this->m_context.storage->repository, *data).first);
}

template<typename Derived>
formalism::FunctionExpressionView NormalizeArithmeticExpressionsTranslator<Derived>::normalize_node(formalism::BinaryFunctionExpressionView source)
{
    switch (source.get_operator())
    {
        case formalism::BinaryArithmeticOperator::Add:
            return normalize_associative(formalism::MultiArithmeticOperator::Add, std::array { source.get_left(), source.get_right() });
        case formalism::BinaryArithmeticOperator::Mul:
            return normalize_associative(formalism::MultiArithmeticOperator::Mul, std::array { source.get_left(), source.get_right() });
        case formalism::BinaryArithmeticOperator::Sub:
        case formalism::BinaryArithmeticOperator::Div:
        {
            const auto left = normalize_arithmetic_expression(source.get_left());
            const auto right = normalize_arithmetic_expression(source.get_right());
            auto data = formalism::checkout<formalism::BinaryFunctionExpression>(this->m_context.builder);
            data->op = source.get_operator();
            data->left = left.get_index();
            data->right = right.get_index();
            return wrap(formalism::insert(this->m_context.storage->repository, *data).first);
        }
    }
    throw std::invalid_argument("invalid BinaryArithmeticOperator");
}

template<typename Derived>
formalism::FunctionExpressionView NormalizeArithmeticExpressionsTranslator<Derived>::normalize_node(formalism::MultiFunctionExpressionView source)
{
    return normalize_associative(source.get_operator(), source.get_args());
}

template<typename Derived>
formalism::FunctionExpressionView NormalizeArithmeticExpressionsTranslator<Derived>::make_number(double value)
{
    auto data = formalism::checkout<formalism::FunctionExpressionNumber>(this->m_context.builder);
    data->value = value;
    return wrap(formalism::insert(this->m_context.storage->repository, *data).first);
}

template<typename Derived>
template<typename Range>
formalism::FunctionExpressionView NormalizeArithmeticExpressionsTranslator<Derived>::normalize_associative(formalism::MultiArithmeticOperator op,
                                                                                                           const Range& source_operands)
{
    auto data = formalism::checkout<formalism::MultiFunctionExpression>(this->m_context.builder);
    auto first_operand = std::optional<formalism::FunctionExpressionView> {};
    for (const auto operand : source_operands)
        append_source_operand(op, operand, data->args, first_operand);

    if (data->args.empty())
        return make_number(op == formalism::MultiArithmeticOperator::Add ? 0.0 : 1.0);
    if (data->args.size() == 1)
        return first_operand.value();

    data->op = op;
    return wrap(formalism::insert(this->m_context.storage->repository, *data).first);
}

template<typename Derived>
void NormalizeArithmeticExpressionsTranslator<Derived>::append_source_operand(formalism::MultiArithmeticOperator op,
                                                                              formalism::FunctionExpressionView source,
                                                                              ygg::IndexList<formalism::FunctionExpression>& operands,
                                                                              std::optional<formalism::FunctionExpressionView>& first_operand)
{
    const auto variant = source.get_variant();
    if (variant.template is<ygg::Index<formalism::BinaryFunctionExpression>>())
    {
        const auto binary = variant.template get<ygg::Index<formalism::BinaryFunctionExpression>>();
        if ((op == formalism::MultiArithmeticOperator::Add && binary.get_operator() == formalism::BinaryArithmeticOperator::Add)
            || (op == formalism::MultiArithmeticOperator::Mul && binary.get_operator() == formalism::BinaryArithmeticOperator::Mul))
        {
            append_source_operand(op, binary.get_left(), operands, first_operand);
            append_source_operand(op, binary.get_right(), operands, first_operand);
            return;
        }
    }
    else if (variant.template is<ygg::Index<formalism::MultiFunctionExpression>>())
    {
        const auto multi = variant.template get<ygg::Index<formalism::MultiFunctionExpression>>();
        if (multi.get_operator() == op)
        {
            for (const auto expression : multi.get_args())
                append_source_operand(op, expression, operands, first_operand);
            return;
        }
    }
    append_normalized_operand(op, source, operands, first_operand);
}

template<typename Derived>
void NormalizeArithmeticExpressionsTranslator<Derived>::append_normalized_operand(formalism::MultiArithmeticOperator op,
                                                                                  formalism::FunctionExpressionView source,
                                                                                  ygg::IndexList<formalism::FunctionExpression>& operands,
                                                                                  std::optional<formalism::FunctionExpressionView>& first_operand)
{
    const auto normalized = normalize_arithmetic_expression(source);
    const auto variant = normalized.get_variant();
    if (variant.template is<ygg::Index<formalism::MultiFunctionExpression>>())
    {
        const auto multi = variant.template get<ygg::Index<formalism::MultiFunctionExpression>>();
        if (multi.get_operator() == op)
        {
            for (const auto expression : multi.get_args())
            {
                if (operands.empty())
                    first_operand = expression;
                operands.push_back(expression.get_index());
            }
            return;
        }
    }
    if (!is_unit(op, normalized))
    {
        if (operands.empty())
            first_operand = normalized;
        operands.push_back(normalized.get_index());
    }
}

template<typename Derived>
bool NormalizeArithmeticExpressionsTranslator<Derived>::is_unit(formalism::MultiArithmeticOperator op, formalism::FunctionExpressionView expression) const
{
    const auto variant = expression.get_variant();
    return variant.template is<ygg::Index<formalism::FunctionExpressionNumber>>()
           && variant.template get<ygg::Index<formalism::FunctionExpressionNumber>>().get_value()
                  == (op == formalism::MultiArithmeticOperator::Add ? 0.0 : 1.0);
}

}  // namespace loki::semantic::detail

#endif
