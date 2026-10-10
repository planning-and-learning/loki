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

#include <algorithm>
#include <gtest/gtest.h>
#include <initializer_list>
#include <loki/formalism/repository.hpp>
#include <loki/formalism/formatter.hpp>
#include <loki/semantic/errors.hpp>
#include <loki/semantic/options.hpp>
#include <loki/semantic/parser.hpp>
#include <loki/semantic/translator.hpp>
#include <loki/semantic/translator/copy_translator.hpp>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
#include <yggdrasil/containers/associative_containers.hpp>

namespace loki::tests
{
namespace
{

std::optional<std::string> conjunct_variable(formalism::ConditionView condition, std::string_view predicate_name, std::size_t term_index)
{
    if (!condition.get_variant().is<ygg::Index<formalism::Condition<formalism::AndTag>>>())
        return std::nullopt;

    for (auto child : condition.get_variant().get<ygg::Index<formalism::Condition<formalism::AndTag>>>().get_conditions())
    {
        auto result = std::optional<std::string> {};
        ygg::visit(
            [&](const auto& node)
            {
                using Node = std::decay_t<decltype(node)>;
                if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::Condition<formalism::LiteralTag>>>)
                {
                    const auto atom = node.get_literal().get_atom();
                    if (atom.get_predicate().get_name() == predicate_name && atom.get_terms().size() > term_index)
                        result = variable_term_name(atom.get_terms()[term_index]);
                }
            },
            child.get_variant());
        if (result)
            return result;
    }
    return std::nullopt;
}

template<typename T, typename Initialize>
formalism::EntityView<T> intern(formalism::Repository& repository, formalism::Builder& builder, Initialize&& initialize)
{
    auto data = formalism::checkout<T>(builder);
    std::forward<Initialize>(initialize)(*data);
    return formalism::insert(repository, *data).first;
}

template<typename T>
formalism::FunctionExpressionView wrap_expression(formalism::Repository& repository, formalism::Builder& builder, T node)
{
    return intern<formalism::FunctionExpression<>>(repository,
                                                 builder,
                                                 [&](auto& data) { data.variant = ygg::Data<formalism::FunctionExpression<>>::Variant(node.get_index()); });
}

formalism::FunctionExpressionView number_expression(formalism::Repository& repository, formalism::Builder& builder, double value)
{
    return wrap_expression(repository, builder, intern<formalism::FunctionExpression<formalism::NumberTag>>(repository, builder, [&](auto& data) { data.value = value; }));
}

formalism::FunctionExpressionView binary_expression(formalism::Repository& repository,
                                                    formalism::Builder& builder,
                                                    formalism::BinaryArithmeticOperator op,
                                                    formalism::FunctionExpressionView left,
                                                    formalism::FunctionExpressionView right)
{
    return wrap_expression(repository,
                           builder,
                           intern<formalism::FunctionExpression<formalism::BinaryTag>>(repository,
                                                                       builder,
                                                                       [&](auto& data)
                                                                       {
                                                                           data.op = op;
                                                                           data.left = left.get_index();
                                                                           data.right = right.get_index();
                                                                       }));
}

formalism::FunctionExpressionView multi_expression(formalism::Repository& repository,
                                                   formalism::Builder& builder,
                                                   formalism::MultiArithmeticOperator op,
                                                   std::initializer_list<formalism::FunctionExpressionView> expressions)
{
    return wrap_expression(repository,
                           builder,
                           intern<formalism::FunctionExpression<formalism::MultiTag>>(repository,
                                                                      builder,
                                                                      [&](auto& data)
                                                                      {
                                                                          data.op = op;
                                                                          for (const auto expression : expressions)
                                                                              data.args.push_back(expression.get_index());
                                                                      }));
}

bool contains_binary_add_or_multiply(formalism::FunctionExpressionView expression)
{
    return ygg::visit(
        [](const auto& node) -> bool
        {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::FunctionExpression<formalism::UnaryTag>>>)
            {
                return contains_binary_add_or_multiply(node.get_expression());
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::FunctionExpression<formalism::BinaryTag>>>)
            {
                if (node.get_operator() == formalism::BinaryArithmeticOperator::Add || node.get_operator() == formalism::BinaryArithmeticOperator::Mul)
                    return true;
                return contains_binary_add_or_multiply(node.get_left()) || contains_binary_add_or_multiply(node.get_right());
            }
            else if constexpr (std::is_same_v<Node, formalism::EntityView<formalism::FunctionExpression<formalism::MultiTag>>>)
            {
                for (const auto child : node.get_args())
                    if (contains_binary_add_or_multiply(child))
                        return true;
            }
            return false;
        },
        expression.get_variant());
}

}  // namespace

