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

#ifndef LOKI_SEMANTIC_TRANSLATOR_TO_NEGATION_NORMAL_FORM_TRANSLATOR_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_TO_NEGATION_NORMAL_FORM_TRANSLATOR_HPP_

#include "loki/semantic/translator/copy_translator_component.hpp"

namespace loki::semantic::detail
{

template<typename Derived>
class ToNegationNormalFormTranslator : public CopyTranslatorComponent<Derived, ToNegationNormalFormTranslator<Derived>>
{
public:
    explicit ToNegationNormalFormTranslator(CopyContext& context) : CopyTranslatorComponent<Derived, ToNegationNormalFormTranslator<Derived>>(context) {}

    formalism::BinaryComparator negate_comparator(formalism::BinaryComparator comparator);
    formalism::ConditionView negate_condition(formalism::ConditionView source);
    formalism::ConditionView negate_condition_node(formalism::ConditionLiteralView source);
    formalism::ConditionView negate_condition_node(formalism::ConditionAndView source);
    formalism::ConditionView negate_condition_node(formalism::ConditionOrView source);
    formalism::ConditionView negate_condition_node(formalism::ConditionNotView source);
    formalism::ConditionView negate_condition_node(formalism::ConditionImplyView source);
    formalism::ConditionView negate_condition_node(formalism::ConditionExistsView source);
    formalism::ConditionView negate_condition_node(formalism::ConditionForallView source);
    formalism::ConditionView negate_condition_node(formalism::ConditionNumericConstraintView source);
    formalism::ConditionView copy_condition_node(formalism::ConditionNotView source);
    formalism::ConditionView copy_condition_node(formalism::ConditionImplyView source);
    template<typename T>
    formalism::ConditionView copy_condition_node(formalism::EntityView<T> source);
};

template<typename Derived>
formalism::BinaryComparator ToNegationNormalFormTranslator<Derived>::negate_comparator(formalism::BinaryComparator comparator)
{
    switch (comparator)
    {
        case formalism::BinaryComparator::Eq:
            return formalism::BinaryComparator::Ne;
        case formalism::BinaryComparator::Ne:
            return formalism::BinaryComparator::Eq;
        case formalism::BinaryComparator::Lt:
            return formalism::BinaryComparator::Ge;
        case formalism::BinaryComparator::Le:
            return formalism::BinaryComparator::Gt;
        case formalism::BinaryComparator::Gt:
            return formalism::BinaryComparator::Le;
        case formalism::BinaryComparator::Ge:
            return formalism::BinaryComparator::Lt;
    }
    return formalism::BinaryComparator::Ne;
}

template<typename Derived>
formalism::ConditionView ToNegationNormalFormTranslator<Derived>::negate_condition(formalism::ConditionView source)
{
    return ygg::visit([&](const auto& arg) { return this->self().negate_condition_node(arg); }, source.get_variant());
}

template<typename Derived>
formalism::ConditionView ToNegationNormalFormTranslator<Derived>::negate_condition_node(formalism::ConditionLiteralView source)
{
    const auto literal = source.get_literal();
    const auto atom = as_index(this->self().copy(literal.get_atom()));
    auto literal_data = formalism::checkout<formalism::Literal>(this->m_context.builder);
    literal_data->atom = atom;
    literal_data->m_polarity = !literal.get_polarity();
    const auto negated_literal = formalism::insert(this->m_context.storage->repository, *literal_data).first;
    auto condition_data = formalism::checkout<formalism::ConditionLiteral>(this->m_context.builder);
    condition_data->literal = negated_literal.get_index();
    return this->self().wrap_condition(formalism::insert(this->m_context.storage->repository, *condition_data).first);
}

template<typename Derived>
formalism::ConditionView ToNegationNormalFormTranslator<Derived>::negate_condition_node(formalism::ConditionAndView source)
{
    auto data = formalism::checkout<formalism::ConditionOr>(this->m_context.builder);
    for (auto condition : source.get_conditions())
        this->self().append_disjunct(*data, this->self().negate_condition(condition));
    return this->self().make_disjunction(*data);
}

template<typename Derived>
formalism::ConditionView ToNegationNormalFormTranslator<Derived>::negate_condition_node(formalism::ConditionOrView source)
{
    auto data = formalism::checkout<formalism::ConditionAnd>(this->m_context.builder);
    for (auto condition : source.get_conditions())
        this->self().append_conjunct(*data, this->self().negate_condition(condition));
    return this->self().make_conjunction(*data);
}

template<typename Derived>
formalism::ConditionView ToNegationNormalFormTranslator<Derived>::negate_condition_node(formalism::ConditionNotView source)
{
    return this->self().copy(source.get_condition());
}

template<typename Derived>
formalism::ConditionView ToNegationNormalFormTranslator<Derived>::negate_condition_node(formalism::ConditionImplyView source)
{
    auto data = formalism::checkout<formalism::ConditionAnd>(this->m_context.builder);
    this->self().append_conjunct(*data, this->self().copy(source.get_left()));
    this->self().append_conjunct(*data, this->self().negate_condition(source.get_right()));
    return this->self().make_conjunction(*data);
}

template<typename Derived>
formalism::ConditionView ToNegationNormalFormTranslator<Derived>::negate_condition_node(formalism::ConditionExistsView source)
{
    this->self().increment_quantifications(source.get_parameters());
    auto parameter_views = this->self().copy_parameter_views(source.get_parameters());
    this->self().enter_scope(parameter_views);
    auto condition = as_index(this->self().negate_condition(source.get_condition()));
    this->self().leave_scope();
    auto data = formalism::checkout<formalism::ConditionForall>(this->m_context.builder);
    for (auto parameter : parameter_views)
        data->parameters.push_back(parameter.get_index());
    data->condition = condition;
    return this->self().flatten_condition(this->self().wrap_condition(formalism::insert(this->m_context.storage->repository, *data).first));
}

template<typename Derived>
formalism::ConditionView ToNegationNormalFormTranslator<Derived>::negate_condition_node(formalism::ConditionForallView source)
{
    this->self().increment_quantifications(source.get_parameters());
    auto parameter_views = this->self().copy_parameter_views(source.get_parameters());
    this->self().enter_scope(parameter_views);
    auto condition = as_index(this->self().negate_condition(source.get_condition()));
    this->self().leave_scope();
    auto data = formalism::checkout<formalism::ConditionExists>(this->m_context.builder);
    for (auto parameter : parameter_views)
        data->parameters.push_back(parameter.get_index());
    data->condition = condition;
    return this->self().flatten_condition(this->self().wrap_condition(formalism::insert(this->m_context.storage->repository, *data).first));
}

template<typename Derived>
formalism::ConditionView ToNegationNormalFormTranslator<Derived>::negate_condition_node(formalism::ConditionNumericConstraintView source)
{
    const auto& data = source.get_data();
    const auto comparator = this->self().negate_comparator(data.comparator);
    const auto left = as_index(this->self().copy(source.get_left()));
    const auto right = as_index(this->self().copy(source.get_right()));
    auto result = formalism::checkout<formalism::ConditionNumericConstraint>(this->m_context.builder);
    result->comparator = comparator;
    result->left = left;
    result->right = right;
    return this->self().wrap_condition(formalism::insert(this->m_context.storage->repository, *result).first);
}

template<typename Derived>
formalism::ConditionView ToNegationNormalFormTranslator<Derived>::copy_condition_node(formalism::ConditionNotView source)
{
    return this->self().negate_condition(source.get_condition());
}

template<typename Derived>
formalism::ConditionView ToNegationNormalFormTranslator<Derived>::copy_condition_node(formalism::ConditionImplyView source)
{
    auto data = formalism::checkout<formalism::ConditionOr>(this->m_context.builder);
    this->self().append_disjunct(*data, this->self().negate_condition(source.get_left()));
    this->self().append_disjunct(*data, this->self().copy(source.get_right()));
    return this->self().make_disjunction(*data);
}

template<typename Derived>
template<typename T>
formalism::ConditionView ToNegationNormalFormTranslator<Derived>::copy_condition_node(formalism::EntityView<T> source)
{
    return this->self().wrap_condition(this->self().copy(source));
}

}  // namespace loki::semantic::detail

#endif
