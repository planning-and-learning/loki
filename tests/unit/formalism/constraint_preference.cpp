#include "loki/formalism/constraint_preference_data.hpp"
#include "loki/formalism/constraint_preference_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

using Index = urd::index<loki::formalism::Constraint<loki::formalism::PreferenceTag>>;
using Data = urd::data<loki::formalism::Constraint<loki::formalism::PreferenceTag>>;
using View = urd::index_view<loki::formalism::Constraint<loki::formalism::PreferenceTag>, loki::formalism::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, loki::formalism::EntityView<loki::formalism::Constraint<loki::formalism::PreferenceTag>>>);
static_assert(requires(Data& data) {
    data.index;
    data.name;
    data.constraint;
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.index();
    view.name();
    view.constraint();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
