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

#include "loki/semantic/translator.hpp"

#include "loki/semantic/errors.hpp"
#include "loki/semantic/options.hpp"
#include "loki/semantic/translator/common.hpp"
#include "loki/semantic/translator/copy_translator.hpp"

#include <utility>

namespace loki::semantic
{

namespace detail
{

struct PhaseStep
{
    TranslationPhase phase;
    std::string_view name;
    // Compiling conditional effects negates when-conditions, which reintroduces disjunctions
    // and quantifiers into preconditions; the normalization phases must run again afterwards.
    bool requires_compile_conditional_effects = false;
};

const std::vector<PhaseStep>& domain_phase_steps()
{
    static const auto steps = std::vector<PhaseStep> {
        { TranslationPhase::ToNegationNormalForm, "to-negation-normal-form" },
        { TranslationPhase::RenameQuantifiedVariables, "rename-quantified-variables" },
        { TranslationPhase::RemoveUniversalQuantifiers, "remove-universal-quantifiers" },
        { TranslationPhase::ToDisjunctiveNormalForm, "to-disjunctive-normal-form" },
        { TranslationPhase::SplitDisjunctiveConditions, "split-disjunctive-conditions" },
        { TranslationPhase::MoveExistentialQuantifiers, "move-existential-quantifiers" },
        { TranslationPhase::CompileTyping, "compile-typing" },
        { TranslationPhase::ToEffectNormalForm, "to-effect-normal-form" },
        { TranslationPhase::CompileConditionalEffects, "compile-conditional-effects", true },
        { TranslationPhase::RemoveUniversalQuantifiers, "remove-universal-quantifiers", true },
        { TranslationPhase::ToDisjunctiveNormalForm, "to-disjunctive-normal-form", true },
        { TranslationPhase::SplitDisjunctiveConditions, "split-disjunctive-conditions", true },
        { TranslationPhase::MoveExistentialQuantifiers, "move-existential-quantifiers", true },
        { TranslationPhase::MaterializeEquality, "materialize-equality" },
        { TranslationPhase::NormalizeArithmeticExpressions, "normalize-arithmetic-expressions" },
    };
    return steps;
}

const std::vector<PhaseStep>& task_phase_steps()
{
    static const auto steps = std::vector<PhaseStep> {
        { TranslationPhase::ToNegationNormalForm, "to-negation-normal-form" },
        { TranslationPhase::RenameQuantifiedVariables, "rename-quantified-variables" },
        { TranslationPhase::RemoveUniversalQuantifiers, "remove-universal-quantifiers" },
        { TranslationPhase::SimplifyGoal, "simplify-goal" },
        { TranslationPhase::ToDisjunctiveNormalForm, "to-disjunctive-normal-form" },
        { TranslationPhase::SplitDisjunctiveConditions, "split-disjunctive-conditions" },
        { TranslationPhase::MoveExistentialQuantifiers, "move-existential-quantifiers" },
        { TranslationPhase::ToEffectNormalForm, "to-effect-normal-form" },
        { TranslationPhase::MaterializeEquality, "materialize-equality" },
        { TranslationPhase::CompileTyping, "compile-typing" },
        { TranslationPhase::NormalizeArithmeticExpressions, "normalize-arithmetic-expressions" },
    };
    return steps;
}

}  // namespace detail

DomainTranslationResult::DomainTranslationResult(formalism::DomainView original_domain_, std::shared_ptr<detail::TranslationStorage> storage_) :
    m_original_domain(original_domain_),
    m_storage(std::move(storage_))
{
}

formalism::DomainView DomainTranslationResult::get_original_domain() const noexcept { return m_original_domain; }

formalism::DomainView DomainTranslationResult::get_translated_domain() const noexcept { return *m_storage->translated_domain; }

const formalism::Repository& DomainTranslationResult::get_repository() const noexcept { return m_storage->repository; }

formalism::Repository& DomainTranslationResult::get_repository() noexcept { return m_storage->repository; }

ProblemTranslationResult::ProblemTranslationResult(formalism::TaskView original_task_,
                                                   std::shared_ptr<detail::TranslationStorage> storage_,
                                                   formalism::TaskView translated_task_) :
    m_original_task(original_task_),
    m_storage(std::move(storage_)),
    m_translated_task(translated_task_)
{
}

formalism::TaskView ProblemTranslationResult::get_original_task() const noexcept { return m_original_task; }

formalism::TaskView ProblemTranslationResult::get_translated_task() const noexcept { return m_translated_task; }

const formalism::Repository& ProblemTranslationResult::get_repository() const noexcept { return m_storage->repository; }

formalism::Repository& ProblemTranslationResult::get_repository() noexcept { return m_storage->repository; }

DomainTranslationResult translate(formalism::DomainView domain, const TranslatorOptions& options)
{
    auto current_domain = domain;
    auto current_storage = std::shared_ptr<detail::TranslationStorage> {};
    auto phase_index = size_t { 1 };

    for (const auto& step : detail::domain_phase_steps())
    {
        if (step.requires_compile_conditional_effects && !options.compile_conditional_effects)
            continue;
        if (step.phase == TranslationPhase::MaterializeEquality && !options.materialize_equality)
            continue;
        if (step.phase == TranslationPhase::NormalizeArithmeticExpressions && !options.normalize_arithmetic_expressions)
            continue;

        auto phase_storage = std::make_shared<detail::TranslationStorage>(phase_index++);
        auto semantic_copier = detail::CopyTranslator(phase_storage, options.compile_typing, step.phase);
        current_domain = detail::copy(current_domain, semantic_copier).first;
        if (current_storage)
            detail::compose_storage_maps_from_previous(*phase_storage, *current_storage);
        current_storage = std::move(phase_storage);
    }

    return DomainTranslationResult(domain, detail::canonicalize_domain_storage(domain, current_storage));
}

ProblemTranslationResult translate(formalism::TaskView task, const DomainTranslationResult& result, const TranslatorOptions& options)
{
    const auto expected_domain = result.get_original_domain();
    const auto task_domain = task.get_domain();
    if (task_domain.get_index() != expected_domain.get_index() || &task_domain.get_context().get_root() != &expected_domain.get_context().get_root())
        throw MismatchedDomainError(std::string(expected_domain.get_name()), std::string(task_domain.get_name()));

    auto current_task = task;
    auto current_storage = std::shared_ptr<detail::TranslationStorage> {};
    auto phase_index = size_t { 1 };

    for (const auto& step : detail::task_phase_steps())
    {
        if (step.phase == TranslationPhase::MaterializeEquality && !options.materialize_equality)
            continue;
        if (step.phase == TranslationPhase::NormalizeArithmeticExpressions && !options.normalize_arithmetic_expressions)
            continue;

        auto phase_storage = std::make_shared<detail::TranslationStorage>(phase_index++, &result.m_storage->repository);
        if (current_storage)
            detail::inherit_domain_identity_mappings(*phase_storage, *result.m_storage);
        else
            detail::inherit_domain_mappings(*phase_storage, *result.m_storage);

        auto semantic_copier = detail::CopyTranslator(phase_storage, options.compile_typing, step.phase);
        current_task = detail::copy(current_task, semantic_copier).first;
        if (current_storage)
            detail::compose_storage_maps_from_previous(*phase_storage, *current_storage);
        current_storage = std::move(phase_storage);
    }

    const auto canonical = detail::canonicalize_problem_storage(current_task, current_storage, *result.m_storage);
    auto translated_task = canonical->tasks.at(task);
    return ProblemTranslationResult(task, canonical, translated_task);
}

}  // namespace loki::semantic
