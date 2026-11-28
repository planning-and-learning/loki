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
#include "../formalism_utils.hpp"
#include "semantic/translator/canonical_copy_translator.hpp"
#include "semantic/translator/copy_translator.hpp"

#include <gtest/gtest.h>
#include <loki/formalism/repository.hpp>
#include <loki/semantic/errors.hpp>
#include <loki/semantic/options.hpp>
#include <loki/semantic/parser.hpp>
#include <loki/semantic/translator.hpp>
#include <memory>
#include <string>
#include <string_view>

namespace loki::tests
{
namespace
{
template<typename V>
concept CanonicalCopyRoot = requires(V source, semantic::detail::CanonicalCopyTranslator& context) { semantic::detail::copy(source, context); };
static_assert(CanonicalCopyRoot<formalism::DomainView>);
static_assert(CanonicalCopyRoot<formalism::TaskView>);
static_assert(!CanonicalCopyRoot<formalism::TypeView>);
}

TEST(LokiTests, CopyRootsRetainCanonicalValuesAndReportInsertion)
{
    auto source = formalism::Repository();
    auto type_data = urd::data<formalism::Type>(urd::flat_string("type"));
    const auto type = source.insert(type_data).first;
    auto domain_data = urd::data<formalism::Domain>();
    domain_data.name = "domain";
    domain_data.types.push_back(type.index());
    const auto domain = source.insert(domain_data).first;
    auto task_data = urd::data<formalism::Task>();
    task_data.name = "task";
    task_data.domain = domain.index();
    const auto task = source.insert(task_data).first;

    auto destination = std::make_shared<semantic::detail::TranslationStorage>();
    auto occupied_data = urd::data<formalism::Type>(urd::flat_string("occupied"));
    static_cast<void>(destination->repository.insert(occupied_data));
    auto context = semantic::detail::CanonicalCopyTranslator(destination);
    const auto [copied_domain, domain_inserted] = semantic::detail::copy(domain, context);
    EXPECT_TRUE(domain_inserted);
    EXPECT_EQ(&copied_domain.context(), &destination->repository);
    ASSERT_EQ(copied_domain.types().size(), 1);
    EXPECT_NE(copied_domain.types()[0].index(), type.index());
    EXPECT_EQ(std::string_view(copied_domain.types()[0].name()), "type");
    const auto [copied_task, task_inserted] = semantic::detail::copy(task, context);
    EXPECT_TRUE(task_inserted);
    EXPECT_EQ(copied_task.domain(), copied_domain);
    EXPECT_EQ(semantic::detail::copy(domain, context), std::make_pair(copied_domain, false));
    EXPECT_EQ(semantic::detail::copy(task, context), std::make_pair(copied_task, false));
    EXPECT_EQ(semantic::detail::copy(copied_domain, context), std::make_pair(copied_domain, false));
    EXPECT_EQ(semantic::detail::copy(copied_task, context), std::make_pair(copied_task, false));

    auto phase_storage = std::make_shared<semantic::detail::TranslationStorage>();
    auto phase = semantic::detail::CopyTranslator(phase_storage, false);
    const auto [phase_domain, phase_inserted] = semantic::detail::copy(domain, phase);
    EXPECT_TRUE(phase_inserted);
    EXPECT_EQ(semantic::detail::copy(domain, phase), std::make_pair(phase_domain, false));

    source.clear_all();
    EXPECT_EQ(std::string_view(copied_task.name()), "task");
    EXPECT_EQ(std::string_view(copied_task.domain().types()[0].name()), "type");
}

TEST(LokiTests, GeneratedUniversalPredicateKeepsNumericFreeVariables)
{
    auto parser = semantic::Parser(fixture_path("numeric_universal"));
    const auto translation = semantic::translate(parser.get_domain());
    const auto domain = translation.get_translated_domain();

    auto found = false;
    for (auto predicate : domain.predicates())
    {
        const auto name = std::string(predicate.name());
        if (!name.starts_with("loki-universal-"))
            continue;

        found = true;
        EXPECT_EQ(predicate.parameters().size(), std::size_t { 1 });
    }
    EXPECT_TRUE(found);
}

TEST(LokiTests, CompileConditionalEffectsSplitsActions)
{
    auto parser = semantic::Parser(fixture_path("conditional_multiply"));
    auto options = semantic::TranslatorOptions {};
    options.compile_conditional_effects = true;

    const auto translation = semantic::translate(parser.get_domain(), options);
    const auto domain = translation.get_translated_domain();

    EXPECT_FALSE(has_requirement_kind(domain, formalism::RequirementKind::ConditionalEffects));

    ASSERT_EQ(domain.actions().size(), std::size_t { 4 });
    for (auto action : domain.actions())
    {
        EXPECT_TRUE(std::string_view(action.name()).starts_with("a_"));
        EXPECT_EQ(std::string_view(action.original_name()), "a");
        ASSERT_TRUE(action.precondition().has_value());
        EXPECT_EQ(count_nodes<formalism::Condition<formalism::LiteralTag>>(action.precondition().value()), std::size_t { 3 });
        if (const auto effect = action.effect())
        {
            EXPECT_EQ(count_nodes<formalism::Effect<formalism::WhenTag>>(effect.value()), std::size_t { 0 });
        }
    }
}

TEST(LokiTests, CompileConditionalEffectsThrowsOnOverflow)
{
    auto parser = semantic::Parser(fixture_path("conditional_overflow"));
    auto options = semantic::TranslatorOptions {};
    options.compile_conditional_effects = true;

    EXPECT_THROW(semantic::translate(parser.get_domain(), options), ygg::diagnostics::SemanticError);
}

TEST(LokiTests, ExistentialConditionalEffectBecomesUniversalEffect)
{
    auto parser = semantic::Parser(fixture_path("conditional_exists"));
    const auto translation = semantic::translate(parser.get_domain());
    const auto domain = translation.get_translated_domain();

    for (auto predicate : domain.predicates())
        EXPECT_FALSE(std::string_view(predicate.name()).starts_with("_condition_"));

    ASSERT_FALSE(domain.actions().empty());
    const auto action = domain.actions().front();
    ASSERT_TRUE(action.effect().has_value());
    EXPECT_EQ(count_nodes<formalism::Effect<formalism::ForallTag>>(action.effect().value()), std::size_t { 1 });
    EXPECT_EQ(count_nodes<formalism::Effect<formalism::WhenTag>>(action.effect().value()), std::size_t { 1 });

    const auto effect_forall = effect_node<formalism::ForallTag>(action.effect().value());
    const auto effect_when = std::get<formalism::EntityView<formalism::Effect<formalism::WhenTag>>>(effect_forall.effect());
    EXPECT_EQ(count_nodes<formalism::Condition<formalism::ExistsTag>>(effect_when.condition()), std::size_t { 0 });
}

TEST(LokiTests, DnfDistributesUniversalOverDisjunction)
{
    auto parser = semantic::Parser(fixture_path("dnf_forall"));
    auto storage = std::make_shared<semantic::detail::TranslationStorage>();
    auto translator = semantic::detail::CopyTranslator(storage, true, semantic::TranslationPhase::ToDisjunctiveNormalForm);
    const auto domain = semantic::detail::copy(parser.get_domain(), translator).first;

    ASSERT_FALSE(domain.actions().empty());
    const auto action = domain.actions().front();
    ASSERT_TRUE(action.precondition().has_value());

    const auto condition_or = std::get<formalism::EntityView<formalism::Condition<formalism::OrTag>>>(action.precondition().value());
    ASSERT_EQ(condition_or.conditions().size(), std::size_t { 2 });
    for (auto condition : condition_or.conditions())
    {
        std::visit(
            [](const auto& node)
            {
                using Node = std::decay_t<decltype(node)>;
                EXPECT_TRUE((std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::ForallTag>>>) );
            },
            condition);
    }
}

TEST(LokiTests, UntypedUniversalEffectKeepsEmptyGuardWhen)
{
    auto parser = semantic::Parser(fixture_path("untyped_universal_effect"));
    const auto translation = semantic::translate(parser.get_domain());
    const auto domain = translation.get_translated_domain();

    ASSERT_FALSE(domain.actions().empty());
    const auto action = domain.actions().front();
    ASSERT_TRUE(action.effect().has_value());
    EXPECT_EQ(count_nodes<formalism::Effect<formalism::ForallTag>>(action.effect().value()), std::size_t { 1 });
    EXPECT_EQ(count_nodes<formalism::Effect<formalism::WhenTag>>(action.effect().value()), std::size_t { 1 });
}

TEST(LokiTests, KeepTypingPreservesPersistentParameters)
{
    auto parser = semantic::Parser(fixture_path("typed_signatures"));
    const auto translation = semantic::translate(parser.get_domain(), semantic::TranslatorOptions { .compile_typing = false });
    const auto domain = translation.get_translated_domain();

    ASSERT_FALSE(domain.types().empty());

    EXPECT_TRUE(has_requirement_kind(domain, formalism::RequirementKind::Typing));

    for (auto predicate : domain.predicates())
    {
        if (std::string(predicate.name()) == "thing")
            continue;
        for (auto parameter : predicate.parameters())
            EXPECT_FALSE(parameter.types().empty());
    }

    for (auto function : domain.functions())
    {
        for (auto parameter : function.parameters())
            EXPECT_FALSE(parameter.types().empty());
    }

    for (auto action : domain.actions())
    {
        for (auto parameter : action.parameters())
            EXPECT_FALSE(parameter.types().empty());
        ASSERT_TRUE(action.precondition().has_value());
        EXPECT_GT(count_nodes<formalism::Condition<formalism::LiteralTag>>(action.precondition().value()), std::size_t { 1 });
    }
}

TEST(LokiTests, RemoveTypingStripsPersistentParameters)
{
    auto parser = semantic::Parser(fixture_path("typed_signatures"));
    const auto translation = semantic::translate(parser.get_domain(), semantic::TranslatorOptions { .compile_typing = true });
    const auto domain = translation.get_translated_domain();
    EXPECT_TRUE(domain.types().empty());

    for (auto predicate : domain.predicates())
    {
        for (auto parameter : predicate.parameters())
            EXPECT_TRUE(parameter.types().empty());
    }

    for (auto function : domain.functions())
    {
        for (auto parameter : function.parameters())
            EXPECT_TRUE(parameter.types().empty());
    }

    for (auto action : domain.actions())
    {
        for (auto parameter : action.parameters())
            EXPECT_TRUE(parameter.types().empty());
    }
}

TEST(LokiTests, RenameQuantifiedVariablesSeparatesNestedBinders)
{
    auto parser = semantic::Parser(fixture_path("variable_renaming"));
    auto storage = std::make_shared<semantic::detail::TranslationStorage>();
    auto translator = semantic::detail::CopyTranslator(storage, true, semantic::TranslationPhase::RenameQuantifiedVariables);
    const auto domain = semantic::detail::copy(parser.get_domain(), translator).first;
    const auto variable_name = [](formalism::ParameterView parameter) { return std::string(parameter.variable().name()); };

    ASSERT_FALSE(domain.actions().empty());
    const auto action = domain.actions().front();
    ASSERT_EQ(action.parameters().size(), std::size_t { 1 });
    EXPECT_EQ(variable_name(action.parameters().front()), "?x");

    ASSERT_TRUE(action.precondition().has_value());
    const auto exists = std::get<formalism::EntityView<formalism::Condition<formalism::ExistsTag>>>(action.precondition().value());
    ASSERT_EQ(exists.parameters().size(), std::size_t { 1 });
    EXPECT_EQ(variable_name(exists.parameters().front()), "?x_0");

    const auto conjunction = std::get<formalism::EntityView<formalism::Condition<formalism::AndTag>>>(exists.condition());
    auto checked_forall = false;
    for (auto child : conjunction.conditions())
    {
        std::visit(
            [&](const auto& node)
            {
                using Node = std::decay_t<decltype(node)>;
                if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::ForallTag>>>)
                {
                    checked_forall = true;
                    ASSERT_EQ(node.parameters().size(), std::size_t { 1 });
                    EXPECT_EQ(variable_name(node.parameters().front()), "?x_1");
                }
            },
            child);
    }
    EXPECT_TRUE(checked_forall);

    ASSERT_TRUE(action.effect().has_value());
    const auto effect_forall = effect_node<formalism::ForallTag>(action.effect().value());
    ASSERT_EQ(effect_forall.parameters().size(), std::size_t { 1 });
    EXPECT_EQ(variable_name(effect_forall.parameters().front()), "?x_2");
}

}  // namespace loki::tests
