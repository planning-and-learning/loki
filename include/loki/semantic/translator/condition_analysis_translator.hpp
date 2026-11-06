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
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

#include <algorithm>
#include <urd/elements/less.hpp>

namespace loki::semantic::detail
{

template<typename Derived>
class ConditionAnalysisTranslator : public CopyTranslatorComponent<Derived, ConditionAnalysisTranslator<Derived>>
{
public:
    explicit ConditionAnalysisTranslator(CopyContext& context) : CopyTranslatorComponent<Derived, ConditionAnalysisTranslator<Derived>>(context) {}

    void
    collect_free_variables(formalism::TermView term, urd::Store<urd::set<formalism::VariableView>>& bound, urd::Store<urd::set<formalism::VariableView>>& free) const;
    void
    collect_free_variables(formalism::AtomView atom, urd::Store<urd::set<formalism::VariableView>>& bound, urd::Store<urd::set<formalism::VariableView>>& free) const;
    void collect_free_variables(formalism::LiteralView literal,
                                urd::Store<urd::set<formalism::VariableView>>& bound,
                                urd::Store<urd::set<formalism::VariableView>>& free) const;
    void collect_free_variables(formalism::FunctionTermView term,
                                urd::Store<urd::set<formalism::VariableView>>& bound,
                                urd::Store<urd::set<formalism::VariableView>>& free) const;
    void collect_free_variables(formalism::EntityView<formalism::FunctionExpression<formalism::NumberTag>>,
                                urd::Store<urd::set<formalism::VariableView>>&,
                                urd::Store<urd::set<formalism::VariableView>>&) const;
    void collect_free_variables(formalism::EntityView<formalism::FunctionExpression<formalism::UnaryTag>> expression,
                                urd::Store<urd::set<formalism::VariableView>>& bound,
                                urd::Store<urd::set<formalism::VariableView>>& free) const;
    void collect_free_variables(formalism::EntityView<formalism::FunctionExpression<formalism::BinaryTag>> expression,
                                urd::Store<urd::set<formalism::VariableView>>& bound,
                                urd::Store<urd::set<formalism::VariableView>>& free) const;
    void collect_free_variables(formalism::EntityView<formalism::FunctionExpression<formalism::MultiTag>> expression,
                                urd::Store<urd::set<formalism::VariableView>>& bound,
                                urd::Store<urd::set<formalism::VariableView>>& free) const;
    void collect_free_variables(formalism::FunctionExpressionView expression,
                                urd::Store<urd::set<formalism::VariableView>>& bound,
                                urd::Store<urd::set<formalism::VariableView>>& free) const;
    void collect_free_variables(formalism::ConditionView condition,
                                urd::Store<urd::set<formalism::VariableView>>& bound,
                                urd::Store<urd::set<formalism::VariableView>>& free) const;
    void collect_free_variables(formalism::EntityView<formalism::Condition<formalism::LiteralTag>> node,
                                urd::Store<urd::set<formalism::VariableView>>& bound,
                                urd::Store<urd::set<formalism::VariableView>>& free) const;
    void collect_free_variables(formalism::EntityView<formalism::Condition<formalism::NumericConstraintTag>> node,
                                urd::Store<urd::set<formalism::VariableView>>& bound,
                                urd::Store<urd::set<formalism::VariableView>>& free) const;
    void collect_free_variables(formalism::EntityView<formalism::Condition<formalism::NotTag>> node,
                                urd::Store<urd::set<formalism::VariableView>>& bound,
                                urd::Store<urd::set<formalism::VariableView>>& free) const;
    void collect_free_variables(formalism::EntityView<formalism::Condition<formalism::ImplyTag>> node,
                                urd::Store<urd::set<formalism::VariableView>>& bound,
                                urd::Store<urd::set<formalism::VariableView>>& free) const;
    void collect_free_variables(formalism::EntityView<formalism::Condition<formalism::AndTag>> node,
                                urd::Store<urd::set<formalism::VariableView>>& bound,
                                urd::Store<urd::set<formalism::VariableView>>& free) const;
    void collect_free_variables(formalism::EntityView<formalism::Condition<formalism::OrTag>> node,
                                urd::Store<urd::set<formalism::VariableView>>& bound,
                                urd::Store<urd::set<formalism::VariableView>>& free) const;
    void collect_free_variables(formalism::EntityView<formalism::Condition<formalism::ExistsTag>> node,
                                urd::Store<urd::set<formalism::VariableView>>& bound,
                                urd::Store<urd::set<formalism::VariableView>>& free) const;
    void collect_free_variables(formalism::EntityView<formalism::Condition<formalism::ForallTag>> node,
                                urd::Store<urd::set<formalism::VariableView>>& bound,
                                urd::Store<urd::set<formalism::VariableView>>& free) const;
    [[nodiscard]] urd::Store<urd::sequence<std::pair<formalism::ParameterView, formalism::VariableView>>> free_parameters_in_scope(formalism::ConditionView condition) const;
};

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::TermView term,
                                                                  urd::Store<urd::set<formalism::VariableView>>& bound,
                                                                  urd::Store<urd::set<formalism::VariableView>>& free) const
{
    std::visit(
        [&]<typename Value>(const Value& value)
        {
            if constexpr (std::is_same_v<Value, formalism::VariableView>)
            {
                if (!bound.contains(value))
                    free.insert(value);
            }
        },
        term);
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::AtomView atom,
                                                                  urd::Store<urd::set<formalism::VariableView>>& bound,
                                                                  urd::Store<urd::set<formalism::VariableView>>& free) const
{
    for (auto term : atom.get_terms())
        this->self().collect_free_variables(term, bound, free);
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::LiteralView literal,
                                                                  urd::Store<urd::set<formalism::VariableView>>& bound,
                                                                  urd::Store<urd::set<formalism::VariableView>>& free) const
{ this->self().collect_free_variables(literal.get_atom(), bound, free); }

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::FunctionTermView term,
                                                                  urd::Store<urd::set<formalism::VariableView>>& bound,
                                                                  urd::Store<urd::set<formalism::VariableView>>& free) const
{
    for (auto argument : term.get_terms())
        this->self().collect_free_variables(argument, bound, free);
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::EntityView<formalism::FunctionExpression<formalism::NumberTag>>,
                                                                  urd::Store<urd::set<formalism::VariableView>>&,
                                                                  urd::Store<urd::set<formalism::VariableView>>&) const
{
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::EntityView<formalism::FunctionExpression<formalism::UnaryTag>> expression,
                                                                  urd::Store<urd::set<formalism::VariableView>>& bound,
                                                                  urd::Store<urd::set<formalism::VariableView>>& free) const
{ this->self().collect_free_variables(expression.get_expression(), bound, free); }

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::EntityView<formalism::FunctionExpression<formalism::BinaryTag>> expression,
                                                                  urd::Store<urd::set<formalism::VariableView>>& bound,
                                                                  urd::Store<urd::set<formalism::VariableView>>& free) const
{
    this->self().collect_free_variables(expression.get_left(), bound, free);
    this->self().collect_free_variables(expression.get_right(), bound, free);
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::EntityView<formalism::FunctionExpression<formalism::MultiTag>> expression,
                                                                  urd::Store<urd::set<formalism::VariableView>>& bound,
                                                                  urd::Store<urd::set<formalism::VariableView>>& free) const
{
    for (auto part : expression.get_args())
        this->self().collect_free_variables(part, bound, free);
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::FunctionExpressionView expression,
                                                                  urd::Store<urd::set<formalism::VariableView>>& bound,
                                                                  urd::Store<urd::set<formalism::VariableView>>& free) const
{
    std::visit([&](const auto& node) { this->self().collect_free_variables(node, bound, free); }, expression);
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::ConditionView condition,
                                                                  urd::Store<urd::set<formalism::VariableView>>& bound,
                                                                  urd::Store<urd::set<formalism::VariableView>>& free) const
{
    std::visit([&](const auto& node) { this->self().collect_free_variables(node, bound, free); }, condition);
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::EntityView<formalism::Condition<formalism::LiteralTag>> node,
                                                                  urd::Store<urd::set<formalism::VariableView>>& bound,
                                                                  urd::Store<urd::set<formalism::VariableView>>& free) const
{ this->self().collect_free_variables(node.get_literal(), bound, free); }

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::EntityView<formalism::Condition<formalism::NumericConstraintTag>> node,
                                                                  urd::Store<urd::set<formalism::VariableView>>& bound,
                                                                  urd::Store<urd::set<formalism::VariableView>>& free) const
{
    this->self().collect_free_variables(node.get_left(), bound, free);
    this->self().collect_free_variables(node.get_right(), bound, free);
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::EntityView<formalism::Condition<formalism::NotTag>> node,
                                                                  urd::Store<urd::set<formalism::VariableView>>& bound,
                                                                  urd::Store<urd::set<formalism::VariableView>>& free) const
{ this->self().collect_free_variables(node.get_condition(), bound, free); }

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::EntityView<formalism::Condition<formalism::ImplyTag>> node,
                                                                  urd::Store<urd::set<formalism::VariableView>>& bound,
                                                                  urd::Store<urd::set<formalism::VariableView>>& free) const
{
    this->self().collect_free_variables(node.get_left(), bound, free);
    this->self().collect_free_variables(node.get_right(), bound, free);
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::EntityView<formalism::Condition<formalism::AndTag>> node,
                                                                  urd::Store<urd::set<formalism::VariableView>>& bound,
                                                                  urd::Store<urd::set<formalism::VariableView>>& free) const
{
    for (auto condition : node.get_conditions())
        this->self().collect_free_variables(condition, bound, free);
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::EntityView<formalism::Condition<formalism::OrTag>> node,
                                                                  urd::Store<urd::set<formalism::VariableView>>& bound,
                                                                  urd::Store<urd::set<formalism::VariableView>>& free) const
{
    for (auto condition : node.get_conditions())
        this->self().collect_free_variables(condition, bound, free);
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::EntityView<formalism::Condition<formalism::ExistsTag>> node,
                                                                  urd::Store<urd::set<formalism::VariableView>>& bound,
                                                                  urd::Store<urd::set<formalism::VariableView>>& free) const
{
    for (auto parameter : node.get_parameters())
        bound.insert(parameter.get_variable());
    this->self().collect_free_variables(node.get_condition(), bound, free);
    for (auto parameter : node.get_parameters())
        bound.erase(parameter.get_variable());
}

