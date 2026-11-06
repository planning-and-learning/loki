#include "loki/formalism/effect_and_data.hpp"
#include "loki/formalism/effect_and_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

using Index = urd::index<loki::formalism::Effect<loki::formalism::AndTag>>;
using Data = urd::data<loki::formalism::Effect<loki::formalism::AndTag>>;
using View = urd::view<loki::formalism::Effect<loki::formalism::AndTag>, loki::formalism::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, loki::formalism::EntityView<loki::formalism::Effect<loki::formalism::AndTag>>>);
static_assert(requires(Data& data) {
    data.index;
    data.effects;
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.index();
    view.get_effects();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
