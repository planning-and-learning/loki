#include "loki/formalism/multi_function_expression_data.hpp"
#include "loki/formalism/multi_function_expression_index.hpp"
#include "loki/formalism/multi_function_expression_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>
#include <gtest/gtest.h>
#include <utility>

namespace f = loki::formalism;
using Index = ygg::Index<f::MultiFunctionExpression>;
using Data = ygg::Data<f::MultiFunctionExpression>;
using View = ygg::View<Index, f::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, f::MultiFunctionExpressionView>);
static_assert(std::constructible_from<Data, f::MultiArithmeticOperator, ygg::IndexList<f::FunctionExpression>>);
static_assert(requires(Data& data) {
    data.index;
    data.op;
    data.args;
    data.clear();
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.get_index();
    view.get_operator();
    view.get_args();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});

template<typename T, typename Initialize>
f::EntityView<T> intern(f::Repository& repository, f::Builder& builder, Initialize&& initialize)
{
    auto data = f::checkout<T>(builder);
    std::forward<Initialize>(initialize)(*data);
    return f::insert(repository, *data).first;
}

TEST(LokiTests, MultiFunctionExpressionAllowsAnyArity)
{
    auto repository = f::Repository(0);
    auto data = Data {};
    const auto [expression, created] = f::insert(repository, data);
    EXPECT_TRUE(created);
    EXPECT_TRUE(expression.get_args().empty());
}

TEST(LokiTests, MultiFunctionExpressionBreaksEqualRenderTiesByIndex)
{
    auto repository = f::Repository(0);
    auto builder = f::Builder {};
    const auto wrap = [&](auto node) {
        return intern<f::FunctionExpression>(repository,
                                             builder,
                                             [&](auto& data) { data.variant = ygg::Data<f::FunctionExpression>::Variant(node.get_index()); });
    };
    const auto make_number = [&](double value) { return intern<f::FunctionExpressionNumber>(repository, builder, [&](auto& data) { data.value = value; }); };
    const auto one = wrap(make_number(1.0));
    const auto two = wrap(make_number(2.0));
    const auto binary = wrap(intern<f::BinaryFunctionExpression>(repository,
                                                                 builder,
                                                                 [&](auto& data)
                                                                 {
                                                                     data.op = f::BinaryArithmeticOperator::Add;
                                                                     data.left = one.get_index();
                                                                     data.right = two.get_index();
                                                                 }));
    const auto multi = wrap(intern<f::MultiFunctionExpression>(repository,
                                                               builder,
                                                               [&](auto& data)
                                                               {
                                                                   data.op = f::MultiArithmeticOperator::Add;
                                                                   data.args.push_back(one.get_index());
                                                                   data.args.push_back(two.get_index());
                                                               }));
    ASSERT_NE(binary, multi);

    const auto make_product = [&](auto left, auto right)
    {
        return intern<f::MultiFunctionExpression>(repository,
                                                  builder,
                                                  [&](auto& data)
                                                  {
                                                      data.op = f::MultiArithmeticOperator::Mul;
                                                      data.args.push_back(left.get_index());
                                                      data.args.push_back(right.get_index());
                                                  });
    };
    const auto first = make_product(binary, multi);
    const auto second = make_product(multi, binary);

    EXPECT_EQ(first, second);
    EXPECT_EQ(first.get_args().size(), 2);
    EXPECT_NE(first.get_args()[0], first.get_args()[1]);
}
