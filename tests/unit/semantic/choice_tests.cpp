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

#include "../benchmark_utils.hpp"

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <loki/formalism/choices.hpp>
#include <loki/semantic/options.hpp>
#include <loki/semantic/parser.hpp>
#include <loki/semantic/translator.hpp>
#include <string>
#include <string_view>
#include <urd/repr/maps/unordered_flat_map.hpp>
#include <urd/store.hpp>
#include <yggdrasil/formatting/cista_formatters.hpp>

namespace loki::tests
{
namespace
{

/// Without action costs, which would add (increase (total-cost) 1) to every effect.
constexpr auto parser_options = semantic::ParserOptions { .add_action_costs = false };

/// The effects of the translated domain's actions, as text by action name.
urd::Store<urd::map<std::string, std::string>> translated_effects(std::string_view fixture,
                                                                  const semantic::TranslatorOptions& options = { .compile_typing = false })
{
    semantic::Parser parser(fixture_path(std::string(fixture)), parser_options);
    const auto translation = semantic::translate(parser.get_domain(), options);
    auto result = urd::Store<urd::map<std::string, std::string>> {};
    for (auto action : translation.get_translated_domain().get_actions())
        result[std::string(action.get_name())] = action.get_effect() ? fmt::format("{}", action.get_effect().value()) : "";
    return result;
}

}  // namespace

TEST(LokiSemanticChoices, KeepsEqualChoicesAsIndependentDraws)
{
    const auto effects = translated_effects("choice_duplicates");
    EXPECT_EQ(effects.at("flip-twice"), "(and (oneof (p) (q)) (oneof (p) (q)))");
}

TEST(LokiSemanticChoices, AddsTheProbabilitiesOfEqualOutcomes)
{
    const auto effects = translated_effects("choice_duplicates");
    EXPECT_EQ(effects.at("sample"), "(p)");
}

TEST(LokiSemanticChoices, GuardsNonConjunctiveConditionsOfChoicesWithDerivedPredicates)
{
    const auto options = semantic::TranslatorOptions { .compile_typing = false };
    semantic::Parser parser(fixture_path("choice_conditions"), parser_options);
    const auto translation = semantic::translate(parser.get_domain(), options);
    const auto domain = translation.get_translated_domain();

    auto effects = urd::Store<urd::map<std::string, std::string>> {};
    for (auto action : domain.get_actions())
        effects[std::string(action.get_name())] = fmt::format("{}", action.get_effect().value());
    EXPECT_EQ(effects.at("disjunctive"), "(when (loki-choice-0 ?x) (oneof (p ?x) (q ?x)))");
    EXPECT_EQ(effects.at("existential"), "(when (loki-choice-1) (oneof (not (r)) (r)))");
    // A deterministic effect is split per disjunct as before.
    EXPECT_EQ(effects.at("deterministic"), "(and (when (a ?x) (p ?x)) (when (b ?x) (p ?x)))");

    auto axioms = urd::Store<urd::map<std::string, std::size_t>> {};
    for (auto axiom : domain.get_axioms())
        ++axioms[std::string(axiom.get_head().get_atom().get_predicate().get_name())];
    EXPECT_EQ(axioms.at("loki-choice-0"), std::size_t { 2 });
    EXPECT_EQ(axioms.at("loki-choice-1"), std::size_t { 1 });
}

TEST(LokiSemanticChoices, KeepsForallOutsideTheChoice)
{
    const auto effects = translated_effects("choice_forall");
    // The type conditions of a forall effect are its own conditional effect's.
    EXPECT_EQ(effects.at("flip-all"),
              "(and (forall (?x - item) (when (and (item ?x) (object ?x)) (oneof (a ?x) (b ?x)))) "
              "(forall (?x - item) (when (and (item ?x) (object ?x)) (p ?x))))");
}

TEST(LokiSemanticChoices, FlattensNestedChoices)
{
    const auto effects = translated_effects("choice_nested");
    EXPECT_EQ(effects.at("nested-oneof"), "(oneof (r) (s) (t))");
    // (r) 0.25 + 0.2, (s) 0.25, and the remaining probability mass an explicit empty outcome.
    EXPECT_TRUE(effects.at("nested-probabilistic").starts_with("(probabilistic 0.25 (s) 0.3"));
    EXPECT_TRUE(effects.at("nested-probabilistic").ends_with(" (and) 0.45 (r))"));
    // A choice inside an outcome stays there, per item.
    EXPECT_EQ(effects.at("choice-in-outcome"),
              "(probabilistic 0.2 (forall (?x - item) (when (and (a ?x) (item ?x) (object ?x)) (probabilistic 0.5 (p ?x) 0.5 (q ?x)))) 0.8 (r))");
}

TEST(LokiSemanticChoices, CompilesConditionalEffectsInsideOutcomes)
{
    const auto effects =
        translated_effects("choice_compile_conditional", semantic::TranslatorOptions { .compile_typing = false, .compile_conditional_effects = true });
    ASSERT_EQ(effects.size(), std::size_t { 2 });
    auto texts = urd::Store<urd::map<std::string, std::size_t>> {};
    for (const auto& [name, effect] : effects)
        ++texts[effect];
    EXPECT_EQ(texts.at("(oneof (p) (q))"), std::size_t { 1 });
    EXPECT_EQ(texts.at("(oneof (and) (q))"), std::size_t { 1 });
}

TEST(LokiSemanticChoices, DetectsChoices)
{
    semantic::Parser parser(fixture_path("choice_nested"), parser_options);
    for (auto action : parser.get_domain().get_actions())
        EXPECT_TRUE(formalism::contains_choice(action.get_effect().value()));
}

}  // namespace loki::tests
