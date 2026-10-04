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

#ifndef LOKI_SEMANTIC_TRANSLATOR_RENAME_QUANTIFIED_VARIABLES_TRANSLATOR_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_RENAME_QUANTIFIED_VARIABLES_TRANSLATOR_HPP_

#include "loki/semantic/translator/copy_translator_component.hpp"

namespace loki::semantic::detail
{

template<typename Derived>
class RenameQuantifiedVariablesTranslator : public CopyTranslatorComponent<Derived, RenameQuantifiedVariablesTranslator<Derived>>
{
public:
    explicit RenameQuantifiedVariablesTranslator(CopyContext& context) : CopyTranslatorComponent<Derived, RenameQuantifiedVariablesTranslator<Derived>>(context)
    {
    }

    void enter_variable_scope();
    void leave_variable_scope();
    formalism::ParameterView rename_parameter(formalism::ParameterView source);
    void rename_parameters(formalism::EntityListView<formalism::Parameter> source, ygg::IndexList<formalism::Parameter>& result);

    formalism::TermView rename_variables(formalism::TermView source);
    formalism::AtomView rename_variables(formalism::AtomView source);
    formalism::LiteralView rename_variables(formalism::LiteralView source);
    formalism::FunctionTermView rename_variables(formalism::FunctionTermView source);
    formalism::UnaryFunctionExpressionView rename_variables(formalism::UnaryFunctionExpressionView source);
    formalism::BinaryFunctionExpressionView rename_variables(formalism::BinaryFunctionExpressionView source);
    formalism::MultiFunctionExpressionView rename_variables(formalism::MultiFunctionExpressionView source);
    formalism::FunctionExpressionView rename_variables(formalism::FunctionExpressionView source);

    formalism::ConditionLiteralView rename_variables(formalism::ConditionLiteralView source);
    formalism::ConditionAndView rename_variables(formalism::ConditionAndView source);
    formalism::ConditionOrView rename_variables(formalism::ConditionOrView source);
    formalism::ConditionNotView rename_variables(formalism::ConditionNotView source);
    formalism::ConditionImplyView rename_variables(formalism::ConditionImplyView source);
    formalism::ConditionExistsView rename_variables(formalism::ConditionExistsView source);
    formalism::ConditionForallView rename_variables(formalism::ConditionForallView source);
    formalism::ConditionNumericConstraintView rename_variables(formalism::ConditionNumericConstraintView source);
    formalism::ConditionView rename_variables(formalism::ConditionView source);

    formalism::EffectLiteralView rename_variables(formalism::EffectLiteralView source);
    formalism::EffectAndView rename_variables(formalism::EffectAndView source);
    formalism::EffectNumericView rename_variables(formalism::EffectNumericView source);
    formalism::EffectForallView rename_variables(formalism::EffectForallView source);
    formalism::EffectWhenView rename_variables(formalism::EffectWhenView source);
    formalism::EffectOneOfView rename_variables(formalism::EffectOneOfView source);
    formalism::EffectProbabilisticAlternativeView rename_variables(formalism::EffectProbabilisticAlternativeView source);
    formalism::EffectProbabilisticView rename_variables(formalism::EffectProbabilisticView source);
    formalism::EffectView rename_variables(formalism::EffectView source);

