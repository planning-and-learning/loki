#include "loki/formalism/effect_and_data.hpp"
#include "loki/formalism/effect_and_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

namespace f = loki::formalism;
using Index = ygg::Index<f::Effect<f::AndTag>>;
using Data = ygg::Data<f::Effect<f::AndTag>>;
using View = ygg::View<Index, f::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, f::EntityView<f::Effect<f::AndTag>>>);
static_assert(requires(Data& data) {
    data.index;
    data.effects;
    data.clear();
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.get_index();
    view.get_effects();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
