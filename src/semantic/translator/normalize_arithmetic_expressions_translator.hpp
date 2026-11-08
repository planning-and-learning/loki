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

#include "copy_translator_component.hpp"

#include <array>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <urd/config.hpp>

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

    formalism::FunctionExpressionView normalize_node(formalism::EntityView<formalism::FunctionExpression<formalism::NumberTag>> source);
    formalism::FunctionExpressionView normalize_node(formalism::FunctionTermView source);
    formalism::FunctionExpressionView normalize_node(formalism::EntityView<formalism::FunctionExpression<formalism::UnaryTag>> source);
    formalism::FunctionExpressionView normalize_node(formalism::EntityView<formalism::FunctionExpression<formalism::BinaryTag>> source);
    formalism::FunctionExpressionView normalize_node(formalism::EntityView<formalism::FunctionExpression<formalism::MultiTag>> source);

    formalism::FunctionExpressionView make_number(double value);
    template<typename Range>
    formalism::FunctionExpressionView normalize_associative(formalism::MultiArithmeticOperator op, const Range& source_operands);
    void append_source_operand(formalism::MultiArithmeticOperator op,
                               formalism::FunctionExpressionView source,
                               formalism::FunctionExpressionList& operands,
                               std::optional<formalism::FunctionExpressionView>& first_operand);
    void append_normalized_operand(formalism::MultiArithmeticOperator op,
                                   formalism::FunctionExpressionView source,
                                   formalism::FunctionExpressionList& operands,
                                   std::optional<formalism::FunctionExpressionView>& first_operand);
    [[nodiscard]] bool is_unit(formalism::MultiArithmeticOperator op, formalism::FunctionExpressionView expression) const;
};

template<typename Derived>
formalism::FunctionExpressionView NormalizeArithmeticExpressionsTranslator<Derived>::normalize_arithmetic_expression(formalism::FunctionExpressionView source)
{
    return std::visit([this](const auto& node) { return this->normalize_node(node); }, source);
}

template<typename Derived>
formalism::FunctionExpressionView
NormalizeArithmeticExpressionsTranslator<Derived>::normalize_node(formalism::EntityView<formalism::FunctionExpression<formalism::NumberTag>> source)
{ return formalism::FunctionExpressionView(this->self().copy(source)); }

template<typename Derived>
formalism::FunctionExpressionView NormalizeArithmeticExpressionsTranslator<Derived>::normalize_node(formalism::FunctionTermView source)
{ return formalism::FunctionExpressionView(this->self().copy(source)); }

template<typename Derived>
formalism::FunctionExpressionView
NormalizeArithmeticExpressionsTranslator<Derived>::normalize_node(formalism::EntityView<formalism::FunctionExpression<formalism::UnaryTag>> source)
{
    const auto expression = normalize_arithmetic_expression(source.get_expression());
    auto data = formalism::checkout<formalism::FunctionExpression<formalism::UnaryTag>>(this->copy_context().builder);
    data->op = source.get_operator();
    data->expression = urd::to_stored(expression);
    return formalism::FunctionExpressionView(formalism::insert(this->copy_context().storage->repository, *data).first);
}

template<typename Derived>
formalism::FunctionExpressionView
NormalizeArithmeticExpressionsTranslator<Derived>::normalize_node(formalism::EntityView<formalism::FunctionExpression<formalism::BinaryTag>> source)
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
            auto data = formalism::checkout<formalism::FunctionExpression<formalism::BinaryTag>>(this->copy_context().builder);
            data->op = source.get_operator();
            data->left = urd::to_stored(left);
            data->right = urd::to_stored(right);
            return formalism::FunctionExpressionView(formalism::insert(this->copy_context().storage->repository, *data).first);
        }
    }
    throw std::invalid_argument("invalid BinaryArithmeticOperator");
}

template<typename Derived>
formalism::FunctionExpressionView
NormalizeArithmeticExpressionsTranslator<Derived>::normalize_node(formalism::EntityView<formalism::FunctionExpression<formalism::MultiTag>> source)
{ return normalize_associative(source.get_operator(), source.get_args()); }

