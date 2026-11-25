#include "loki/formalism/constraint_not_data.hpp"
#include "loki/formalism/constraint_not_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

using Index = urd::index<loki::formalism::Constraint<loki::formalism::NotTag>>;
using Data = urd::data<loki::formalism::Constraint<loki::formalism::NotTag>>;
using View = urd::index_view<loki::formalism::Constraint<loki::formalism::NotTag>, loki::formalism::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, loki::formalism::EntityView<loki::formalism::Constraint<loki::formalism::NotTag>>>);
static_assert(requires(Data& data) {
    data.index;
    data.constraint;
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.index();
    view.get_constraint();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
