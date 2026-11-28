#include "loki/formalism/repository.hpp"
#include "loki/formalism/type_data.hpp"
#include "loki/formalism/type_view.hpp"

#include <concepts>
#include <gtest/gtest.h>
#include <type_traits>

using Index = urd::index<loki::formalism::Type>;
using Data = urd::data<loki::formalism::Type>;
using View = urd::index_view<loki::formalism::Type, loki::formalism::Repository>;

namespace
{
template<typename T>
concept CanInsert = requires(loki::formalism::Repository& repository, urd::data<T>& data) { repository.insert(data); };
}

static_assert(CanInsert<loki::formalism::Type>);
static_assert(!CanInsert<int>);
static_assert(urd::resolves<loki::formalism::Repository, loki::formalism::Type>);
static_assert(ygg::formalism::SupportsSymbol<loki::formalism::Repository, loki::formalism::Type>);
static_assert(!ygg::formalism::SupportsSymbol<loki::formalism::Repository, int>);
static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, loki::formalism::TypeView>);
static_assert(requires(Data& data) {
    data.index;
    data.name;
    data.bases;
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.index();
    view.name();
    view.bases();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});

TEST(LokiTests, SharedInterningCanonicalizesTypeBasesUsingTheRepository)
{
    auto repository = loki::formalism::Repository();
    auto builder = loki::formalism::Builder {};
    auto zebra_data = Data(urd::flat_string("zebra"));
    auto alpha_data = Data(urd::flat_string("alpha"));
    const auto zebra = repository.insert(zebra_data).first;
    const auto alpha = repository.insert(alpha_data).first;
    ASSERT_LT(zebra.index(), alpha.index());

    auto data = loki::formalism::checkout<loki::formalism::Type>(builder);
    data->name = urd::flat_string("combined");
    data->bases.emplace_back(zebra.index());
    data->bases.emplace_back(alpha.index());
    data->bases.emplace_back(zebra.index());
    const auto [view, inserted] = repository.insert(*data, builder);
    EXPECT_TRUE(inserted);
    ASSERT_EQ(view.bases().size(), 2);
    EXPECT_EQ(view.bases()[0], alpha);
    EXPECT_EQ(view.bases()[1], zebra);

    data->bases.clear();
    data->bases.emplace_back(zebra.index());
    data->bases.emplace_back(alpha.index());
    data->index = Index();
    const auto [duplicate, duplicate_inserted] = repository.insert(*data, builder);
    EXPECT_FALSE(duplicate_inserted);
    EXPECT_EQ(duplicate, view);
}

TEST(LokiTests, SymbolViewsUseTheRepositoryOfTheirContext)
{
    auto repository = loki::formalism::Repository();
    auto data = Data(urd::flat_string("type"));
    const auto original = repository.insert(data).first;
    const auto again = repository.view(original.index());
    EXPECT_EQ(&again.name(), &original.name());
    EXPECT_EQ(again.name(), original.name());
    EXPECT_EQ(urd::detail::identity(again), urd::detail::identity(original));

    auto child = loki::formalism::Repository(&repository);
    const auto [inherited, inserted] = child.insert(data);
    EXPECT_FALSE(inserted);
    EXPECT_EQ(&inherited.context(), &repository);
    EXPECT_EQ(inherited, original);
}