TEST(LokiSemanticTranslator, RenamesQuantifiedVariablesDeterministically)
{
    semantic::Parser parser(fixture_path("rename-exists"));

    const auto translation = semantic::translate(parser.get_domain());
    const auto translated_domain = translation.get_translated_domain();
    ASSERT_EQ(translated_domain.get_actions().size(), 1);
    const auto action = translated_domain.get_actions()[0];
    ASSERT_EQ(action.get_parameters().size(), 2);
    EXPECT_EQ(std::string(action.get_parameters()[0].get_variable().get_name()), "?x");
    EXPECT_EQ(std::string(action.get_parameters()[1].get_variable().get_name()), "?x_0");
    ASSERT_TRUE(action.get_precondition().has_value());
    EXPECT_FALSE(contains_exists(action.get_precondition().value()));
}

TEST(LokiSemanticTranslator, SeparatesSiblingExistentialsOverSameVariable)
{
    semantic::Parser parser(fixture_path("sibling-exists"));
    const auto translation = semantic::translate(parser.get_domain());
    const auto action = translation.get_translated_domain().get_actions()[0];

    // Both existentials are lifted; the sibling scopes must stay independent bindings.
    ASSERT_EQ(action.get_parameters().size(), 2);
    const auto first = std::string(action.get_parameters()[0].get_variable().get_name());
    const auto second = std::string(action.get_parameters()[1].get_variable().get_name());
    EXPECT_NE(first, second);

    // Each conjunct must reference its own hoisted variable.
    ASSERT_TRUE(action.get_precondition().has_value());
    const auto first_reference = conjunct_variable(action.get_precondition().value(), "a", 0);
    const auto second_reference = conjunct_variable(action.get_precondition().value(), "b", 0);
    ASSERT_TRUE(first_reference.has_value());
    ASSERT_TRUE(second_reference.has_value());
    EXPECT_EQ(*first_reference, first);
    EXPECT_EQ(*second_reference, second);
}

TEST(LokiSemanticTranslator, LiftedParameterAvoidsFreeVariablesWhenRenaming)
{
    // The second sibling collides on ?p; its fresh name must avoid the action parameter ?p_0,
    // which occurs free in the sibling body.
    semantic::Parser parser(fixture_path("lift-capture"));
    const auto translation = semantic::translate(parser.get_domain());
    const auto action = translation.get_translated_domain().get_actions()[0];

    ASSERT_EQ(action.get_parameters().size(), 3);
    auto names = std::vector<std::string> {};
    for (auto parameter : action.get_parameters())
        names.push_back(std::string(parameter.get_variable().get_name()));
    // All parameters distinct, and the renamed binder did not capture the free ?p_0.
    EXPECT_EQ(names.size(), std::size_t { 3 });
    std::sort(names.begin(), names.end());
    EXPECT_EQ(std::unique(names.begin(), names.end()), names.end());
    ASSERT_TRUE(action.get_precondition().has_value());
    const auto free_reference = conjunct_variable(action.get_precondition().value(), "b", 1);
    ASSERT_TRUE(free_reference.has_value());
    EXPECT_EQ(*free_reference, "?p_0");
}

TEST(LokiSemanticTranslator, RenamesBeforeNegationNormalFormOnlyOnce)
{
    semantic::Parser parser(fixture_path("rename-once"));

    const auto translation = semantic::translate(parser.get_domain());
    const auto translated_domain = translation.get_translated_domain();
    ASSERT_EQ(translated_domain.get_axioms().size(), 1);
    const auto axiom = translated_domain.get_axioms()[0];
    ASSERT_EQ(axiom.get_parameters().size(), 1);
    EXPECT_EQ(std::string(axiom.get_parameters()[0].get_variable().get_name()), "?x_0");

    ASSERT_EQ(translated_domain.get_actions().size(), 1);
    const auto action = translated_domain.get_actions()[0];
    ASSERT_EQ(action.get_parameters().size(), 1);
    EXPECT_EQ(std::string(action.get_parameters()[0].get_variable().get_name()), "?x");
}