    formalism::ActionView rename_action_variables(formalism::ActionView source);
    formalism::AxiomView rename_axiom_variables(formalism::AxiomView source);

private:
    formalism::VariableView lookup_variable(formalism::VariableView source);
    formalism::VariableView fresh_variable(formalism::VariableView source);
};

template<typename Derived>
void RenameQuantifiedVariablesTranslator<Derived>::enter_variable_scope()
{
    this->m_variable_bindings.emplace_back();
}

template<typename Derived>
void RenameQuantifiedVariablesTranslator<Derived>::leave_variable_scope()
{
    this->m_variable_bindings.pop_back();
}

template<typename Derived>
formalism::VariableView RenameQuantifiedVariablesTranslator<Derived>::lookup_variable(formalism::VariableView source)
{
    for (auto it = this->m_variable_bindings.rbegin(); it != this->m_variable_bindings.rend(); ++it)
    {
        if (auto mapped = it->find(source); mapped != it->end())
            return mapped->second;
    }
    return this->self().copy(source);
}

template<typename Derived>
formalism::VariableView RenameQuantifiedVariablesTranslator<Derived>::fresh_variable(formalism::VariableView source)
{
    // Pure capture avoidance: keep the name unless this binding shadows an enclosing one.
    // Sibling scopes may share names; the existential-lifting phase uniquifies where scopes
    // actually merge, which keeps renaming idempotent across repeated translations.
    auto shadowed = false;
    for (const auto& scope : this->m_variable_bindings)
        shadowed = shadowed || scope.contains(source);
    if (!shadowed)
        return this->self().copy(source);

    // Pick the first suffix that neither an enclosing binding nor an earlier rename in this
    // action uses; earlier renames may later be lifted into the action parameters, so reusing
    // one would shadow it on the next translation and break idempotence.
    auto taken = this->m_renamed_variable_names;
    for (const auto& scope : this->m_variable_bindings)
        for (const auto& [_, target] : scope)
            taken.insert(std::string(target.get_name()));
    for (auto k = std::size_t { 0 };; ++k)
    {
        auto name = std::string(source.get_name()) + "_" + std::to_string(k);
        if (!taken.contains(name))
        {
            this->m_renamed_variable_names.insert(name);
            auto data = formalism::checkout<formalism::Variable>(this->m_context.builder);
            data->name = cista::offset::string(name);
            return formalism::insert(this->m_storage->repository, *data).first;
        }
    }
}

template<typename Derived>
formalism::ParameterView RenameQuantifiedVariablesTranslator<Derived>::rename_parameter(formalism::ParameterView source)
{
    const auto variable = this->self().fresh_variable(source.get_variable());
    this->m_variable_bindings.back().emplace(source.get_variable(), variable);
    auto data = formalism::checkout<formalism::Parameter>(this->m_context.builder);
    data->variable = variable.get_index();
    this->self().template copy_list<formalism::Type>(source.get_types(), data->types);
    return formalism::insert(this->m_storage->repository, *data).first;
}

template<typename Derived>
void RenameQuantifiedVariablesTranslator<Derived>::rename_parameters(formalism::EntityListView<formalism::Parameter> source,
                                                                     ygg::IndexList<formalism::Parameter>& result)
{
    for (auto parameter : source)
        result.push_back(as_index(this->self().rename_parameter(parameter)));
}

template<typename Derived>
formalism::TermView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::TermView source)
{
    auto value = ygg::visit(
        [&](const auto& arg) -> ygg::Data<formalism::Term>::Variant
        {
            using Arg = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<Arg, formalism::VariableView>)
                return as_index(this->self().lookup_variable(arg));
            else
                return as_index(this->self().copy(arg));
        },
        source.get_variant());
    auto data = formalism::checkout<formalism::Term>(this->m_context.builder);
    data->variant = std::move(value);
    return formalism::insert(this->m_storage->repository, *data).first;
}

template<typename Derived>
formalism::AtomView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::AtomView source)
{
    auto data = formalism::checkout<formalism::Atom>(this->m_context.builder);
    for (auto term : source.get_terms())
        data->terms.push_back(as_index(this->self().rename_variables(term)));
    data->predicate = as_index(this->self().copy(source.get_predicate()));
    return formalism::insert(this->m_storage->repository, *data).first;
}

template<typename Derived>
formalism::LiteralView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::LiteralView source)
{
    const auto atom = as_index(this->self().rename_variables(source.get_atom()));
    auto data = formalism::checkout<formalism::Literal>(this->m_context.builder);
    data->atom = atom;
    data->m_polarity = source.get_data().m_polarity;
    return formalism::insert(this->m_storage->repository, *data).first;
}

