#include "loki/formalism/condition_and_data.hpp"
#include "loki/formalism/condition_and_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

using Index = urd::index<loki::formalism::Condition<loki::formalism::AndTag>>;
using Data = urd::data<loki::formalism::Condition<loki::formalism::AndTag>>;
using View = urd::view<loki::formalism::Condition<loki::formalism::AndTag>, loki::formalism::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, loki::formalism::EntityView<loki::formalism::Condition<loki::formalism::AndTag>>>);
static_assert(requires(Data& data) {
    data.index;
    data.conditions;
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.index();
    view.get_conditions();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
