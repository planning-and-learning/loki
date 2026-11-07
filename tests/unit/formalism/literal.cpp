#include "loki/formalism/literal_data.hpp"
#include "loki/formalism/literal_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

using Index = urd::index<loki::formalism::Literal>;
using Data = urd::data<loki::formalism::Literal>;
using View = urd::index_view<loki::formalism::Literal, loki::formalism::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, loki::formalism::LiteralView>);
static_assert(requires(Data& data) {
    data.index;
    data.atom;
    data.m_polarity;
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.index();
    view.get_atom();
    view.get_polarity();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
