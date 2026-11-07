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

#ifndef LOKI_SEMANTIC_TRANSLATOR_COPY_CONTEXT_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_COPY_CONTEXT_HPP_

#include "loki/formalism/repository.hpp"
#include "loki/semantic/translator/common.hpp"

#include <urd/repr/maps/std_map.hpp>
#include <urd/repr/maps/unordered_flat_map.hpp>
#include <urd/repr/values/std_set.hpp>
#include <urd/repr/values/unordered_flat_set.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>


namespace loki::semantic::detail
{

using ScopedFreeParameter = std::pair<formalism::ParameterView, formalism::VariableView>;
using GeneratedUniversalConditionKey = std::pair<urd::Store<urd::sequence<ScopedFreeParameter>>, formalism::ConditionView>;

struct CopyContext
{
    formalism::Builder builder;
    std::shared_ptr<TranslationStorage> storage;
    urd::Store<urd::map<formalism::VariableView, size_t>> num_quantifications;
    /// The parameters in scope with their variables, a sequence per scope, innermost last.
    urd::Store<urd::sequence<urd::Store<urd::sequence<ScopedFreeParameter>>>> active_parameters;
    urd::Store<urd::sequence<urd::Store<urd::map<formalism::VariableView, formalism::VariableView>>>> variable_bindings;
    urd::Store<urd::set<std::string>> renamed_variable_names;
    urd::Store<urd::set<std::string>> used_predicate_names;
    urd::Store<urd::sequence<formalism::PredicateView>> generated_predicates;
    urd::Store<urd::sequence<formalism::AxiomView>> generated_axioms;
    urd::Store<urd::map<GeneratedUniversalConditionKey, formalism::ConditionView>> generated_universal_conditions;
    size_t num_generated_axioms = 0;
    urd::Store<urd::map<formalism::TypeView, formalism::PredicateView>> type_predicates;
    bool compile_typing = true;
    bool renaming_enabled = true;
    TranslationPhase phase = TranslationPhase::ToNegationNormalForm;
};

class CopyContextOwner
{
public:
    explicit CopyContextOwner(std::shared_ptr<TranslationStorage> storage, bool compile_typing, TranslationPhase phase)
    {
        m_context.storage = std::move(storage);
        m_context.compile_typing = compile_typing;
        m_context.phase = phase;
    }

    CopyContext& context() noexcept { return m_context; }
    const CopyContext& context() const noexcept { return m_context; }

private:
    CopyContext m_context;
};

}  // namespace loki::semantic::detail

#endif
