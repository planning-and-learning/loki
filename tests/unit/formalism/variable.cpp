#include "loki/formalism/repository.hpp"
#include "loki/formalism/variable_data.hpp"
#include "loki/formalism/variable_view.hpp"

#include <concepts>
#include <gtest/gtest.h>

using Index = urd::index<loki::formalism::Variable>;
using Data = urd::data<loki::formalism::Variable>;
using View = urd::index_view<loki::formalism::Variable, loki::formalism::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, loki::formalism::VariableView>);
static_assert(requires(Data& data) {
    data.index;
    data.name;
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.index();
    view.get_name();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});

TEST(LokiTests, VariableViewIdentityUsesTheOwningLayer)
{
    auto first_repository = loki::formalism::Repository();
    auto second_repository = loki::formalism::Repository();
    auto first_data = Data(urd::flat_string("?x"));
    auto second_data = Data(urd::flat_string("?x"));

    const auto [first, first_created] = first_repository.insert(first_data);
    const auto [second, second_created] = second_repository.insert(second_data);

    ASSERT_TRUE(first_created);
    ASSERT_TRUE(second_created);
    ASSERT_EQ(first.index(), second.index());
    EXPECT_NE(first, second);
    EXPECT_NE(urd::Hash<View> {}(first), urd::Hash<View> {}(second));
    const auto again = first_repository.view(first.index());
    EXPECT_EQ(first, again);
    EXPECT_EQ(urd::Hash<View> {}(first), urd::Hash<View> {}(again));
}