template<typename Derived>
formalism::FunctionExpressionView NormalizeArithmeticExpressionsTranslator<Derived>::make_number(double value)
{
    auto data = formalism::checkout<formalism::FunctionExpression<formalism::NumberTag>>(this->copy_context().builder);
    data->value = value;
    return formalism::FunctionExpressionView(formalism::insert(this->copy_context().storage->repository, *data).first);
}

template<typename Derived>
template<typename Range>
formalism::FunctionExpressionView NormalizeArithmeticExpressionsTranslator<Derived>::normalize_associative(formalism::MultiArithmeticOperator op,
                                                                                                           const Range& source_operands)
{
    auto data = formalism::checkout<formalism::FunctionExpression<formalism::MultiTag>>(this->copy_context().builder);
    auto first_operand = std::optional<formalism::FunctionExpressionView> {};
    for (const auto operand : source_operands)
        append_source_operand(op, operand, data->args, first_operand);

    if (data->args.empty())
        return make_number(op == formalism::MultiArithmeticOperator::Add ? 0.0 : 1.0);
    if (data->args.size() == 1)
    {
        URD_ASSUME(first_operand.has_value());  // the one operand appended is the first
        return *first_operand;
    }

    data->op = op;
    return formalism::FunctionExpressionView(formalism::insert(this->copy_context().storage->repository, *data).first);
}

template<typename Derived>
void NormalizeArithmeticExpressionsTranslator<Derived>::append_source_operand(formalism::MultiArithmeticOperator op,
                                                                              formalism::FunctionExpressionView source,
                                                                              formalism::FunctionExpressionList& operands,
                                                                              std::optional<formalism::FunctionExpressionView>& first_operand)
{
    if (std::holds_alternative<formalism::EntityView<formalism::FunctionExpression<formalism::BinaryTag>>>(source))
    {
        const auto binary = std::get<formalism::EntityView<formalism::FunctionExpression<formalism::BinaryTag>>>(source);
        if ((op == formalism::MultiArithmeticOperator::Add && binary.get_operator() == formalism::BinaryArithmeticOperator::Add)
            || (op == formalism::MultiArithmeticOperator::Mul && binary.get_operator() == formalism::BinaryArithmeticOperator::Mul))
        {
            append_source_operand(op, binary.get_left(), operands, first_operand);
            append_source_operand(op, binary.get_right(), operands, first_operand);
            return;
        }
    }
    else if (std::holds_alternative<formalism::EntityView<formalism::FunctionExpression<formalism::MultiTag>>>(source))
    {
        const auto multi = std::get<formalism::EntityView<formalism::FunctionExpression<formalism::MultiTag>>>(source);
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
                                                                                  formalism::FunctionExpressionList& operands,
                                                                                  std::optional<formalism::FunctionExpressionView>& first_operand)
{
    const auto normalized = normalize_arithmetic_expression(source);
    if (std::holds_alternative<formalism::EntityView<formalism::FunctionExpression<formalism::MultiTag>>>(normalized))
    {
        const auto multi = std::get<formalism::EntityView<formalism::FunctionExpression<formalism::MultiTag>>>(normalized);
        if (multi.get_operator() == op)
        {
            for (const auto expression : multi.get_args())
            {
                if (operands.empty())
                    first_operand = expression;
                operands.push_back(urd::to_stored(expression));
            }
            return;
        }
    }
    if (!is_unit(op, normalized))
    {
        if (operands.empty())
            first_operand = normalized;
        operands.push_back(urd::to_stored(normalized));
    }
}

template<typename Derived>
bool NormalizeArithmeticExpressionsTranslator<Derived>::is_unit(formalism::MultiArithmeticOperator op, formalism::FunctionExpressionView expression) const
{
    return std::holds_alternative<formalism::EntityView<formalism::FunctionExpression<formalism::NumberTag>>>(expression)
           && std::get<formalism::EntityView<formalism::FunctionExpression<formalism::NumberTag>>>(expression).get_value()
                  == (op == formalism::MultiArithmeticOperator::Add ? 0.0 : 1.0);
}

}  // namespace loki::semantic::detail

#endif
