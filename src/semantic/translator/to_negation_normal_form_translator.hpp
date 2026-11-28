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

#include "copy_translator_component.hpp"
#include "loki/semantic/errors.hpp"

#include <concepts>
#include <type_traits>

namespace loki::semantic::detail
{

template<typename Derived>
class ToNegationNormalFormTranslator : public CopyTranslatorComponent<Derived, ToNegationNormalFormTranslator<Derived>>
{
public:
    explicit ToNegationNormalFormTranslator(CopyContext& context) : CopyTranslatorComponent<Derived, ToNegationNormalFormTranslator<Derived>>(context) {}

    formalism::BinaryComparator negate_comparator(formalism::BinaryComparator comparator);
    formalism::ConditionView negate_condition(formalism::ConditionView source);
    formalism::ConditionView negate_condition_node(formalism::EntityView<formalism::Condition<formalism::LiteralTag>> source);
    formalism::ConditionView negate_condition_node(formalism::EntityView<formalism::Condition<formalism::AndTag>> source);
    formalism::ConditionView negate_condition_node(formalism::EntityView<formalism::Condition<formalism::OrTag>> source);
    formalism::ConditionView negate_condition_node(formalism::EntityView<formalism::Condition<formalism::NotTag>> source);
    formalism::ConditionView negate_condition_node(formalism::EntityView<formalism::Condition<formalism::ImplyTag>> source);
    /// The negation of a quantifier: the dual quantifier over the negated condition.
    template<typename Tag>
        requires std::same_as<Tag, formalism::ExistsTag> || std::same_as<Tag, formalism::ForallTag>
    formalism::ConditionView negate_condition_node(formalism::EntityView<formalism::Condition<Tag>> source);
    formalism::ConditionView negate_condition_node(formalism::EntityView<formalism::Condition<formalism::NumericConstraintTag>> source);
    /// At a time point, the negation of the condition; "not over all" holds at some time point of the interval and has
    /// no timed condition.
    formalism::ConditionView negate_condition_node(formalism::EntityView<formalism::Condition<formalism::TimedTag>> source);
    /// A preference counts its violations; its negation is no condition.
    formalism::ConditionView negate_condition_node(formalism::EntityView<formalism::Condition<formalism::PreferenceTag>> source);
    formalism::ConditionView copy_condition_node(formalism::EntityView<formalism::Condition<formalism::NotTag>> source);
    formalism::ConditionView copy_condition_node(formalism::EntityView<formalism::Condition<formalism::ImplyTag>> source);
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
    return std::visit([&](const auto& arg) { return this->self().negate_condition_node(arg); }, source);
}

template<typename Derived>
formalism::ConditionView
ToNegationNormalFormTranslator<Derived>::negate_condition_node(formalism::EntityView<formalism::Condition<formalism::LiteralTag>> source)
{
    const auto literal = source.literal();
    const auto atom = this->self().copy(literal.atom());
    auto literal_data = formalism::checkout<formalism::Literal>(this->copy_context().builder);
    urd::store(literal_data->atom, atom);
    literal_data->polarity = !literal.polarity();
    const auto negated_literal = this->copy_context().storage->repository.insert(*literal_data, this->copy_context().storage->builder).first;
    auto condition_data = formalism::checkout<formalism::Condition<formalism::LiteralTag>>(this->copy_context().builder);
    condition_data->literal = negated_literal.index();
    return formalism::ConditionView(this->copy_context().storage->repository.insert(*condition_data, this->copy_context().storage->builder).first);
}

template<typename Derived>
formalism::ConditionView ToNegationNormalFormTranslator<Derived>::negate_condition_node(formalism::EntityView<formalism::Condition<formalism::AndTag>> source)
{
    auto data = formalism::checkout<formalism::Condition<formalism::OrTag>>(this->copy_context().builder);
    for (auto condition : source.conditions())
        this->self().append_disjunct(*data, this->self().negate_condition(condition));
    return this->self().make_disjunction(*data);
}

template<typename Derived>
formalism::ConditionView ToNegationNormalFormTranslator<Derived>::negate_condition_node(formalism::EntityView<formalism::Condition<formalism::OrTag>> source)
{
    auto data = formalism::checkout<formalism::Condition<formalism::AndTag>>(this->copy_context().builder);
    for (auto condition : source.conditions())
        this->self().append_conjunct(*data, this->self().negate_condition(condition));
    return this->self().make_conjunction(*data);
}

template<typename Derived>
formalism::ConditionView ToNegationNormalFormTranslator<Derived>::negate_condition_node(formalism::EntityView<formalism::Condition<formalism::NotTag>> source)
{ return this->self().copy(source.condition()); }

template<typename Derived>
formalism::ConditionView ToNegationNormalFormTranslator<Derived>::negate_condition_node(formalism::EntityView<formalism::Condition<formalism::ImplyTag>> source)
{
    auto data = formalism::checkout<formalism::Condition<formalism::AndTag>>(this->copy_context().builder);
    this->self().append_conjunct(*data, this->self().copy(source.left()));
    this->self().append_conjunct(*data, this->self().negate_condition(source.right()));
    return this->self().make_conjunction(*data);
}

template<typename Derived>
template<typename Tag>
    requires std::same_as<Tag, formalism::ExistsTag> || std::same_as<Tag, formalism::ForallTag>
formalism::ConditionView ToNegationNormalFormTranslator<Derived>::negate_condition_node(formalism::EntityView<formalism::Condition<Tag>> source)
{
    using Dual = std::conditional_t<std::same_as<Tag, formalism::ExistsTag>, formalism::ForallTag, formalism::ExistsTag>;

    this->self().increment_quantifications(source.parameters());
    auto parameter_views = this->self().copy_parameter_views(source.parameters());
    this->self().enter_scope(parameter_views);
    auto condition = this->self().negate_condition(source.condition());
    this->self().leave_scope();
    auto data = formalism::checkout<formalism::Condition<Dual>>(this->copy_context().builder);
    for (auto parameter : parameter_views)
        data->parameters.push_back(parameter.index());
    urd::store(data->condition, condition);
    return this->self().flatten_condition(
        formalism::ConditionView(this->copy_context().storage->repository.insert(*data, this->copy_context().storage->builder).first));
}

template<typename Derived>
formalism::ConditionView
ToNegationNormalFormTranslator<Derived>::negate_condition_node(formalism::EntityView<formalism::Condition<formalism::NumericConstraintTag>> source)
{
    const auto comparator = this->self().negate_comparator(source.comparator());
    const auto left = this->self().copy(source.left());
    const auto right = this->self().copy(source.right());
    auto result = formalism::checkout<formalism::Condition<formalism::NumericConstraintTag>>(this->copy_context().builder);
    result->comparator = comparator;
    urd::store(result->left, left);
    urd::store(result->right, right);
    return formalism::ConditionView(this->copy_context().storage->repository.insert(*result, this->copy_context().storage->builder).first);
}

template<typename Derived>
formalism::ConditionView ToNegationNormalFormTranslator<Derived>::copy_condition_node(formalism::EntityView<formalism::Condition<formalism::NotTag>> source)
{ return this->self().negate_condition(source.condition()); }

template<typename Derived>
formalism::ConditionView ToNegationNormalFormTranslator<Derived>::copy_condition_node(formalism::EntityView<formalism::Condition<formalism::ImplyTag>> source)
{
    auto data = formalism::checkout<formalism::Condition<formalism::OrTag>>(this->copy_context().builder);
    this->self().append_disjunct(*data, this->self().negate_condition(source.left()));
    this->self().append_disjunct(*data, this->self().copy(source.right()));
    return this->self().make_disjunction(*data);
}

template<typename Derived>
template<typename T>
formalism::ConditionView ToNegationNormalFormTranslator<Derived>::copy_condition_node(formalism::EntityView<T> source)
{ return formalism::ConditionView(this->self().copy(source)); }

template<typename Derived>
formalism::ConditionView ToNegationNormalFormTranslator<Derived>::negate_condition_node(formalism::EntityView<formalism::Condition<formalism::PreferenceTag>>)
{ throw ygg::diagnostics::SemanticError("a preference cannot be negated"); }

template<typename Derived>
formalism::ConditionView ToNegationNormalFormTranslator<Derived>::negate_condition_node(formalism::EntityView<formalism::Condition<formalism::TimedTag>> source)
{
    if (source.time() == formalism::TimeSpecifier::OverAll)
        throw ygg::diagnostics::SemanticError("an over all condition cannot be negated");
    auto data = formalism::checkout<formalism::Condition<formalism::TimedTag>>(this->copy_context().builder);
    data->time = source.time();
    urd::store(data->condition, this->self().negate_condition(source.condition()));
    return formalism::ConditionView(this->copy_context().storage->repository.insert(*data, this->copy_context().storage->builder).first);
}

}  // namespace loki::semantic::detail

#endif