TEST(LokiSemanticTranslator, LowersNegatedExistsWithoutNegatingInnerCondition)
{
    semantic::Parser parser(fixture_path("negated-exists"));

    const auto translation = semantic::translate(parser.get_domain());
    const auto translated_domain = translation.get_translated_domain();
    ASSERT_EQ(translated_domain.get_axioms().size(), 1);
    const auto axiom = translated_domain.get_axioms()[0];
    EXPECT_TRUE(condition_mentions_predicate(axiom.get_condition(), "p"));
    EXPECT_TRUE(condition_mentions_predicate(axiom.get_condition(), "object"));
    ASSERT_EQ(translated_domain.get_actions().size(), 1);
    const auto action = translated_domain.get_actions()[0];
    ASSERT_TRUE(action.get_precondition().has_value());
    EXPECT_FALSE(contains_exists(action.get_precondition().value()));
}

TEST(LokiSemanticTranslator, GeneratesFreshAxiomsForIdenticalUniversalConditions)
{
    semantic::Parser parser(fixture_path("universal-cache"));

    const auto translation = semantic::translate(parser.get_domain(), semantic::TranslatorOptions { .compile_typing = false });
    const auto translated_domain = translation.get_translated_domain();

    auto generated_predicates = std::size_t {};
    auto generated_parameter_types = ygg::UnorderedSet<std::string> {};
    for (auto predicate : translated_domain.get_predicates())
    {
        if (!std::string(predicate.get_name()).starts_with("loki-universal-"))
            continue;
        ++generated_predicates;
        ASSERT_EQ(predicate.get_parameters().size(), std::size_t { 1 });
        const auto parameter = predicate.get_parameters().front();
        ASSERT_EQ(parameter.get_types().size(), std::size_t { 1 });
        generated_parameter_types.insert(std::string(parameter.get_types().front().get_name()));
    }

    EXPECT_EQ(generated_predicates, 2);
    EXPECT_EQ(generated_parameter_types.size(), std::size_t { 2 });
    EXPECT_TRUE(generated_parameter_types.contains("left"));
    EXPECT_TRUE(generated_parameter_types.contains("right"));
    EXPECT_EQ(translated_domain.get_axioms().size(), 2);
    ASSERT_EQ(translated_domain.get_actions().size(), 2);
    for (auto action : translated_domain.get_actions())
    {
        ASSERT_TRUE(action.get_precondition().has_value());
        EXPECT_FALSE(contains_forall(action.get_precondition().value()));
    }
}

TEST(LokiSemanticTranslator, MovesExistentialPreconditionVariablesToActionParameters)
{
    semantic::Parser parser(fixture_path("exists-precondition"));

    const auto translation = semantic::translate(parser.get_domain());
    const auto translated_domain = translation.get_translated_domain();
    ASSERT_EQ(translated_domain.get_actions().size(), 1);
    const auto action = translated_domain.get_actions()[0];
    EXPECT_EQ(action.get_arity(), 2);
    EXPECT_EQ(action.get_original_arity(), 1);
    EXPECT_EQ(action.get_parameters().size(), 2);
    ASSERT_TRUE(action.get_precondition().has_value());
    EXPECT_FALSE(contains_exists(action.get_precondition().value()));
}

TEST(LokiSemanticTranslator, CompilesTypingWhenEnabled)
{
    semantic::Parser parser(fixture_path("compile-typing"));

    const auto options = semantic::TranslatorOptions { .compile_typing = true };
    const auto translation = semantic::translate(parser.get_domain(), options);
    const auto translated_domain = translation.get_translated_domain();

    EXPECT_TRUE(has_predicate_named(translated_domain, "thing"));
    EXPECT_TRUE(has_predicate_named(translated_domain, "object"));
    EXPECT_FALSE(has_requirement_kind(translated_domain, formalism::RequirementKind::Typing));
    EXPECT_TRUE(translated_domain.get_types().empty());
    ASSERT_EQ(translated_domain.get_actions().size(), 1);
    const auto action = translated_domain.get_actions().front();
    ASSERT_EQ(action.get_parameters().size(), 1);
    EXPECT_TRUE(action.get_parameters().front().get_types().empty());
    ASSERT_TRUE(action.get_precondition().has_value());
    EXPECT_TRUE(condition_mentions_predicate(action.get_precondition().value(), "thing"));

    const auto task = parser.parse_task(fixture_path("compile-typing", "task.pddl"));
    const auto translated_task_result = semantic::translate(task, translation, options);
    const auto translated_task = translated_task_result.get_translated_task();
    EXPECT_GE(count_initial_literals_for_predicate(translated_task.get_initial_literals(), "thing"), 2);
    EXPECT_GE(count_initial_literals_for_predicate(translated_task.get_initial_literals(), "object"), 2);

    const auto thing_predicate = predicate_named(translated_domain, "thing");
    ASSERT_TRUE(thing_predicate.has_value());
    EXPECT_TRUE(initial_literals_use_predicate(translated_task.get_initial_literals(), "thing", *thing_predicate));
}

