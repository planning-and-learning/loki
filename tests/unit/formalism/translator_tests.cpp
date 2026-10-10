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

#include <gtest/gtest.h>
#include <loki/semantic/errors.hpp>
#include <loki/semantic/options.hpp>
#include <loki/semantic/parser.hpp>
#include <loki/semantic/translator.hpp>
#include <loki/semantic/translator/canonical_copy_translator.hpp>
#include <loki/semantic/translator/copy_translator.hpp>
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
    auto source = formalism::Repository(101);
    auto type_data = ygg::Data<formalism::Type>(cista::offset::string("type"));
    const auto type = formalism::insert(source, type_data).first;
    auto domain_data = ygg::Data<formalism::Domain>();
    domain_data.name = "domain";
    domain_data.types.push_back(type.get_index());
    const auto domain = formalism::insert(source, domain_data).first;
    auto task_data = ygg::Data<formalism::Task>();
    task_data.name = "task";
    task_data.domain = domain.get_index();
    const auto task = formalism::insert(source, task_data).first;

    auto destination = std::make_shared<semantic::detail::TranslationStorage>(102);
    auto occupied_data = ygg::Data<formalism::Type>(cista::offset::string("occupied"));
    formalism::insert(destination->repository, occupied_data);
    auto context = semantic::detail::CanonicalCopyTranslator(destination);
    const auto [copied_domain, domain_inserted] = semantic::detail::copy(domain, context);
    EXPECT_TRUE(domain_inserted);
    EXPECT_EQ(&copied_domain.get_context(), &destination->repository);
    ASSERT_EQ(copied_domain.get_types().size(), 1);
    EXPECT_NE(copied_domain.get_types()[0].get_index(), type.get_index());
    EXPECT_EQ(std::string_view(copied_domain.get_types()[0].get_name()), "type");
    const auto [copied_task, task_inserted] = semantic::detail::copy(task, context);
    EXPECT_TRUE(task_inserted);
    EXPECT_EQ(copied_task.get_domain(), copied_domain);
    EXPECT_EQ(semantic::detail::copy(domain, context), std::make_pair(copied_domain, false));
    EXPECT_EQ(semantic::detail::copy(task, context), std::make_pair(copied_task, false));
    EXPECT_EQ(semantic::detail::copy(copied_domain, context), std::make_pair(copied_domain, false));
    EXPECT_EQ(semantic::detail::copy(copied_task, context), std::make_pair(copied_task, false));

    auto phase_storage = std::make_shared<semantic::detail::TranslationStorage>(103);
    auto phase = semantic::detail::CopyTranslator(phase_storage, false);
    const auto [phase_domain, phase_inserted] = semantic::detail::copy(domain, phase);
    EXPECT_TRUE(phase_inserted);
    EXPECT_EQ(semantic::detail::copy(domain, phase), std::make_pair(phase_domain, false));

    source.clear();
    EXPECT_EQ(std::string_view(copied_task.get_name()), "task");
    EXPECT_EQ(std::string_view(copied_task.get_domain().get_types()[0].get_name()), "type");
}

TEST(LokiTests, GeneratedUniversalPredicateKeepsNumericFreeVariables)
{
    auto parser = semantic::Parser(fixture_path("numeric-universal"));
    const auto translation = semantic::translate(parser.get_domain());
    const auto domain = translation.get_translated_domain();

    auto found = false;
    for (auto predicate : domain.get_predicates())
    {
        const auto name = std::string(predicate.get_name());
        if (!name.starts_with("loki-universal-"))
            continue;

        found = true;
        EXPECT_EQ(predicate.get_parameters().size(), std::size_t { 1 });
    }
    EXPECT_TRUE(found);
}

TEST(LokiTests, CompileConditionalEffectsSplitsActions)
{
    auto parser = semantic::Parser(fixture_path("conditional-multiply"));
    auto options = semantic::TranslatorOptions {};
    options.compile_conditional_effects = true;

    const auto translation = semantic::translate(parser.get_domain(), options);
    const auto domain = translation.get_translated_domain();

    EXPECT_FALSE(has_requirement_kind(domain, formalism::RequirementKind::ConditionalEffects));

    ASSERT_EQ(domain.get_actions().size(), std::size_t { 4 });
    for (auto action : domain.get_actions())
    {
        EXPECT_TRUE(std::string_view(action.get_name()).starts_with("a_"));
        EXPECT_EQ(std::string_view(action.get_original_name()), "a");
        ASSERT_TRUE(action.get_precondition().has_value());
        EXPECT_EQ(count_condition_nodes<formalism::Condition<formalism::LiteralTag>>(action.get_precondition().value()), std::size_t { 3 });
        if (const auto effect = action.get_effect())
        {
            EXPECT_EQ(count_effect_nodes<formalism::Effect<formalism::WhenTag>>(effect.value()), std::size_t { 0 });
        }
    }
}

