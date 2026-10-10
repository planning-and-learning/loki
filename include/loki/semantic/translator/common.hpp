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

#ifndef LOKI_SEMANTIC_TRANSLATOR_COMMON_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_COMMON_HPP_

#include "loki/formalism/repository.hpp"
#include "loki/formalism/views.hpp"

#include <cista/containers/optional.h>
#include <cstddef>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/containers/associative_containers.hpp>
#include <yggdrasil/semantics/equal_to.hpp>
#include <yggdrasil/semantics/hash.hpp>

namespace loki::semantic
{

enum class TranslationPhase
{
    ToNegationNormalForm,
    RenameQuantifiedVariables,
    RemoveUniversalQuantifiers,
    SimplifyGoal,
    ToDisjunctiveNormalForm,
    SplitDisjunctiveConditions,
    MoveExistentialQuantifiers,
    CompileTyping,
    ToEffectNormalForm,
    CompileConditionalEffects,
    MaterializeEquality,
    NormalizeArithmeticExpressions,
};

inline const std::vector<std::string_view>& domain_translation_steps()
{
    static const auto steps = std::vector<std::string_view> {
        "to-negation-normal-form",
        "rename-quantified-variables",
        "remove-universal-quantifiers",
        "to-disjunctive-normal-form",
        "split-disjunctive-conditions",
        "move-existential-quantifiers",
        "compile-typing",
        "to-effect-normal-form",
        "materialize-equality",
        "normalize-arithmetic-expressions",
    };
    return steps;
}

inline const std::vector<std::string_view>& task_translation_steps()
{
    static const auto steps = std::vector<std::string_view> {
        "to-negation-normal-form",
        "rename-quantified-variables",
        "remove-universal-quantifiers",
        "simplify-goal",
        "to-disjunctive-normal-form",
        "split-disjunctive-conditions",
        "move-existential-quantifiers",
        "to-effect-normal-form",
        "materialize-equality",
        "compile-typing",
        "normalize-arithmetic-expressions",
    };
    return steps;
}

namespace detail
{

template<typename T>
using ViewMap = ygg::UnorderedMap<formalism::EntityView<T>, formalism::EntityView<T>>;

struct TranslationStorage
{
    // Keep the inherited repository layer alive until this repository is destroyed.
    std::shared_ptr<const TranslationStorage> parent;
    formalism::Repository repository;
    std::optional<formalism::DomainView> translated_domain;

    ViewMap<formalism::Requirement> requirements;
    ViewMap<formalism::Type> types;
    ViewMap<formalism::Object> objects;
    ViewMap<formalism::Variable> variables;
    ViewMap<formalism::Parameter> parameters;
    ViewMap<formalism::Predicate> predicates;
    ViewMap<formalism::FunctionSkeleton> functions;
    ViewMap<formalism::Term> terms;
    ViewMap<formalism::Atom> atoms;
    ViewMap<formalism::Literal> literals;
    ViewMap<formalism::FunctionExpression<formalism::NumberTag>> numbers;
    ViewMap<formalism::FunctionTerm> function_terms;
    ViewMap<formalism::FunctionExpression<formalism::UnaryTag>> unary_expressions;
    ViewMap<formalism::FunctionExpression<formalism::BinaryTag>> binary_expressions;
    ViewMap<formalism::FunctionExpression<formalism::MultiTag>> multi_expressions;
    ViewMap<formalism::FunctionExpression<>> function_expressions;
    ViewMap<formalism::Condition<formalism::LiteralTag>> condition_literals;
    ViewMap<formalism::Condition<formalism::AndTag>> condition_ands;
    ViewMap<formalism::Condition<formalism::OrTag>> condition_ors;
    ViewMap<formalism::Condition<formalism::NotTag>> condition_nots;
    ViewMap<formalism::Condition<formalism::ImplyTag>> condition_implies;
    ViewMap<formalism::Condition<formalism::ExistsTag>> condition_exists;
    ViewMap<formalism::Condition<formalism::ForallTag>> condition_foralls;
    ViewMap<formalism::Condition<formalism::NumericConstraintTag>> condition_numeric_constraints;
    ViewMap<formalism::Condition<>> conditions;
    ViewMap<formalism::Effect<formalism::LiteralTag>> effect_literals;
    ViewMap<formalism::Effect<formalism::AndTag>> effect_ands;
    ViewMap<formalism::Effect<formalism::NumericTag>> effect_numerics;
    ViewMap<formalism::Effect<formalism::ForallTag>> effect_foralls;
    ViewMap<formalism::Effect<formalism::WhenTag>> effect_whens;
    ViewMap<formalism::Effect<formalism::OneOfTag>> effect_one_ofs;
    ViewMap<formalism::EffectProbabilisticAlternative> effect_probabilistic_alternatives;
    ViewMap<formalism::Effect<formalism::ProbabilisticTag>> effect_probabilistics;
    ViewMap<formalism::Effect<>> effects;
    ViewMap<formalism::Action> actions;
    ViewMap<formalism::Axiom> axioms;
    ViewMap<formalism::Metric> metrics;
    ViewMap<formalism::InitialFunctionValue> initial_function_values;
    ViewMap<formalism::Domain> domains;
    ViewMap<formalism::Task> tasks;
    ygg::UnorderedMap<formalism::ObjectView, std::vector<formalism::TypeView>> object_type_views;

    explicit TranslationStorage(size_t index = 1, std::shared_ptr<const TranslationStorage> parent_ = {}) :
        parent(std::move(parent_)),
        repository(index, parent ? &parent->repository : nullptr)
    {
    }
};

std::shared_ptr<TranslationStorage> canonicalize_domain_storage(formalism::DomainView original_domain, const std::shared_ptr<TranslationStorage>& middle);
void compose_storage_maps_from_previous(TranslationStorage& target, const TranslationStorage& previous);
void inherit_domain_mappings(TranslationStorage& problem, const TranslationStorage& domain);
void inherit_domain_identity_mappings(TranslationStorage& problem, const TranslationStorage& domain);
std::shared_ptr<TranslationStorage>
canonicalize_problem_storage(formalism::TaskView middle_task,
                             const std::shared_ptr<TranslationStorage>& middle,
                             std::shared_ptr<const TranslationStorage> domain);

template<typename T>
ygg::Index<T> as_index(ygg::Index<T> index) noexcept
{
    return index;
}

template<typename T>
ygg::Index<T> as_index(formalism::EntityView<T> view) noexcept
{
    return view.get_index();
}

template<typename T>
std::optional<formalism::EntityView<T>> find_mapped(const ViewMap<T>& map, formalism::EntityView<T> source)
{
    if (auto it = map.find(source); it != map.end())
        return it->second;
    return std::nullopt;
}

template<typename T>
void remember(ViewMap<T>& map, formalism::EntityView<T> source, formalism::EntityView<T> target)
{
    map.emplace(source, target);
}

}  // namespace detail
}  // namespace loki::semantic

#endif
