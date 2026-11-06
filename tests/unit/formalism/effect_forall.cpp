#include "loki/formalism/effect_forall_data.hpp"
#include "loki/formalism/effect_forall_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

using Index = urd::index<loki::formalism::Effect<loki::formalism::ForallTag>>;
using Data = urd::data<loki::formalism::Effect<loki::formalism::ForallTag>>;
using View = urd::view<loki::formalism::Effect<loki::formalism::ForallTag>, loki::formalism::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, loki::formalism::EntityView<loki::formalism::Effect<loki::formalism::ForallTag>>>);
static_assert(requires(Data& data) {
    data.index;
    data.parameters;
    data.effect;
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.index();
    view.get_parameters();
    view.get_effect();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
