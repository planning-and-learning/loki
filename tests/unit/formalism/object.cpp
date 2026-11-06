#include "loki/formalism/object_data.hpp"
#include "loki/formalism/object_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

using Index = urd::index<loki::formalism::Object>;
using Data = urd::data<loki::formalism::Object>;
using View = urd::view<loki::formalism::Object, loki::formalism::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, loki::formalism::ObjectView>);
static_assert(requires(Data& data) {
    data.index;
    data.name;
    data.types;
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.index();
    view.get_name();
    view.get_types();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
