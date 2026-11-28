#include "loki/formalism/constraint_state_data.hpp"
#include "loki/formalism/constraint_state_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

using Index = urd::index<loki::formalism::Constraint<loki::formalism::StateTag>>;
using Data = urd::data<loki::formalism::Constraint<loki::formalism::StateTag>>;
using View = urd::index_view<loki::formalism::Constraint<loki::formalism::StateTag>, loki::formalism::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, loki::formalism::EntityView<loki::formalism::Constraint<loki::formalism::StateTag>>>);
static_assert(requires(Data& data) {
    data.index;
    data.condition;
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.index();
    view.condition();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