template<typename Derived>
formalism::FunctionTermView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::FunctionTermView source)
{
    auto data = formalism::checkout<formalism::FunctionTerm>(this->m_context.builder);
    for (auto term : source.get_terms())
        data->terms.push_back(as_index(this->self().rename_variables(term)));
    data->function = as_index(this->self().copy(source.get_function()));
    return formalism::insert(this->m_storage->repository, *data).first;
}

template<typename Derived>
formalism::UnaryFunctionExpressionView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::UnaryFunctionExpressionView source)
{
    const auto& data = source.get_data();
    const auto expression = as_index(this->self().rename_variables(source.get_expression()));
    auto result = formalism::checkout<formalism::UnaryFunctionExpression>(this->m_context.builder);
    result->op = data.op;
    result->expression = expression;
    return formalism::insert(this->m_storage->repository, *result).first;
}

template<typename Derived>
formalism::BinaryFunctionExpressionView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::BinaryFunctionExpressionView source)
{
    const auto& data = source.get_data();
    const auto left = as_index(this->self().rename_variables(source.get_left()));
    const auto right = as_index(this->self().rename_variables(source.get_right()));
    auto result = formalism::checkout<formalism::BinaryFunctionExpression>(this->m_context.builder);
    result->op = data.op;
    result->left = left;
    result->right = right;
    return formalism::insert(this->m_storage->repository, *result).first;
}

template<typename Derived>
formalism::MultiFunctionExpressionView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::MultiFunctionExpressionView source)
{
    const auto& data = source.get_data();
    auto result = formalism::checkout<formalism::MultiFunctionExpression>(this->m_context.builder);
    for (auto expression : source.get_args())
        result->args.push_back(as_index(this->self().rename_variables(expression)));
    result->op = data.op;
    return formalism::insert(this->m_storage->repository, *result).first;
}

template<typename Derived>
formalism::FunctionExpressionView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::FunctionExpressionView source)
{
    auto value = ygg::visit(
        [&](const auto& arg) -> ygg::Data<formalism::FunctionExpression>::Variant
        {
            using Arg = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<Arg, formalism::FunctionExpressionNumberView>)
                return as_index(this->self().copy(arg));
            else
                return as_index(this->self().rename_variables(arg));
        },
        source.get_variant());
    auto data = formalism::checkout<formalism::FunctionExpression>(this->m_context.builder);
    data->variant = std::move(value);
    return formalism::insert(this->m_storage->repository, *data).first;
}

template<typename Derived>
formalism::ConditionLiteralView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::ConditionLiteralView source)
{
    const auto literal = as_index(this->self().rename_variables(source.get_literal()));
    auto data = formalism::checkout<formalism::ConditionLiteral>(this->m_context.builder);
    data->literal = literal;
    return formalism::insert(this->m_storage->repository, *data).first;
}

template<typename Derived>
formalism::ConditionAndView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::ConditionAndView source)
{
    auto data = formalism::checkout<formalism::ConditionAnd>(this->m_context.builder);
    for (auto condition : source.get_conditions())
        data->conditions.push_back(as_index(this->self().rename_variables(condition)));
    return formalism::insert(this->m_storage->repository, *data).first;
}

template<typename Derived>
formalism::ConditionOrView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::ConditionOrView source)
{
    auto data = formalism::checkout<formalism::ConditionOr>(this->m_context.builder);
    for (auto condition : source.get_conditions())
        data->conditions.push_back(as_index(this->self().rename_variables(condition)));
    return formalism::insert(this->m_storage->repository, *data).first;
}

template<typename Derived>
formalism::ConditionNotView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::ConditionNotView source)
{
    const auto condition = as_index(this->self().rename_variables(source.get_condition()));
    auto data = formalism::checkout<formalism::ConditionNot>(this->m_context.builder);
    data->condition = condition;
    return formalism::insert(this->m_storage->repository, *data).first;
}

template<typename Derived>
formalism::ConditionImplyView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::ConditionImplyView source)
{
    // Sequence the recursions: both sides can assign collision-rename suffixes, and argument
    // evaluation order is unspecified.
    const auto left = as_index(this->self().rename_variables(source.get_left()));
    const auto right = as_index(this->self().rename_variables(source.get_right()));
    auto data = formalism::checkout<formalism::ConditionImply>(this->m_context.builder);
    data->left = left;
    data->right = right;
    return formalism::insert(this->m_storage->repository, *data).first;
}

