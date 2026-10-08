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

#include "loki/semantic/translator/basic_copy_translator.hpp"
#include "loki/semantic/translator/compile_conditional_effects_translator.hpp"
#include "loki/semantic/translator/compile_typing_translator.hpp"
#include "loki/semantic/translator/condition_analysis_translator.hpp"
#include "loki/semantic/translator/condition_builder_translator.hpp"
#include "loki/semantic/translator/condition_translator.hpp"
#include "loki/semantic/translator/copy_translator_facade.hpp"
#include "loki/semantic/translator/effect_translator.hpp"
#include "loki/semantic/translator/materialize_equality_translator.hpp"
#include "loki/semantic/translator/move_existential_quantifiers_translator.hpp"
#include "loki/semantic/translator/normalize_arithmetic_expressions_translator.hpp"
#include "loki/semantic/translator/remove_universal_quantifiers_translator.hpp"
#include "loki/semantic/translator/rename_quantified_variables_translator.hpp"
#include "loki/semantic/translator/simplify_goal_translator.hpp"
#include "loki/semantic/translator/split_disjunctive_conditions_translator.hpp"
#include "loki/semantic/translator/to_disjunctive_normal_form_translator.hpp"
#include "loki/semantic/translator/to_effect_normal_form_translator.hpp"
#include "loki/semantic/translator/to_negation_normal_form_translator.hpp"
#include "loki/semantic/translator/top_level_translator.hpp"

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
    using CopyTranslatorFacade<CopyTranslator>::used_predicate_names;
    using CopyTranslatorFacade<CopyTranslator>::next_generated_predicate_name;

    using SplitDisjunctiveConditionsTranslator<CopyTranslator>::split_disjunctive_actions;
    using SplitDisjunctiveConditionsTranslator<CopyTranslator>::split_disjunctive_axioms;

    using CompileConditionalEffectsTranslator<CopyTranslator>::compile_conditional_effect_actions;

    using BasicCopyTranslator<CopyTranslator>::copy;
    using BasicCopyTranslator<CopyTranslator>::copy_list;
    using BasicCopyTranslator<CopyTranslator>::increment_quantifications;
    using BasicCopyTranslator<CopyTranslator>::enter_scope;
    using BasicCopyTranslator<CopyTranslator>::leave_scope;
    using BasicCopyTranslator<CopyTranslator>::append_generated_domain_objects;
    using BasicCopyTranslator<CopyTranslator>::strip_requirement;
    using BasicCopyTranslator<CopyTranslator>::strip_typing_requirement;
    using ConditionTranslator<CopyTranslator>::copy;

    using ConditionAnalysisTranslator<CopyTranslator>::term_from_variable;
    using ConditionAnalysisTranslator<CopyTranslator>::collect_free_variables;
    using ConditionAnalysisTranslator<CopyTranslator>::free_parameters_in_scope;

    using ConditionBuilderTranslator<CopyTranslator>::wrap_condition;
    using ConditionBuilderTranslator<CopyTranslator>::as_or;
    using ConditionBuilderTranslator<CopyTranslator>::flatten_condition;
    using ConditionBuilderTranslator<CopyTranslator>::make_conjunction;
    using ConditionBuilderTranslator<CopyTranslator>::make_disjunction;

    using ToDisjunctiveNormalFormTranslator<CopyTranslator>::to_dnf;
    using ToDisjunctiveNormalFormTranslator<CopyTranslator>::to_dnf_node;

    using ToNegationNormalFormTranslator<CopyTranslator>::negate_comparator;
    using ToNegationNormalFormTranslator<CopyTranslator>::negate_condition;
    using ToNegationNormalFormTranslator<CopyTranslator>::negate_condition_node;
    using ToNegationNormalFormTranslator<CopyTranslator>::copy_condition_node;

    using RemoveUniversalQuantifiersTranslator<CopyTranslator>::make_generated_axiom_condition;
    using RemoveUniversalQuantifiersTranslator<CopyTranslator>::remove_universal_quantifiers;
    using RemoveUniversalQuantifiersTranslator<CopyTranslator>::remove_universal_quantifiers_node;

    using MoveExistentialQuantifiersTranslator<CopyTranslator>::as_exists;
    using MoveExistentialQuantifiersTranslator<CopyTranslator>::move_existentials;
    using MoveExistentialQuantifiersTranslator<CopyTranslator>::move_existentials_node;
    using MoveExistentialQuantifiersTranslator<CopyTranslator>::lift_top_level_exists;

    using SimplifyGoalTranslator<CopyTranslator>::make_generated_goal_condition;
    using SimplifyGoalTranslator<CopyTranslator>::simplify_goal_condition;
    using SimplifyGoalTranslator<CopyTranslator>::simplify_goal_condition_node;

    using EffectTranslator<CopyTranslator>::copy;

    using MaterializeEqualityTranslator<CopyTranslator>::has_requirement;
    using MaterializeEqualityTranslator<CopyTranslator>::equality_required;
    using MaterializeEqualityTranslator<CopyTranslator>::find_domain_equality_predicate;
    using MaterializeEqualityTranslator<CopyTranslator>::equality_literal;
    using MaterializeEqualityTranslator<CopyTranslator>::domain_uses_equality;
    using MaterializeEqualityTranslator<CopyTranslator>::add_equality_predicate_to_domain;
    using MaterializeEqualityTranslator<CopyTranslator>::materialize_equality;

    using ToEffectNormalFormTranslator<CopyTranslator>::wrap_effect;
    using ToEffectNormalFormTranslator<CopyTranslator>::as_effect;
    using ToEffectNormalFormTranslator<CopyTranslator>::normalize_effect;
    using ToEffectNormalFormTranslator<CopyTranslator>::normalize_effect_node;

    using NormalizeArithmeticExpressionsTranslator<CopyTranslator>::normalize_arithmetic_expression;

    using TopLevelTranslator<CopyTranslator>::copy;

    using CompileTypingTranslator<CopyTranslator>::collect_type_hierarchy;
    using CompileTypingTranslator<CopyTranslator>::copy_type_hierarchy;
    using CompileTypingTranslator<CopyTranslator>::copy_parameters_without_types;
    using CompileTypingTranslator<CopyTranslator>::type_predicate;
    using CompileTypingTranslator<CopyTranslator>::type_literal;
    using CompileTypingTranslator<CopyTranslator>::type_condition;
    using CompileTypingTranslator<CopyTranslator>::type_conditions_for_parameters;
    using CompileTypingTranslator<CopyTranslator>::prepend_type_conditions;
    using CompileTypingTranslator<CopyTranslator>::compile_typing_to_domain;
    using CompileTypingTranslator<CopyTranslator>::add_type_literals_for_object;
    using CompileTypingTranslator<CopyTranslator>::initialize_type_literals;

    using RenameQuantifiedVariablesTranslator<CopyTranslator>::enter_variable_scope;
    using RenameQuantifiedVariablesTranslator<CopyTranslator>::leave_variable_scope;
    using RenameQuantifiedVariablesTranslator<CopyTranslator>::rename_parameter;
    using RenameQuantifiedVariablesTranslator<CopyTranslator>::rename_parameters;
    using RenameQuantifiedVariablesTranslator<CopyTranslator>::rename_variables;
    using RenameQuantifiedVariablesTranslator<CopyTranslator>::rename_action_variables;
    using RenameQuantifiedVariablesTranslator<CopyTranslator>::rename_axiom_variables;
};

inline auto copy(formalism::DomainView source, CopyTranslator& context) { return context.copy(source); }
inline auto copy(formalism::TaskView source, CopyTranslator& context) { return context.copy(source); }

}  // namespace loki::semantic::detail

#endif