TEST(LokiSemanticTranslator, ComposesMappingsAfterRecoveringObjectTypeMetadata)
{
    auto original = formalism::Repository(0);
    auto previous = semantic::detail::TranslationStorage(1);
    auto target = semantic::detail::TranslationStorage(2);
    auto builder = formalism::Builder {};
    const auto original_base = intern<formalism::Type>(original, builder, [](auto& data) { data.name = "parent"; });
    const auto original_type = intern<formalism::Type>(original, builder, [&](auto& data)
    {
        data.name = "child";
        data.bases.push_back(original_base.get_index());
    });
    const auto middle_base = intern<formalism::Type>(previous.repository, builder, [](auto& data) { data.name = "parent"; });
    const auto middle_type = intern<formalism::Type>(previous.repository, builder, [&](auto& data)
    {
        data.name = "child";
        data.bases.push_back(middle_base.get_index());
    });
    const auto final_base = intern<formalism::Type>(target.repository, builder, [](auto& data) { data.name = "parent"; });
    const auto object = [&](formalism::Repository& repository, const char* name)
    {
        return intern<formalism::Object>(repository, builder, [&](auto& data) { data.name = name; });
    };
    const auto original_object = object(original, "kept");
    const auto middle_object = object(previous.repository, "kept");
    const auto final_object = object(target.repository, "kept");
    const auto original_removed = object(original, "removed");
    const auto middle_removed = object(previous.repository, "removed");

    previous.types.emplace(original_base, middle_base);
    previous.types.emplace(original_type, middle_type);
    target.types.emplace(middle_base, final_base);
    previous.objects.emplace(original_object, middle_object);
    previous.objects.emplace(original_removed, middle_removed);
    target.objects.emplace(middle_object, final_object);
    previous.object_type_views.emplace(middle_object, std::vector { middle_type });
    previous.object_type_views.emplace(middle_removed, std::vector { middle_base });

    // The child type exists only in metadata; its final mapping must be created before composition.
    semantic::detail::compose_storage_maps_from_previous(target, previous);

    ASSERT_EQ(target.objects.size(), 1);
    EXPECT_EQ(target.objects.at(original_object), final_object);
    EXPECT_FALSE(target.objects.contains(original_removed));
    EXPECT_FALSE(target.objects.contains(middle_object));
    ASSERT_EQ(target.types.size(), 2);
    EXPECT_EQ(target.types.at(original_base), final_base);
    const auto final_type = target.types.at(original_type);
    EXPECT_EQ(final_type.get_context().get_index(), target.repository.get_index());
    EXPECT_EQ(std::string(final_type.get_name()), "child");
    ASSERT_EQ(final_type.get_bases().size(), 1);
    EXPECT_EQ(final_type.get_bases().front(), final_base);
    ASSERT_EQ(target.object_type_views.size(), 1);
    EXPECT_EQ(target.object_type_views.at(final_object), std::vector { final_type });
    EXPECT_EQ(previous.objects.at(original_object), middle_object);
    EXPECT_EQ(previous.types.at(original_type), middle_type);
}

TEST(LokiSemanticTranslator, TranslationStorageRetainsParent)
{
    auto parent = std::make_shared<semantic::detail::TranslationStorage>(1);
    const auto lifetime = std::weak_ptr<const semantic::detail::TranslationStorage>(parent);
    auto builder = formalism::Builder {};
    const auto object = intern<formalism::Object>(parent->repository, builder, [](auto& data) { data.name = "inherited"; });
    auto child = std::make_shared<semantic::detail::TranslationStorage>(2, parent);

    parent.reset();
    ASSERT_FALSE(lifetime.expired());
    EXPECT_EQ(std::string(ygg::make_view(object.get_index(), child->repository).get_name()), "inherited");

    child.reset();
    EXPECT_TRUE(lifetime.expired());
}

