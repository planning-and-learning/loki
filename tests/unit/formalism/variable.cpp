#include "loki/formalism/repository.hpp"
#include "loki/formalism/variable_data.hpp"
#include "loki/formalism/variable_view.hpp"

#include <concepts>
#include <gtest/gtest.h>

namespace f = loki::formalism;
using Index = ygg::Index<f::Variable>;
using Data = ygg::Data<f::Variable>;
using View = ygg::View<Index, f::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, f::VariableView>);
static_assert(requires(Data& data) {
    data.index;
    data.name;
    data.clear();
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.get_index();
    view.get_name();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});

TEST(LokiTests, VariableViewIdentityUsesRepositoryIndex)
{
    auto first_repository = f::Repository(0);
    auto second_repository = f::Repository(1);
    auto equivalent_repository = f::Repository(0);
    auto first_data = Data(cista::offset::string("?x"));
    auto second_data = Data(cista::offset::string("?x"));
    auto equivalent_data = Data(cista::offset::string("?x"));

    const auto [first, first_created] = first_repository.insert(first_data);
    const auto [second, second_created] = second_repository.insert(second_data);
    const auto [equivalent, equivalent_created] = equivalent_repository.insert(equivalent_data);

    ASSERT_TRUE(first_created);
    ASSERT_TRUE(second_created);
    ASSERT_TRUE(equivalent_created);
    EXPECT_NE(first, second);
    EXPECT_NE(ygg::Hash<View> {}(first), ygg::Hash<View> {}(second));
    EXPECT_EQ(first, equivalent);
    EXPECT_EQ(ygg::Hash<View> {}(first), ygg::Hash<View> {}(equivalent));
}