TEST(LokiTests, CompileConditionalEffectsThrowsOnOverflow)
{
    auto parser = semantic::Parser(fixture_path("conditional-overflow"));
    auto options = semantic::TranslatorOptions {};
    options.compile_conditional_effects = true;

    EXPECT_THROW(semantic::translate(parser.get_domain(), options), semantic::SemanticError);
}

TEST(LokiTests, ExistentialConditionalEffectBecomesUniversalEffect)
{
    auto parser = semantic::Parser(fixture_path("conditional-exists"));
    const auto translation = semantic::translate(parser.get_domain());
    const auto domain = translation.get_translated_domain();

    for (auto predicate : domain.get_predicates())
        EXPECT_FALSE(std::string_view(predicate.get_name()).starts_with("_condition_"));

    ASSERT_FALSE(domain.get_actions().empty());
    const auto action = domain.get_actions().front();
    ASSERT_TRUE(action.get_effect().has_value());
    EXPECT_EQ(count_effect_nodes<formalism::Effect<formalism::ForallTag>>(action.get_effect().value()), std::size_t { 1 });
    EXPECT_EQ(count_effect_nodes<formalism::Effect<formalism::WhenTag>>(action.get_effect().value()), std::size_t { 1 });

    const auto effect_forall = action.get_effect().value().get_variant().get<ygg::Index<formalism::Effect<formalism::ForallTag>>>();
    const auto effect_when = effect_forall.get_effect().get_variant().get<ygg::Index<formalism::Effect<formalism::WhenTag>>>();
    EXPECT_EQ(count_condition_nodes<formalism::Condition<formalism::ExistsTag>>(effect_when.get_condition()), std::size_t { 0 });
}

TEST(LokiTests, DnfDistributesUniversalOverDisjunction)
{
    auto parser = semantic::Parser(fixture_path("dnf-forall"));
    auto storage = std::make_shared<semantic::detail::TranslationStorage>(1);
    auto translator = semantic::detail::CopyTranslator(storage, true, semantic::TranslationPhase::ToDisjunctiveNormalForm);
    const auto domain = semantic::detail::copy(parser.get_domain(), translator).first;

    ASSERT_FALSE(domain.get_actions().empty());
    const auto action = domain.get_actions().front();
    ASSERT_TRUE(action.get_precondition().has_value());

    const auto condition_or = action.get_precondition().value().get_variant().get<ygg::Index<formalism::Condition<formalism::OrTag>>>();
    ASSERT_EQ(condition_or.get_conditions().size(), std::size_t { 2 });
    for (auto condition : condition_or.get_conditions())
    {
        ygg::visit(
            [](const auto& node)
            {
                using Node = std::decay_t<decltype(node)>;
                EXPECT_TRUE((std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::ForallTag>>>) );
            },
            condition.get_variant());
    }
}

TEST(LokiTests, UntypedUniversalEffectKeepsEmptyGuardWhen)
{
    auto parser = semantic::Parser(fixture_path("untyped-universal-effect"));
    const auto translation = semantic::translate(parser.get_domain());
    const auto domain = translation.get_translated_domain();

    ASSERT_FALSE(domain.get_actions().empty());
    const auto action = domain.get_actions().front();
    ASSERT_TRUE(action.get_effect().has_value());
    EXPECT_EQ(count_effect_nodes<formalism::Effect<formalism::ForallTag>>(action.get_effect().value()), std::size_t { 1 });
    EXPECT_EQ(count_effect_nodes<formalism::Effect<formalism::WhenTag>>(action.get_effect().value()), std::size_t { 1 });
}

