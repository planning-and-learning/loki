#include "loki/formalism/repository.hpp"
#include "loki/formalism/timed_initial_literal_data.hpp"
#include "loki/formalism/timed_initial_literal_view.hpp"

#include <concepts>

using Index = urd::index<loki::formalism::TimedInitialLiteral>;
using Data = urd::data<loki::formalism::TimedInitialLiteral>;
using View = urd::index_view<loki::formalism::TimedInitialLiteral, loki::formalism::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, loki::formalism::EntityView<loki::formalism::TimedInitialLiteral>>);
static_assert(requires(Data& data) {
    data.index;
    data.time;
    data.literal;
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.index();
    view.time();
    view.literal();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
