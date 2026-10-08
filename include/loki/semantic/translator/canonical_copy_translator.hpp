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
    formalism::FunctionExpressionNumberView copy(formalism::FunctionExpressionNumberView source);
    formalism::FunctionTermView copy(formalism::FunctionTermView source);
    formalism::UnaryFunctionExpressionView copy(formalism::UnaryFunctionExpressionView source);
    formalism::BinaryFunctionExpressionView copy(formalism::BinaryFunctionExpressionView source);
    formalism::MultiFunctionExpressionView copy(formalism::MultiFunctionExpressionView source);
    formalism::FunctionExpressionView copy(formalism::FunctionExpressionView source);
    formalism::ConditionLiteralView copy(formalism::ConditionLiteralView source);
    formalism::ConditionAndView copy(formalism::ConditionAndView source);
    formalism::ConditionOrView copy(formalism::ConditionOrView source);
    formalism::ConditionNotView copy(formalism::ConditionNotView source);
    formalism::ConditionImplyView copy(formalism::ConditionImplyView source);
    formalism::ConditionExistsView copy(formalism::ConditionExistsView source);
    formalism::ConditionForallView copy(formalism::ConditionForallView source);
    formalism::ConditionNumericConstraintView copy(formalism::ConditionNumericConstraintView source);
    formalism::ConditionView copy(formalism::ConditionView source);
    formalism::EffectLiteralView copy(formalism::EffectLiteralView source);
    formalism::EffectAndView copy(formalism::EffectAndView source);
    formalism::EffectNumericView copy(formalism::EffectNumericView source);
    formalism::EffectForallView copy(formalism::EffectForallView source);
    formalism::EffectWhenView copy(formalism::EffectWhenView source);
    formalism::EffectOneOfView copy(formalism::EffectOneOfView source);
    formalism::EffectProbabilisticAlternativeView copy(formalism::EffectProbabilisticAlternativeView source);
    formalism::EffectProbabilisticView copy(formalism::EffectProbabilisticView source);
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
