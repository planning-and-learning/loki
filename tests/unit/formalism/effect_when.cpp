#include "loki/formalism/effect_when_data.hpp"
#include "loki/formalism/effect_when_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

using Index = urd::index<loki::formalism::Effect<loki::formalism::WhenTag>>;
using Data = urd::data<loki::formalism::Effect<loki::formalism::WhenTag>>;
using View = urd::index_view<loki::formalism::Effect<loki::formalism::WhenTag>, loki::formalism::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, loki::formalism::EntityView<loki::formalism::Effect<loki::formalism::WhenTag>>>);
static_assert(requires(Data& data) {
    data.index;
    data.condition;
    data.effect;
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.index();
    view.get_condition();
    view.get_effect();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
