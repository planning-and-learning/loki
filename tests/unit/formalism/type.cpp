#include "loki/formalism/repository.hpp"
#include "loki/formalism/type_data.hpp"
#include "loki/formalism/type_view.hpp"

#include <concepts>
#include <gtest/gtest.h>

using Index = urd::index<loki::formalism::Type>;
using Data = urd::data<loki::formalism::Type>;
using View = urd::index_view<loki::formalism::Type, loki::formalism::Repository>;

namespace
{
struct TypeRepository
{
    using SymbolTypes = boost::mp11::mp_list<loki::formalism::Type>;
    const Data& operator[](Index) const;
    [[nodiscard]] std::size_t layer() const;
};
struct TypeContext
{
    const TypeRepository* repository;
    // Only checked by SymbolContextFor in unevaluated contexts.
    [[maybe_unused]] friend const TypeRepository& get_repository(const TypeContext& context) { return *context.repository; }
};

template<typename T>
concept CanInsert = requires(loki::formalism::Repository& repository, urd::data<T>& data) { loki::formalism::insert(repository, data); };
}

static_assert(ygg::formalism::SymbolRepositoryFor<TypeRepository, loki::formalism::Type>);
static_assert(!ygg::formalism::SymbolRepositoryFor<TypeRepository, loki::formalism::Object>);
static_assert(ygg::formalism::SymbolContextFor<TypeContext, loki::formalism::Type>);
static_assert(!ygg::formalism::SymbolContextFor<TypeContext, loki::formalism::Object>);
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
    view.get_name();
    view.get_bases();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});

TEST(LokiTests, SharedInterningCanonicalizesTypeBasesUsingTheRepository)
{
    auto repository = loki::formalism::Repository(0);
    auto builder = loki::formalism::Builder {};
    auto zebra_data = Data(urd::flat_string("zebra"));
    auto alpha_data = Data(urd::flat_string("alpha"));
    const auto zebra = loki::formalism::insert(repository, zebra_data).first;
    const auto alpha = loki::formalism::insert(repository, alpha_data).first;
    ASSERT_LT(zebra.index(), alpha.index());

    auto data = loki::formalism::checkout<loki::formalism::Type>(builder);
    data->name = urd::flat_string("combined");
    data->bases.push_back(zebra.index());
    data->bases.push_back(alpha.index());
    data->bases.push_back(zebra.index());
    const auto [view, inserted] = loki::formalism::insert(repository, *data);
    EXPECT_TRUE(inserted);
    EXPECT_EQ(data->index, view.index());
    ASSERT_EQ(view.get_bases().size(), 2);
    EXPECT_EQ(view.get_bases()[0], alpha);
    EXPECT_EQ(view.get_bases()[1], zebra);

    data->bases.clear();
    data->bases.push_back(zebra.index());
    data->bases.push_back(alpha.index());
    data->index = Index();
    const auto [duplicate, duplicate_inserted] = loki::formalism::insert(repository, *data);
    EXPECT_FALSE(duplicate_inserted);
    EXPECT_EQ(duplicate, view);
    EXPECT_EQ(data->index, view.index());
}

TEST(LokiTests, SymbolViewsUseTheRepositoryOfTheirContext)
{
    auto repository = loki::formalism::Repository(13);
    auto data = Data(urd::flat_string("type"));
    const auto original = loki::formalism::insert(repository, data).first;
    const auto again = repository.view(original.index());
    EXPECT_EQ(&again.data(), &original.data());
    EXPECT_EQ(again.get_name(), original.get_name());
    EXPECT_EQ(urd::detail::identity(again), urd::detail::identity(original));

    auto child = loki::formalism::Repository(14, &repository);
    const auto [inherited, inserted] = loki::formalism::insert(child, data);
    EXPECT_FALSE(inserted);
    EXPECT_EQ(&inherited.context(), &repository);
    EXPECT_EQ(inherited, original);
}