template<typename Derived>
formalism::ConditionExistsView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::ConditionExistsView source)
{
    this->self().enter_variable_scope();
    auto data = formalism::checkout<formalism::ConditionExists>(this->m_context.builder);
    this->self().rename_parameters(source.get_parameters(), data->parameters);
    auto condition = as_index(this->self().rename_variables(source.get_condition()));
    this->self().leave_variable_scope();
    data->condition = condition;
    return formalism::insert(this->m_storage->repository, *data).first;
}

template<typename Derived>
formalism::ConditionForallView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::ConditionForallView source)
{
    this->self().enter_variable_scope();
    auto data = formalism::checkout<formalism::ConditionForall>(this->m_context.builder);
    this->self().rename_parameters(source.get_parameters(), data->parameters);
    auto condition = as_index(this->self().rename_variables(source.get_condition()));
    this->self().leave_variable_scope();
    data->condition = condition;
    return formalism::insert(this->m_storage->repository, *data).first;
}

template<typename Derived>
formalism::ConditionNumericConstraintView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::ConditionNumericConstraintView source)
{
    const auto& data = source.get_data();
    const auto left = as_index(this->self().rename_variables(source.get_left()));
    const auto right = as_index(this->self().rename_variables(source.get_right()));
    auto result = formalism::checkout<formalism::ConditionNumericConstraint>(this->m_context.builder);
    result->comparator = data.comparator;
    result->left = left;
    result->right = right;
    return formalism::insert(this->m_storage->repository, *result).first;
}

template<typename Derived>
formalism::ConditionView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::ConditionView source)
{
    auto value = ygg::visit([&](const auto& arg) -> ygg::Data<formalism::Condition>::Variant { return as_index(this->self().rename_variables(arg)); },
                            source.get_variant());
    auto data = formalism::checkout<formalism::Condition>(this->m_context.builder);
    data->variant = std::move(value);
    return formalism::insert(this->m_storage->repository, *data).first;
}

template<typename Derived>
formalism::EffectLiteralView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::EffectLiteralView source)
{
    const auto literal = as_index(this->self().rename_variables(source.get_literal()));
    auto data = formalism::checkout<formalism::EffectLiteral>(this->m_context.builder);
    data->literal = literal;
    return formalism::insert(this->m_storage->repository, *data).first;
}

template<typename Derived>
formalism::EffectAndView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::EffectAndView source)
{
    auto data = formalism::checkout<formalism::EffectAnd>(this->m_context.builder);
    for (auto effect : source.get_effects())
        data->effects.push_back(as_index(this->self().rename_variables(effect)));
    return formalism::insert(this->m_storage->repository, *data).first;
}

template<typename Derived>
formalism::EffectNumericView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::EffectNumericView source)
{
    const auto& data = source.get_data();
    const auto function = as_index(this->self().rename_variables(source.get_function()));
    const auto expression = as_index(this->self().rename_variables(source.get_expression()));
    auto result = formalism::checkout<formalism::EffectNumeric>(this->m_context.builder);
    result->op = data.op;
    result->function = function;
    result->expression = expression;
    return formalism::insert(this->m_storage->repository, *result).first;
}

template<typename Derived>
formalism::EffectForallView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::EffectForallView source)
{
    this->self().enter_variable_scope();
    auto data = formalism::checkout<formalism::EffectForall>(this->m_context.builder);
    this->self().rename_parameters(source.get_parameters(), data->parameters);
    auto effect = as_index(this->self().rename_variables(source.get_effect()));
    this->self().leave_variable_scope();
    data->effect = effect;
    return formalism::insert(this->m_storage->repository, *data).first;
}

