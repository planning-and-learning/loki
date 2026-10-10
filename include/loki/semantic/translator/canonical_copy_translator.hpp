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

#ifndef LOKI_SEMANTIC_TRANSLATOR_CANONICAL_COPY_TRANSLATOR_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_CANONICAL_COPY_TRANSLATOR_HPP_

#include "loki/formalism/repository.hpp"

#include <memory>
#include <utility>

namespace loki::semantic::detail
{

struct TranslationStorage;

class CanonicalCopyTranslator
{
public:
    explicit CanonicalCopyTranslator(std::shared_ptr<TranslationStorage> storage);

    std::pair<formalism::DomainView, bool> copy(formalism::DomainView domain);
    std::pair<formalism::TaskView, bool> copy(formalism::TaskView task);

private:
    formalism::Builder m_builder;
    std::shared_ptr<TranslationStorage> m_storage;

    template<typename T>
    void copy_list(formalism::EntityListView<T> source, ygg::IndexList<T>& target);

    formalism::RequirementView copy(formalism::RequirementView source);
    formalism::TypeView copy(formalism::TypeView source);
    formalism::ObjectView copy(formalism::ObjectView source);
    formalism::VariableView copy(formalism::VariableView source);
    formalism::ParameterView copy(formalism::ParameterView source);
    formalism::PredicateView copy(formalism::PredicateView source);
    formalism::FunctionSkeletonView copy(formalism::FunctionSkeletonView source);
    formalism::TermView copy(formalism::TermView source);
    formalism::AtomView copy(formalism::AtomView source);
    formalism::LiteralView copy(formalism::LiteralView source);
    formalism::EntityView<formalism::FunctionExpression<formalism::NumberTag>> copy(formalism::EntityView<formalism::FunctionExpression<formalism::NumberTag>> source);
    formalism::FunctionTermView copy(formalism::FunctionTermView source);
    formalism::EntityView<formalism::FunctionExpression<formalism::UnaryTag>> copy(formalism::EntityView<formalism::FunctionExpression<formalism::UnaryTag>> source);
    formalism::EntityView<formalism::FunctionExpression<formalism::BinaryTag>> copy(formalism::EntityView<formalism::FunctionExpression<formalism::BinaryTag>> source);
    formalism::EntityView<formalism::FunctionExpression<formalism::MultiTag>> copy(formalism::EntityView<formalism::FunctionExpression<formalism::MultiTag>> source);
    formalism::FunctionExpressionView copy(formalism::FunctionExpressionView source);
    formalism::EntityView<formalism::Condition<formalism::LiteralTag>> copy(formalism::EntityView<formalism::Condition<formalism::LiteralTag>> source);
    formalism::EntityView<formalism::Condition<formalism::AndTag>> copy(formalism::EntityView<formalism::Condition<formalism::AndTag>> source);
    formalism::EntityView<formalism::Condition<formalism::OrTag>> copy(formalism::EntityView<formalism::Condition<formalism::OrTag>> source);
    formalism::EntityView<formalism::Condition<formalism::NotTag>> copy(formalism::EntityView<formalism::Condition<formalism::NotTag>> source);
    formalism::EntityView<formalism::Condition<formalism::ImplyTag>> copy(formalism::EntityView<formalism::Condition<formalism::ImplyTag>> source);
    formalism::EntityView<formalism::Condition<formalism::ExistsTag>> copy(formalism::EntityView<formalism::Condition<formalism::ExistsTag>> source);
    formalism::EntityView<formalism::Condition<formalism::ForallTag>> copy(formalism::EntityView<formalism::Condition<formalism::ForallTag>> source);
    formalism::EntityView<formalism::Condition<formalism::NumericConstraintTag>> copy(formalism::EntityView<formalism::Condition<formalism::NumericConstraintTag>> source);
    formalism::ConditionView copy(formalism::ConditionView source);
    formalism::EntityView<formalism::Effect<formalism::LiteralTag>> copy(formalism::EntityView<formalism::Effect<formalism::LiteralTag>> source);
    formalism::EntityView<formalism::Effect<formalism::AndTag>> copy(formalism::EntityView<formalism::Effect<formalism::AndTag>> source);
    formalism::EntityView<formalism::Effect<formalism::NumericTag>> copy(formalism::EntityView<formalism::Effect<formalism::NumericTag>> source);
    formalism::EntityView<formalism::Effect<formalism::ForallTag>> copy(formalism::EntityView<formalism::Effect<formalism::ForallTag>> source);
    formalism::EntityView<formalism::Effect<formalism::WhenTag>> copy(formalism::EntityView<formalism::Effect<formalism::WhenTag>> source);
    formalism::EntityView<formalism::Effect<formalism::OneOfTag>> copy(formalism::EntityView<formalism::Effect<formalism::OneOfTag>> source);
    formalism::EffectProbabilisticAlternativeView copy(formalism::EffectProbabilisticAlternativeView source);
    formalism::EntityView<formalism::Effect<formalism::ProbabilisticTag>> copy(formalism::EntityView<formalism::Effect<formalism::ProbabilisticTag>> source);
    formalism::EffectView copy(formalism::EffectView source);
    formalism::ActionView copy(formalism::ActionView source);
    formalism::AxiomView copy(formalism::AxiomView source);
    formalism::MetricView copy(formalism::MetricView source);
    formalism::InitialFunctionValueView copy(formalism::InitialFunctionValueView source);
};

inline auto copy(formalism::DomainView source, CanonicalCopyTranslator& context) { return context.copy(source); }
inline auto copy(formalism::TaskView source, CanonicalCopyTranslator& context) { return context.copy(source); }

template<typename T>
void CanonicalCopyTranslator::copy_list(formalism::EntityListView<T> source, ygg::IndexList<T>& target)
{
    for (auto view : source)
        target.push_back(copy(view).get_index());
}

}  // namespace loki::semantic::detail

#endif
