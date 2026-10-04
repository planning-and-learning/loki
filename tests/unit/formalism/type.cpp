#include "loki/formalism/repository.hpp"
#include "loki/formalism/type_data.hpp"
#include "loki/formalism/type_index.hpp"
#include "loki/formalism/type_view.hpp"

#include <concepts>
#include <gtest/gtest.h>

namespace f = loki::formalism;
using Index = ygg::Index<f::Type>;
using Data = ygg::Data<f::Type>;
using View = ygg::View<Index, f::Repository>;

namespace
{
struct TypeRepository
{
    using SymbolTypes = ygg::TypeList<f::Type>;
    const Data& operator[](Index) const;
    size_t get_index() const;
};
struct TypeContext
{
    const TypeRepository& repository;
    friend const TypeRepository& get_repository(const TypeContext& context) { return context.repository; }
};
struct RepositoryContext
{
    const f::Repository& repository;
    friend const f::Repository& get_repository(const RepositoryContext& context) { return context.repository; }
};

template<typename T>
concept CanInsert = requires(f::Repository& repository, ygg::Data<T>& data) { f::insert(repository, data); };
}

static_assert(ygg::formalism::SymbolRepositoryFor<TypeRepository, f::Type>);
static_assert(!ygg::formalism::SymbolRepositoryFor<TypeRepository, f::Object>);
static_assert(ygg::formalism::SymbolContextFor<TypeContext, f::Type>);
static_assert(!ygg::formalism::SymbolContextFor<TypeContext, f::Object>);
static_assert(CanInsert<f::Type>);
static_assert(!CanInsert<int>);
static_assert(ygg::ViewConcept<Index, f::Repository>);
static_assert(ygg::formalism::SupportsSymbol<f::Repository, f::Type>);
static_assert(!ygg::formalism::SupportsSymbol<f::Repository, int>);
static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, f::TypeView>);
static_assert(requires(Data& data) {
    data.index;
    data.name;
    data.bases;
    data.clear();
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.get_index();
    view.get_name();
    view.get_bases();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});

TEST(LokiTests, SharedInterningCanonicalizesTypeBasesUsingTheRepository)
{
    auto repository = f::Repository(0);
    auto builder = f::Builder {};
    auto zebra_data = Data(cista::offset::string("zebra"));
    auto alpha_data = Data(cista::offset::string("alpha"));
    const auto zebra = f::insert(repository, zebra_data).first;
    const auto alpha = f::insert(repository, alpha_data).first;
    ASSERT_LT(zebra.get_index(), alpha.get_index());

    auto data = f::checkout<f::Type>(builder);
    data->name = cista::offset::string("combined");
    data->bases.push_back(zebra.get_index());
    data->bases.push_back(alpha.get_index());
    data->bases.push_back(zebra.get_index());
    const auto [view, inserted] = f::insert(repository, *data);
    EXPECT_TRUE(inserted);
    EXPECT_EQ(data->index, view.get_index());
    ASSERT_EQ(view.get_bases().size(), 2);
    EXPECT_EQ(view.get_bases()[0], alpha);
    EXPECT_EQ(view.get_bases()[1], zebra);

    data->bases.clear();
    data->bases.push_back(zebra.get_index());
    data->bases.push_back(alpha.get_index());
    data->index = Index::max();
    const auto [duplicate, duplicate_inserted] = f::insert(repository, *data);
    EXPECT_FALSE(duplicate_inserted);
    EXPECT_EQ(duplicate, view);
    EXPECT_EQ(data->index, view.get_index());
}

TEST(LokiTests, SymbolViewsUseTheRepositoryOfTheirContext)
{
    auto repository = f::Repository(13);
    auto data = Data(cista::offset::string("type"));
    const auto original = f::insert(repository, data).first;
    const auto context = RepositoryContext { repository };
    const auto forwarded = ygg::make_view(original.get_index(), context);
    EXPECT_EQ(&forwarded.get_data(), &original.get_data());
    EXPECT_EQ(forwarded.get_name(), original.get_name());
    EXPECT_EQ(forwarded.identifying_members(), original.identifying_members());

    auto child = f::Repository(14, &repository);
    const auto [inherited, inserted] = f::insert(child, data);
    EXPECT_FALSE(inserted);
    EXPECT_EQ(&inherited.get_context(), &repository);
    EXPECT_EQ(inherited, original);
}