TEST(LokiSemanticTranslator, ProblemTranslationRetainsDomainStorage)
{
    semantic::Parser parser(fixture_path("task-object-types"));
    const auto task = parser.parse_task(fixture_path("task-object-types", "task.pddl"));
    const auto translated_result = [&]
    {
        const auto domain_translation = semantic::translate(parser.get_domain());
        return semantic::translate(task, domain_translation);
    }();
    const auto translated = translated_result.get_translated_task();
    const auto domain = translated.get_domain();

    EXPECT_EQ(std::string(domain.get_name()), "task-object-types");
    ASSERT_EQ(domain.get_actions().size(), 1);
    EXPECT_EQ(std::string(domain.get_actions().front().get_name()), "mark-ready");
    EXPECT_TRUE(has_initial_unary_literal(translated.get_initial_literals(), "bread-portion", "bread1"));
    EXPECT_TRUE(has_initial_unary_literal(translated.get_initial_literals(), "content-portion", "content1"));
}

TEST(LokiSemanticTranslator, PreservesTaskObjectTypesAfterDomainCanonicalization)
{
    semantic::Parser parser(fixture_path("task-object-types"));
    const auto domain_translation = semantic::translate(parser.get_domain());
    const auto translated_result = semantic::translate(parser.parse_task(fixture_path("task-object-types", "task.pddl")), domain_translation);
    const auto translated = translated_result.get_translated_task();

    EXPECT_TRUE(has_initial_unary_literal(translated.get_initial_literals(), "bread-portion", "bread1"));
    EXPECT_TRUE(has_initial_unary_literal(translated.get_initial_literals(), "content-portion", "content1"));
    EXPECT_TRUE(has_initial_unary_literal(translated.get_initial_literals(), "place", "table1"));
    EXPECT_FALSE(has_initial_unary_literal(translated.get_initial_literals(), "place", "bread1"));
}

TEST(LokiSemanticTranslator, AddsEqualityPredicateWhenAdlDomainUsesEquality)
{
    semantic::Parser parser(fixture_path("adl-equality"));
    const auto options = semantic::TranslatorOptions { .materialize_equality = true };
    const auto translation = semantic::translate(parser.get_domain(), options);
    EXPECT_TRUE(has_equality_predicate(translation.get_translated_domain()));

    const auto translated_result = semantic::translate(parser.parse_task(fixture_path("adl-equality", "task.pddl")), translation, options);
    const auto translated = translated_result.get_translated_task();
    EXPECT_TRUE(has_equality_predicate(translated.get_domain()));
    EXPECT_EQ(count_equality_literals(translated.get_initial_literals()), count_unique_object_names(translation.get_translated_domain(), translated));
}

TEST(LokiSemanticTranslator, InitializesEqualityForConstantsAndTaskObjects)
{
    semantic::Parser parser(fixture_path("equality-constants"));

    const auto options = semantic::TranslatorOptions { .materialize_equality = true };
    const auto translation = semantic::translate(parser.get_domain(), options);
    EXPECT_TRUE(has_equality_predicate(translation.get_translated_domain()));

    const auto task = parser.parse_task(fixture_path("equality-constants", "task.pddl"));
    const auto translated_result = semantic::translate(task, translation, options);
    const auto translated = translated_result.get_translated_task();

    EXPECT_TRUE(has_equality_predicate(translated.get_domain()));
    EXPECT_EQ(count_equality_literals(translated.get_initial_literals()), count_unique_object_names(translation.get_translated_domain(), translated));
    EXPECT_EQ(&translated.get_domain().get_context().get_root(), &translation.get_translated_domain().get_context().get_root());
}

TEST(LokiSemanticTranslator, SkipsEqualityInitializationWhenDisabled)
{
    semantic::Parser parser(fixture_path("equality-negated"));

    auto options = semantic::TranslatorOptions {};
    options.materialize_equality = false;
    const auto translation = semantic::translate(parser.get_domain(), options);
    EXPECT_FALSE(has_equality_predicate(translation.get_translated_domain()));

    const auto translated_result = semantic::translate(parser.parse_task(fixture_path("equality-negated", "task.pddl")), translation, options);
    const auto translated = translated_result.get_translated_task();
    EXPECT_FALSE(has_equality_predicate(translated.get_domain()));
    EXPECT_EQ(count_equality_literals(translated.get_initial_literals()), 0);
}