template<typename Derived>
formalism::EffectWhenView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::EffectWhenView source)
{
    const auto condition = as_index(this->self().rename_variables(source.get_condition()));
    const auto effect = as_index(this->self().rename_variables(source.get_effect()));
    auto data = formalism::checkout<formalism::EffectWhen>(this->m_context.builder);
    data->condition = condition;
    data->effect = effect;
    return formalism::insert(this->m_storage->repository, *data).first;
}

template<typename Derived>
formalism::EffectOneOfView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::EffectOneOfView source)
{
    auto data = formalism::checkout<formalism::EffectOneOf>(this->m_context.builder);
    for (auto effect : source.get_effects())
        data->effects.push_back(as_index(this->self().rename_variables(effect)));
    return formalism::insert(this->m_storage->repository, *data).first;
}

template<typename Derived>
formalism::EffectProbabilisticAlternativeView
RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::EffectProbabilisticAlternativeView source)
{
    const auto& data = source.get_data();
    const auto effect = as_index(this->self().rename_variables(source.get_effect()));
    auto result = formalism::checkout<formalism::EffectProbabilisticAlternative>(this->m_context.builder);
    result->probability = data.probability;
    result->effect = effect;
    return formalism::insert(this->m_storage->repository, *result).first;
}

template<typename Derived>
formalism::EffectProbabilisticView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::EffectProbabilisticView source)
{
    auto data = formalism::checkout<formalism::EffectProbabilistic>(this->m_context.builder);
    for (auto alternative : source.get_alternatives())
        data->alternatives.push_back(as_index(this->self().rename_variables(alternative)));
    return formalism::insert(this->m_storage->repository, *data).first;
}

template<typename Derived>
formalism::EffectView RenameQuantifiedVariablesTranslator<Derived>::rename_variables(formalism::EffectView source)
{
    auto value =
        ygg::visit([&](const auto& arg) -> ygg::Data<formalism::Effect>::Variant { return as_index(this->self().rename_variables(arg)); }, source.get_variant());
    auto data = formalism::checkout<formalism::Effect>(this->m_context.builder);
    data->variant = std::move(value);
    return formalism::insert(this->m_storage->repository, *data).first;
}

template<typename Derived>
formalism::ActionView RenameQuantifiedVariablesTranslator<Derived>::rename_action_variables(formalism::ActionView source)
{
    const auto& data = source.get_data();
    this->m_num_quantifications.clear();
    this->m_renamed_variable_names.clear();
    this->self().enter_variable_scope();
    auto result = formalism::checkout<formalism::Action>(this->m_context.builder);
    this->self().rename_parameters(source.get_parameters(), result->parameters);
    auto precondition = cista::optional<ygg::Index<formalism::Condition>> {};
    if (const auto condition = source.get_precondition())
        precondition = as_index(this->self().rename_variables(condition.value()));
    auto effect = cista::optional<ygg::Index<formalism::Effect>> {};
    if (const auto effect_view = source.get_effect())
        effect = as_index(this->self().rename_variables(effect_view.value()));
    this->self().leave_variable_scope();
    result->name = data.name;
    result->original_name = data.original_name;
    result->original_arity = data.original_arity;
    result->precondition = precondition;
    result->effect = effect;
    return formalism::insert(this->m_storage->repository, *result).first;
}

template<typename Derived>
formalism::AxiomView RenameQuantifiedVariablesTranslator<Derived>::rename_axiom_variables(formalism::AxiomView source)
{
    const auto& data = source.get_data();
    this->m_num_quantifications.clear();
    this->m_renamed_variable_names.clear();
    this->self().enter_variable_scope();
    auto result = formalism::checkout<formalism::Axiom>(this->m_context.builder);
    this->self().rename_parameters(source.get_parameters(), result->parameters);
    auto head = as_index(this->self().rename_variables(source.get_head()));
    auto condition = as_index(this->self().rename_variables(source.get_condition()));
    this->self().leave_variable_scope();
    result->original_arity = data.original_arity;
    result->head = head;
    result->condition = condition;
    return formalism::insert(this->m_storage->repository, *result).first;
}

}  // namespace loki::semantic::detail

#endif