template<typename Derived>
void ConditionAnalysisTranslator<Derived>::collect_free_variables(formalism::EntityView<formalism::Condition<formalism::ForallTag>> node,
                                                                  urd::Store<urd::set<formalism::VariableView>>& bound,
                                                                  urd::Store<urd::set<formalism::VariableView>>& free) const
{
    for (auto parameter : node.get_parameters())
        bound.insert(parameter.get_variable());
    this->self().collect_free_variables(node.get_condition(), bound, free);
    for (auto parameter : node.get_parameters())
        bound.erase(parameter.get_variable());
}

template<typename Derived>
urd::Store<urd::sequence<std::pair<formalism::ParameterView, formalism::VariableView>>>
ConditionAnalysisTranslator<Derived>::free_parameters_in_scope(formalism::ConditionView condition) const
{
    auto bound = urd::Store<urd::set<formalism::VariableView>> {};
    auto free = urd::Store<urd::set<formalism::VariableView>> {};
    this->self().collect_free_variables(condition, bound, free);

    auto result = urd::Store<urd::sequence<std::pair<formalism::ParameterView, formalism::VariableView>>> {};
    for (size_t scope_index = 0; scope_index < this->copy_context().active_parameters.size(); ++scope_index)
    {
        const auto& parameters = this->copy_context().active_parameters[scope_index];
        const auto& variables = this->copy_context().active_parameter_variables[scope_index];
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