TEST(LokiSemanticTranslator, RejectsTaskFromDifferentDomainTranslationRepository)
{
    auto first_parser = semantic::Parser(fixture_path("nnf"));
    auto second_parser = semantic::Parser(fixture_path("repo-mismatch"));

    const auto first_translation = semantic::translate(first_parser.get_domain());
    const auto second_task = second_parser.parse_task(fixture_path("repo-mismatch", "task.pddl"));

    try
    {
        static_cast<void>(semantic::translate(second_task, first_translation));
        FAIL() << "Expected mismatched domain diagnostic";
    }
    catch (const semantic::MismatchedDomainError& error)
    {
        const auto message = std::string(error.what());
        EXPECT_NE(message.find("nnf"), std::string::npos);
        EXPECT_NE(message.find("second-translation-domain"), std::string::npos);
    }
}

TEST(LokiSemanticTranslator, TaskEqualityRequiresTranslatedDomainEqualityPredicate)
{
    semantic::Parser parser(fixture_path("task-only-equality"));
    const auto options = semantic::TranslatorOptions { .materialize_equality = true };
    const auto translation = semantic::translate(parser.get_domain(), options);
    ASSERT_FALSE(has_equality_predicate(translation.get_translated_domain()));

    const auto task = parser.parse_task(fixture_path("task-only-equality", "task.pddl"));
    EXPECT_THROW(static_cast<void>(semantic::translate(task, translation, options)), semantic::InvalidEqualityError);
}

TEST(LokiSemanticTranslator, RenamesTaskGoalVariablesBeforeGoalSimplificationOnlyOnce)
{
    semantic::Parser parser(fixture_path("task-rename-once"));
    const auto domain_translation = semantic::translate(parser.get_domain());
    const auto translated_result = semantic::translate(parser.parse_task(fixture_path("task-rename-once", "task.pddl")), domain_translation);
    const auto translated = translated_result.get_translated_task();
    ASSERT_EQ(translated.get_axioms().size(), 1);
    const auto axiom = translated.get_axioms().front();
    ASSERT_EQ(axiom.get_parameters().size(), 1);
    EXPECT_EQ(std::string(axiom.get_parameters().front().get_variable().get_name()), "?x");
}

TEST(LokiSemanticTranslator, NormalizesArithmeticExpressionsModuloAcu)
{
    auto repository = formalism::Repository(0);
    auto builder = formalism::Builder {};
    const auto zero = number_expression(repository, builder, 0.0);
    const auto one = number_expression(repository, builder, 1.0);
    const auto two = number_expression(repository, builder, 2.0);
    const auto three = number_expression(repository, builder, 3.0);

    const auto first = multi_expression(repository,
                                        builder,
                                        formalism::MultiArithmeticOperator::Add,
                                        { three, binary_expression(repository, builder, formalism::BinaryArithmeticOperator::Add, two, zero), two });
    const auto second = binary_expression(repository,
                                          builder,
                                          formalism::BinaryArithmeticOperator::Add,
                                          two,
                                          multi_expression(repository, builder, formalism::MultiArithmeticOperator::Add, { two, three, zero }));

    auto storage = std::make_shared<semantic::detail::TranslationStorage>(1);
    auto translator = semantic::detail::CopyTranslator(storage, true, semantic::TranslationPhase::NormalizeArithmeticExpressions);
    const auto normalized_first = translator.copy(first);
    const auto normalized_second = translator.copy(second);

    const auto empty_sum = translator.copy(multi_expression(repository, builder, formalism::MultiArithmeticOperator::Add, {}));
    const auto unary_product = translator.copy(multi_expression(repository, builder, formalism::MultiArithmeticOperator::Mul, { three }));

    EXPECT_EQ(normalized_first, normalized_second);
    EXPECT_EQ(formalism::format::to_string(normalized_first), "(+ 2 2 3)");
    EXPECT_EQ(formalism::format::to_string(empty_sum), "0");
    EXPECT_EQ(unary_product, translator.copy(three));
    EXPECT_FALSE(contains_binary_add_or_multiply(normalized_first));

    const auto collapsed_product =
        translator.copy(multi_expression(repository,
                                         builder,
                                         formalism::MultiArithmeticOperator::Mul,
                                         { one, binary_expression(repository, builder, formalism::BinaryArithmeticOperator::Mul, three, one) }));
    EXPECT_EQ(formalism::format::to_string(collapsed_product), "3");

    const auto zero_product = translator.copy(binary_expression(repository, builder, formalism::BinaryArithmeticOperator::Mul, zero, three));
    EXPECT_EQ(formalism::format::to_string(zero_product), "(* 0 3)");

    const auto product_exposed_by_sum = translator.copy(
        binary_expression(repository,
                          builder,
                          formalism::BinaryArithmeticOperator::Mul,
                          multi_expression(repository,
                                           builder,
                                           formalism::MultiArithmeticOperator::Add,
                                           { zero, binary_expression(repository, builder, formalism::BinaryArithmeticOperator::Mul, two, three) }),
                          two));
    EXPECT_EQ(formalism::format::to_string(product_exposed_by_sum), "(* 2 2 3)");

    const auto subtraction = translator.copy(binary_expression(repository,
                                                               builder,
                                                               formalism::BinaryArithmeticOperator::Sub,
                                                               first,
                                                               binary_expression(repository, builder, formalism::BinaryArithmeticOperator::Mul, three, one)));
    EXPECT_EQ(formalism::format::to_string(subtraction), "(- (+ 2 2 3) 3)");
    EXPECT_FALSE(contains_binary_add_or_multiply(subtraction));
}

