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

#ifndef LOKI_SEMANTIC_TRANSLATOR_CONDITION_ANALYSIS_TRANSLATOR_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_CONDITION_ANALYSIS_TRANSLATOR_HPP_

#include "loki/semantic/translator/copy_translator_component.hpp"

#include <algorithm>
#include <yggdrasil/semantics/comparators.hpp>

namespace loki::semantic::detail
{

template<typename Derived>
class ConditionAnalysisTranslator : public CopyTranslatorComponent<Derived, ConditionAnalysisTranslator<Derived>>
{
public:
    explicit ConditionAnalysisTranslator(CopyContext& context) : CopyTranslatorComponent<Derived, ConditionAnalysisTranslator<Derived>>(context) {}

    formalism::TermView term_from_variable(formalism::VariableView variable);
    void
    collect_free_variables(formalism::TermView term, ygg::UnorderedSet<formalism::VariableView>& bound, ygg::UnorderedSet<formalism::VariableView>& free) const;
    void
    collect_free_variables(formalism::AtomView atom, ygg::UnorderedSet<formalism::VariableView>& bound, ygg::UnorderedSet<formalism::VariableView>& free) const;
    void collect_free_variables(formalism::LiteralView literal,
                                ygg::UnorderedSet<formalism::VariableView>& bound,
                                ygg::UnorderedSet<formalism::VariableView>& free) const;
    void collect_free_variables(formalism::FunctionTermView term,
                                ygg::UnorderedSet<formalism::VariableView>& bound,
                                ygg::UnorderedSet<formalism::VariableView>& free) const;
    void collect_free_variables(formalism::EntityView<formalism::FunctionExpression<formalism::NumberTag>>,
                                ygg::UnorderedSet<formalism::VariableView>&,
                                ygg::UnorderedSet<formalism::VariableView>&) const;
    void collect_free_variables(formalism::EntityView<formalism::FunctionExpression<formalism::UnaryTag>> expression,
                                ygg::UnorderedSet<formalism::VariableView>& bound,
                                ygg::UnorderedSet<formalism::VariableView>& free) const;
    void collect_free_variables(formalism::EntityView<formalism::FunctionExpression<formalism::BinaryTag>> expression,
                                ygg::UnorderedSet<formalism::VariableView>& bound,
                                ygg::UnorderedSet<formalism::VariableView>& free) const;
    void collect_free_variables(formalism::EntityView<formalism::FunctionExpression<formalism::MultiTag>> expression,
                                ygg::UnorderedSet<formalism::VariableView>& bound,
                                ygg::UnorderedSet<formalism::VariableView>& free) const;
    void collect_free_variables(formalism::FunctionExpressionView expression,
                                ygg::UnorderedSet<formalism::VariableView>& bound,
                                ygg::UnorderedSet<formalism::VariableView>& free) const;
    void collect_free_variables(formalism::ConditionView condition,
                                ygg::UnorderedSet<formalism::VariableView>& bound,
                                ygg::UnorderedSet<formalism::VariableView>& free) const;
    void collect_free_variables(formalism::EntityView<formalism::Condition<formalism::LiteralTag>> node,
                                ygg::UnorderedSet<formalism::VariableView>& bound,
                                ygg::UnorderedSet<formalism::VariableView>& free) const;
    void collect_free_variables(formalism::EntityView<formalism::Condition<formalism::NumericConstraintTag>> node,
                                ygg::UnorderedSet<formalism::VariableView>& bound,
                                ygg::UnorderedSet<formalism::VariableView>& free) const;
    void collect_free_variables(formalism::EntityView<formalism::Condition<formalism::NotTag>> node,
                                ygg::UnorderedSet<formalism::VariableView>& bound,
                                ygg::UnorderedSet<formalism::VariableView>& free) const;
    void collect_free_variables(formalism::EntityView<formalism::Condition<formalism::ImplyTag>> node,
                                ygg::UnorderedSet<formalism::VariableView>& bound,
                                ygg::UnorderedSet<formalism::VariableView>& free) const;
    void collect_free_variables(formalism::EntityView<formalism::Condition<formalism::AndTag>> node,
                                ygg::UnorderedSet<formalism::VariableView>& bound,
                                ygg::UnorderedSet<formalism::VariableView>& free) const;
    void collect_free_variables(formalism::EntityView<formalism::Condition<formalism::OrTag>> node,
                                ygg::UnorderedSet<formalism::VariableView>& bound,
                                ygg::UnorderedSet<formalism::VariableView>& free) const;
    void collect_free_variables(formalism::EntityView<formalism::Condition<formalism::ExistsTag>> node,
                                ygg::UnorderedSet<formalism::VariableView>& bound,
                                ygg::UnorderedSet<formalism::VariableView>& free) const;
    void collect_free_variables(formalism::EntityView<formalism::Condition<formalism::ForallTag>> node,
                                ygg::UnorderedSet<formalism::VariableView>& bound,
                                ygg::UnorderedSet<formalism::VariableView>& free) const;
    std::vector<std::pair<formalism::ParameterView, formalism::VariableView>> free_parameters_in_scope(formalism::ConditionView condition) const;
};

template<typename Derived>
formalism::TermView ConditionAnalysisTranslator<Derived>::term_from_variable(formalism::VariableView variable)
{
    auto data = formalism::checkout<formalism::Term>(this->m_context.builder);
    data->variant = ygg::Data<formalism::Term>::Variant(variable.get_index());
    return formalism::insert(this->m_context.storage->repository, *data).first;
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::TermView term,
                                                                  ygg::UnorderedSet<formalism::VariableView>& bound,
                                                                  ygg::UnorderedSet<formalism::VariableView>& free) const
{
    ygg::visit(
        [&](const auto& value)
        {
            using Value = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<Value, formalism::VariableView>)
            {
                if (!bound.contains(value))
                    free.insert(value);
            }
        },
        term.get_variant());
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::AtomView atom,
                                                                  ygg::UnorderedSet<formalism::VariableView>& bound,
                                                                  ygg::UnorderedSet<formalism::VariableView>& free) const
{
    for (auto term : atom.get_terms())
        this->self().collect_free_variables(term, bound, free);
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::LiteralView literal,
                                                                  ygg::UnorderedSet<formalism::VariableView>& bound,
                                                                  ygg::UnorderedSet<formalism::VariableView>& free) const
{
    this->self().collect_free_variables(literal.get_atom(), bound, free);
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::FunctionTermView term,
                                                                  ygg::UnorderedSet<formalism::VariableView>& bound,
                                                                  ygg::UnorderedSet<formalism::VariableView>& free) const
{
    for (auto argument : term.get_terms())
        this->self().collect_free_variables(argument, bound, free);
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::EntityView<formalism::FunctionExpression<formalism::NumberTag>>,
                                                                  ygg::UnorderedSet<formalism::VariableView>&,
                                                                  ygg::UnorderedSet<formalism::VariableView>&) const
{
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::EntityView<formalism::FunctionExpression<formalism::UnaryTag>> expression,
                                                                  ygg::UnorderedSet<formalism::VariableView>& bound,
                                                                  ygg::UnorderedSet<formalism::VariableView>& free) const
{
    this->self().collect_free_variables(expression.get_expression(), bound, free);
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::EntityView<formalism::FunctionExpression<formalism::BinaryTag>> expression,
                                                                  ygg::UnorderedSet<formalism::VariableView>& bound,
                                                                  ygg::UnorderedSet<formalism::VariableView>& free) const
{
    this->self().collect_free_variables(expression.get_left(), bound, free);
    this->self().collect_free_variables(expression.get_right(), bound, free);
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::EntityView<formalism::FunctionExpression<formalism::MultiTag>> expression,
                                                                  ygg::UnorderedSet<formalism::VariableView>& bound,
                                                                  ygg::UnorderedSet<formalism::VariableView>& free) const
{
    for (auto part : expression.get_args())
        this->self().collect_free_variables(part, bound, free);
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::FunctionExpressionView expression,
                                                                  ygg::UnorderedSet<formalism::VariableView>& bound,
                                                                  ygg::UnorderedSet<formalism::VariableView>& free) const
{
    ygg::visit([&](const auto& node) { this->self().collect_free_variables(node, bound, free); }, expression.get_variant());
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::ConditionView condition,
                                                                  ygg::UnorderedSet<formalism::VariableView>& bound,
                                                                  ygg::UnorderedSet<formalism::VariableView>& free) const
{
    ygg::visit([&](const auto& node) { this->self().collect_free_variables(node, bound, free); }, condition.get_variant());
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::EntityView<formalism::Condition<formalism::LiteralTag>> node,
                                                                  ygg::UnorderedSet<formalism::VariableView>& bound,
                                                                  ygg::UnorderedSet<formalism::VariableView>& free) const
{
    this->self().collect_free_variables(node.get_literal(), bound, free);
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::EntityView<formalism::Condition<formalism::NumericConstraintTag>> node,
                                                                  ygg::UnorderedSet<formalism::VariableView>& bound,
                                                                  ygg::UnorderedSet<formalism::VariableView>& free) const
{
    this->self().collect_free_variables(node.get_left(), bound, free);
    this->self().collect_free_variables(node.get_right(), bound, free);
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::EntityView<formalism::Condition<formalism::NotTag>> node,
                                                                  ygg::UnorderedSet<formalism::VariableView>& bound,
                                                                  ygg::UnorderedSet<formalism::VariableView>& free) const
{
    this->self().collect_free_variables(node.get_condition(), bound, free);
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::EntityView<formalism::Condition<formalism::ImplyTag>> node,
                                                                  ygg::UnorderedSet<formalism::VariableView>& bound,
                                                                  ygg::UnorderedSet<formalism::VariableView>& free) const
{
    this->self().collect_free_variables(node.get_left(), bound, free);
    this->self().collect_free_variables(node.get_right(), bound, free);
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::EntityView<formalism::Condition<formalism::AndTag>> node,
                                                                  ygg::UnorderedSet<formalism::VariableView>& bound,
                                                                  ygg::UnorderedSet<formalism::VariableView>& free) const
{
    for (auto condition : node.get_conditions())
        this->self().collect_free_variables(condition, bound, free);
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::EntityView<formalism::Condition<formalism::OrTag>> node,
                                                                  ygg::UnorderedSet<formalism::VariableView>& bound,
                                                                  ygg::UnorderedSet<formalism::VariableView>& free) const
{
    for (auto condition : node.get_conditions())
        this->self().collect_free_variables(condition, bound, free);
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::EntityView<formalism::Condition<formalism::ExistsTag>> node,
                                                                  ygg::UnorderedSet<formalism::VariableView>& bound,
                                                                  ygg::UnorderedSet<formalism::VariableView>& free) const
{
    for (auto parameter : node.get_parameters())
        bound.insert(parameter.get_variable());
    this->self().collect_free_variables(node.get_condition(), bound, free);
    for (auto parameter : node.get_parameters())
        bound.erase(parameter.get_variable());
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::EntityView<formalism::Condition<formalism::ForallTag>> node,
                                                                  ygg::UnorderedSet<formalism::VariableView>& bound,
                                                                  ygg::UnorderedSet<formalism::VariableView>& free) const
{
    for (auto parameter : node.get_parameters())
        bound.insert(parameter.get_variable());
    this->self().collect_free_variables(node.get_condition(), bound, free);
    for (auto parameter : node.get_parameters())
        bound.erase(parameter.get_variable());
}

template<typename Derived>
std::vector<std::pair<formalism::ParameterView, formalism::VariableView>>
ConditionAnalysisTranslator<Derived>::free_parameters_in_scope(formalism::ConditionView condition) const
{
    auto bound = ygg::UnorderedSet<formalism::VariableView> {};
    auto free = ygg::UnorderedSet<formalism::VariableView> {};
    this->self().collect_free_variables(condition, bound, free);

    auto result = std::vector<std::pair<formalism::ParameterView, formalism::VariableView>> {};
    for (size_t scope_index = 0; scope_index < this->m_context.active_parameters.size(); ++scope_index)
    {
        const auto& parameters = this->m_context.active_parameters[scope_index];
        const auto& variables = this->m_context.active_parameter_variables[scope_index];
        for (size_t i = 0; i < parameters.size(); ++i)
        {
            if (free.contains(variables[i]))
                result.emplace_back(parameters[i], variables[i]);
        }
    }
    std::sort(result.begin(), result.end());
    return result;
}

}  // namespace loki::semantic::detail

#endif
