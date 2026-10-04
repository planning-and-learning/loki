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
    const auto zebra = f::get_or_create(repository, zebra_data).first;
    const auto alpha = f::get_or_create(repository, alpha_data).first;
    ASSERT_LT(zebra.get_index(), alpha.get_index());

    auto data = f::checkout<f::Type>(builder);
    data->name = cista::offset::string("combined");
    data->bases.push_back(zebra.get_index());
    data->bases.push_back(alpha.get_index());
    data->bases.push_back(zebra.get_index());
    const auto [view, inserted] = f::get_or_create(repository, *data);
    EXPECT_TRUE(inserted);
    EXPECT_EQ(data->index, view.get_index());
    ASSERT_EQ(view.get_bases().size(), 2);
    EXPECT_EQ(view.get_bases()[0], alpha);
    EXPECT_EQ(view.get_bases()[1], zebra);

    data->bases.clear();
    data->bases.push_back(zebra.get_index());
    data->bases.push_back(alpha.get_index());
    data->index = Index::max();
    const auto [duplicate, duplicate_inserted] = f::get_or_create(repository, *data);
    EXPECT_FALSE(duplicate_inserted);
    EXPECT_EQ(duplicate, view);
    EXPECT_EQ(data->index, view.get_index());
}