TEST(LokiSemanticTranslator, FlattensDeepArithmeticBeforeMaterializing)
{
    constexpr auto operand_count = std::size_t { 256 };
    auto repository = formalism::Repository(0);
    auto builder = formalism::Builder {};
    const auto two = number_expression(repository, builder, 2.0);
    auto source = two;
    for (auto i = std::size_t { 1 }; i < operand_count; ++i)
        source = binary_expression(repository, builder, formalism::BinaryArithmeticOperator::Add, source, two);

    auto storage = std::make_shared<semantic::detail::TranslationStorage>(1);
    auto translator = semantic::detail::CopyTranslator(storage, true, semantic::TranslationPhase::NormalizeArithmeticExpressions);
    const auto normalized = translator.copy(source);

    ASSERT_TRUE(normalized.get_variant().is<ygg::Index<formalism::FunctionExpression<formalism::MultiTag>>>());
    const auto multi = normalized.get_variant().get<ygg::Index<formalism::FunctionExpression<formalism::MultiTag>>>();
    EXPECT_EQ(multi.get_args().size(), operand_count);
    EXPECT_EQ(storage->repository.size<formalism::FunctionExpression<formalism::MultiTag>>(), 1);
}

TEST(LokiSemanticTranslator, AppliesArithmeticNormalizationAfterEffectNormalFormWhenEnabled)
{
    const auto domain_source = std::string(R"(
(define (domain arithmetic-normalization)
  (:requirements :strips :numeric-fluents)
  (:functions (f) (x) (y))
  (:action step
    :parameters ()
    :precondition (and)
    :effect (and
      (increase (f) (+ 0 (y) (x)))
      (increase (f) (x)))))
)");
    const auto task_source = std::string(R"(
(define (problem arithmetic-normalization-task)
  (:domain arithmetic-normalization)
  (:requirements :strips :numeric-fluents)
  (:init (= (f) 0) (= (x) 1) (= (y) 2))
  (:goal (> (+ (y) (+ 0 (x)) (x)) 0))
  (:metric minimize (+ (y) (+ 0 (x)) (x))))
)");
    auto parser = semantic::Parser(domain_source, semantic::ParserOptions { .add_action_costs = false });

    const auto defaults = semantic::TranslatorOptions {};
    ASSERT_FALSE(defaults.normalize_arithmetic_expressions);
    const auto unnormalized_domain = semantic::translate(parser.get_domain(), defaults);
    const auto unnormalized_task = semantic::translate(parser.parse_task(task_source), unnormalized_domain, defaults);
    EXPECT_NE(formalism::format::to_string(unnormalized_domain.get_translated_domain()).find("(+ (+ (x) (y) 0) (x))"), std::string::npos);
    ASSERT_TRUE(unnormalized_task.get_translated_task().get_metric().has_value());
    EXPECT_EQ(formalism::format::to_string(unnormalized_task.get_translated_task().get_metric().value().get_expression()), "(+ (+ (x) 0) (x) (y))");

    auto options = semantic::TranslatorOptions {};
    options.normalize_arithmetic_expressions = true;
    const auto normalized_domain = semantic::translate(parser.get_domain(), options);
    const auto normalized_task = semantic::translate(parser.parse_task(task_source), normalized_domain, options);
    const auto normalized_domain_text = formalism::format::to_string(normalized_domain.get_translated_domain());
    EXPECT_NE(normalized_domain_text.find("(increase (f) (+ (x) (x) (y)))"), std::string::npos);
    ASSERT_TRUE(normalized_task.get_translated_task().get_goal().has_value());
    EXPECT_EQ(formalism::format::to_string(normalized_task.get_translated_task().get_goal().value()), "(> (+ (x) (x) (y)) 0)");
    ASSERT_TRUE(normalized_task.get_translated_task().get_metric().has_value());
    EXPECT_EQ(formalism::format::to_string(normalized_task.get_translated_task().get_metric().value().get_expression()), "(+ (x) (x) (y))");

    const auto normalized_again = semantic::translate(normalized_domain.get_translated_domain(), options);
    EXPECT_EQ(formalism::format::to_string(normalized_again.get_translated_domain()), normalized_domain_text);
}

