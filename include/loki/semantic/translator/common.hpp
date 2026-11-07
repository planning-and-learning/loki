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

#include <boost/mp11/algorithm.hpp>
#include <cista/containers/optional.h>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <urd/elements/equal_to.hpp>
#include <urd/elements/hash.hpp>
#include <urd/repr/maps/std_map.hpp>
#include <urd/repr/maps/unordered_flat_map.hpp>
#include <urd/repr/values/unordered_flat_set.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <urd/utils/family.hpp>
#include <utility>
#include <variant>
#include <vector>
#include <urd/config.hpp>

namespace loki::semantic
{

enum class TranslationPhase : std::uint8_t
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
    InferRequirements,
};

inline const urd::Store<urd::sequence<std::string_view>>& domain_translation_steps()
{
    static const auto steps = urd::Store<urd::sequence<std::string_view>> {
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
        "infer-requirements",
    };
    return steps;
}

inline const urd::Store<urd::sequence<std::string_view>>& task_translation_steps()
{
    static const auto steps = urd::Store<urd::sequence<std::string_view>> {
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
        "infer-requirements",
    };
    return steps;
}

namespace detail
{

template<typename T>
using ViewMap = urd::Store<urd::map<formalism::EntityView<T>, formalism::EntityView<T>>>;

template<typename... Ts>
using ViewMapFamily = urd::utils::Family<ViewMap, Ts...>;

/// One source-to-target map per kind of entity in the repository.
using ViewMaps = boost::mp11::mp_apply<ViewMapFamily, formalism::SymbolRepositoryTypes>;

struct TranslationStorage
{
    // Keep the inherited repository layer alive until this repository is destroyed.
    std::shared_ptr<const TranslationStorage> parent;
    formalism::Repository repository;
    std::optional<formalism::DomainView> translated_domain;

    /// The translated domain; a problem's translation starts from it, so it is set (asserted in Debug).
    [[nodiscard]] formalism::DomainView translated() const noexcept
    {
        URD_ASSUME(translated_domain.has_value());
        return *translated_domain;
    }

    ViewMaps maps;
    urd::Store<urd::map<formalism::ObjectView, urd::Store<urd::sequence<formalism::TypeView>>>> object_type_views;

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
std::shared_ptr<TranslationStorage> canonicalize_problem_storage(formalism::TaskView middle_task,
                                                                 const std::shared_ptr<TranslationStorage>& middle,
                                                                 const std::shared_ptr<const TranslationStorage>& domain);

template<typename T>
std::optional<formalism::EntityView<T>> find_mapped(const ViewMaps& maps, formalism::EntityView<T> source)
{
    const auto& map = maps.template get<T>();
    if (auto it = map.find(source); it != map.end())
        return it->second;
    return std::nullopt;
}

template<typename T>
void remember(ViewMaps& maps, formalism::EntityView<T> source, formalism::EntityView<T> target)
{ maps.template get<T>().emplace(source, target); }

}  // namespace detail
}  // namespace loki::semantic

#endif