TEST(LokiTests, KeepTypingPreservesPersistentParameters)
{
    auto parser = semantic::Parser(fixture_path("typed-signatures"));
    const auto translation = semantic::translate(parser.get_domain(), semantic::TranslatorOptions { .compile_typing = false });
    const auto domain = translation.get_translated_domain();

    ASSERT_FALSE(domain.get_types().empty());

    EXPECT_TRUE(has_requirement_kind(domain, formalism::RequirementKind::Typing));

    for (auto predicate : domain.get_predicates())
    {
        if (std::string(predicate.get_name()) == "thing")
            continue;
        for (auto parameter : predicate.get_parameters())
            EXPECT_FALSE(parameter.get_types().empty());
    }

    for (auto function : domain.get_functions())
    {
        for (auto parameter : function.get_parameters())
            EXPECT_FALSE(parameter.get_types().empty());
    }

    for (auto action : domain.get_actions())
    {
        for (auto parameter : action.get_parameters())
            EXPECT_FALSE(parameter.get_types().empty());
        ASSERT_TRUE(action.get_precondition().has_value());
        EXPECT_GT(count_condition_nodes<formalism::Condition<formalism::LiteralTag>>(action.get_precondition().value()), std::size_t { 1 });
    }
}

TEST(LokiTests, RemoveTypingStripsPersistentParameters)
{
    auto parser = semantic::Parser(fixture_path("typed-signatures"));
    const auto translation = semantic::translate(parser.get_domain(), semantic::TranslatorOptions { .compile_typing = true });
    const auto domain = translation.get_translated_domain();
    EXPECT_TRUE(domain.get_types().empty());

    for (auto predicate : domain.get_predicates())
    {
        for (auto parameter : predicate.get_parameters())
            EXPECT_TRUE(parameter.get_types().empty());
    }

    for (auto function : domain.get_functions())
    {
        for (auto parameter : function.get_parameters())
            EXPECT_TRUE(parameter.get_types().empty());
    }

    for (auto action : domain.get_actions())
    {
        for (auto parameter : action.get_parameters())
            EXPECT_TRUE(parameter.get_types().empty());
    }
}

TEST(LokiTests, RenameQuantifiedVariablesSeparatesNestedBinders)
{
    auto parser = semantic::Parser(fixture_path("variable-renaming"));
    auto storage = std::make_shared<semantic::detail::TranslationStorage>(1);
    auto translator = semantic::detail::CopyTranslator(storage, true, semantic::TranslationPhase::RenameQuantifiedVariables);
    const auto domain = semantic::detail::copy(parser.get_domain(), translator).first;
    const auto variable_name = [](formalism::ParameterView parameter) { return std::string(parameter.get_variable().get_name()); };

    ASSERT_FALSE(domain.get_actions().empty());
    const auto action = domain.get_actions().front();
    ASSERT_EQ(action.get_parameters().size(), std::size_t { 1 });
    EXPECT_EQ(variable_name(action.get_parameters().front()), "?x");

    ASSERT_TRUE(action.get_precondition().has_value());
    const auto exists = action.get_precondition().value().get_variant().get<ygg::Index<formalism::Condition<formalism::ExistsTag>>>();
    ASSERT_EQ(exists.get_parameters().size(), std::size_t { 1 });
    EXPECT_EQ(variable_name(exists.get_parameters().front()), "?x_0");

    const auto conjunction = exists.get_condition().get_variant().get<ygg::Index<formalism::Condition<formalism::AndTag>>>();
    auto checked_forall = false;
    for (auto child : conjunction.get_conditions())
    {
        ygg::visit(
            [&](const auto& node)
            {
                using Node = std::decay_t<decltype(node)>;
                if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::ForallTag>>>)
                {
                    checked_forall = true;
                    ASSERT_EQ(node.get_parameters().size(), std::size_t { 1 });
                    EXPECT_EQ(variable_name(node.get_parameters().front()), "?x_1");
                }
            },
            child.get_variant());
    }
    EXPECT_TRUE(checked_forall);

    ASSERT_TRUE(action.get_effect().has_value());
    const auto effect_forall = action.get_effect().value().get_variant().get<ygg::Index<formalism::Effect<formalism::ForallTag>>>();
    ASSERT_EQ(effect_forall.get_parameters().size(), std::size_t { 1 });
    EXPECT_EQ(variable_name(effect_forall.get_parameters().front()), "?x_2");
}

}  // namespace loki::tests
