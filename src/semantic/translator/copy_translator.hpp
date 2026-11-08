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

#ifndef LOKI_SEMANTIC_TRANSLATOR_COPY_TRANSLATOR_HPP_
#define LOKI_SEMANTIC_TRANSLATOR_COPY_TRANSLATOR_HPP_

#include "basic_copy_translator.hpp"
#include "compile_conditional_effects_translator.hpp"
#include "compile_typing_translator.hpp"
#include "condition_analysis_translator.hpp"
#include "condition_builder_translator.hpp"
#include "condition_translator.hpp"
#include "copy_translator_facade.hpp"
#include "effect_translator.hpp"
#include "materialize_equality_translator.hpp"
#include "move_existential_quantifiers_translator.hpp"
#include "normalize_arithmetic_expressions_translator.hpp"
#include "remove_universal_quantifiers_translator.hpp"
#include "rename_quantified_variables_translator.hpp"
#include "simplify_goal_translator.hpp"
#include "split_disjunctive_conditions_translator.hpp"
#include "to_disjunctive_normal_form_translator.hpp"
#include "to_effect_normal_form_translator.hpp"
#include "to_negation_normal_form_translator.hpp"
#include "top_level_translator.hpp"

#include <utility>

namespace loki::semantic::detail
{

class CopyTranslator :
    private CopyContextOwner,
    public CopyTranslatorFacade<CopyTranslator>,
    public BasicCopyTranslator<CopyTranslator>,
    public CompileConditionalEffectsTranslator<CopyTranslator>,
    public ConditionTranslator<CopyTranslator>,
    public ConditionAnalysisTranslator<CopyTranslator>,
    public ConditionBuilderTranslator<CopyTranslator>,
    public ToDisjunctiveNormalFormTranslator<CopyTranslator>,
    public ToNegationNormalFormTranslator<CopyTranslator>,
    public RemoveUniversalQuantifiersTranslator<CopyTranslator>,
    public MoveExistentialQuantifiersTranslator<CopyTranslator>,
    public SplitDisjunctiveConditionsTranslator<CopyTranslator>,
    public EffectTranslator<CopyTranslator>,
    public ToEffectNormalFormTranslator<CopyTranslator>,
    public NormalizeArithmeticExpressionsTranslator<CopyTranslator>,
    public MaterializeEqualityTranslator<CopyTranslator>,
    public SimplifyGoalTranslator<CopyTranslator>,
    public TopLevelTranslator<CopyTranslator>,
    public CompileTypingTranslator<CopyTranslator>,
    public RenameQuantifiedVariablesTranslator<CopyTranslator>
{
public:
    explicit CopyTranslator(std::shared_ptr<TranslationStorage> storage,
                            bool compile_typing = true,
                            TranslationPhase phase = TranslationPhase::ToNegationNormalForm) :
        CopyContextOwner(std::move(storage), compile_typing, phase),
        CopyTranslatorFacade<CopyTranslator>(context()),
        BasicCopyTranslator<CopyTranslator>(context()),
        CompileConditionalEffectsTranslator<CopyTranslator>(context()),
        ConditionTranslator<CopyTranslator>(context()),
        ConditionAnalysisTranslator<CopyTranslator>(context()),
        ConditionBuilderTranslator<CopyTranslator>(context()),
        ToDisjunctiveNormalFormTranslator<CopyTranslator>(context()),
        ToNegationNormalFormTranslator<CopyTranslator>(context()),
        RemoveUniversalQuantifiersTranslator<CopyTranslator>(context()),
        MoveExistentialQuantifiersTranslator<CopyTranslator>(context()),
        SplitDisjunctiveConditionsTranslator<CopyTranslator>(context()),
        EffectTranslator<CopyTranslator>(context()),
        ToEffectNormalFormTranslator<CopyTranslator>(context()),
        NormalizeArithmeticExpressionsTranslator<CopyTranslator>(context()),
        MaterializeEqualityTranslator<CopyTranslator>(context()),
        SimplifyGoalTranslator<CopyTranslator>(context()),
        TopLevelTranslator<CopyTranslator>(context()),
        CompileTypingTranslator<CopyTranslator>(context()),
        RenameQuantifiedVariablesTranslator<CopyTranslator>(context())
    {
    }

    using CopyTranslatorFacade<CopyTranslator>::copy;
    using BasicCopyTranslator<CopyTranslator>::copy;
    using ConditionTranslator<CopyTranslator>::copy;
    using EffectTranslator<CopyTranslator>::copy;
    using TopLevelTranslator<CopyTranslator>::copy;
};

// Defined in the library: inline definitions would instantiate the whole copy stack in every includer.
std::pair<formalism::DomainView, bool> copy(formalism::DomainView source, CopyTranslator& context);
std::pair<formalism::TaskView, bool> copy(formalism::TaskView source, CopyTranslator& context);

}  // namespace loki::semantic::detail

#endif