TEST(LokiCanonicalization, SortsSemanticFreeListsLexicographicallyBeforeInterning)
{
    auto repository = formalism::Repository(0);
    auto builder = formalism::Builder {};
    const auto make_predicate = [&](const char* name)
    { return intern<formalism::Predicate>(repository, builder, [&](auto& data) { data.name = cista::offset::string(name); }); };
    const auto p_predicate = make_predicate("p");
    const auto q_predicate = make_predicate("q");
    const auto p = p_predicate.get_index();
    const auto q = q_predicate.get_index();
    const auto make_atom = [&](auto predicate)
    { return intern<formalism::Atom>(repository, builder, [&](auto& data) { data.predicate = predicate; }).get_index(); };
    const auto p_atom = make_atom(p);
    const auto q_atom = make_atom(q);
    const auto make_literal = [&](auto atom)
    {
        return intern<formalism::Literal>(repository,
                                          builder,
                                          [&](auto& data)
                                          {
                                              data.atom = atom;
                                              data.m_polarity = true;
                                          })
            .get_index();
    };
    const auto p_literal = make_literal(p_atom);
    const auto q_literal = make_literal(q_atom);
    const auto make_condition = [&](auto literal)
    {
        const auto condition_literal = intern<formalism::Condition<formalism::LiteralTag>>(repository, builder, [&](auto& data) { data.literal = literal; }).get_index();
        return intern<formalism::Condition<>>(repository, builder, [&](auto& data) { data.variant = ygg::Data<formalism::Condition<>>::Variant(condition_literal); });
    };
    const auto p_condition_view = make_condition(p_literal);
    const auto q_condition_view = make_condition(q_literal);
    const auto p_condition = p_condition_view.get_index();
    const auto q_condition = q_condition_view.get_index();

    const auto first = intern<formalism::Condition<formalism::AndTag>>(repository,
                                                       builder,
                                                       [&](auto& data)
                                                       {
                                                           data.conditions.push_back(q_condition);
                                                           data.conditions.push_back(p_condition);
                                                       });

    const auto second = intern<formalism::Condition<formalism::AndTag>>(repository,
                                                        builder,
                                                        [&](auto& data)
                                                        {
                                                            data.conditions.push_back(p_condition);
                                                            data.conditions.push_back(q_condition);
                                                        });

    EXPECT_EQ(first, second);
    ASSERT_EQ(first.get_conditions().size(), 2);
    EXPECT_EQ(first.get_conditions()[0], p_condition_view);
    EXPECT_EQ(first.get_conditions()[1], q_condition_view);

    const auto first_domain = intern<formalism::Domain>(repository,
                                                        builder,
                                                        [&](auto& data)
                                                        {
                                                            data.name = cista::offset::string("d");
                                                            data.predicates.push_back(q);
                                                            data.predicates.push_back(p);
                                                        });

    const auto second_domain = intern<formalism::Domain>(repository,
                                                         builder,
                                                         [&](auto& data)
                                                         {
                                                             data.name = cista::offset::string("d");
                                                             data.predicates.push_back(p);
                                                             data.predicates.push_back(q);
                                                         });

    EXPECT_EQ(first_domain, second_domain);
    ASSERT_EQ(first_domain.get_predicates().size(), 2);
    EXPECT_EQ(first_domain.get_predicates()[0], p_predicate);
    EXPECT_EQ(first_domain.get_predicates()[1], q_predicate);
}

}  // namespace loki::tests
