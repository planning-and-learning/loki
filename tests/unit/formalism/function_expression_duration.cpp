#include "loki/formalism/function_expression_duration_data.hpp"
#include "loki/formalism/function_expression_duration_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

using Index = urd::index<loki::formalism::FunctionExpression<loki::formalism::DurationTag>>;
using Data = urd::data<loki::formalism::FunctionExpression<loki::formalism::DurationTag>>;
using View = urd::index_view<loki::formalism::FunctionExpression<loki::formalism::DurationTag>, loki::formalism::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, loki::formalism::EntityView<loki::formalism::FunctionExpression<loki::formalism::DurationTag>>>);
static_assert(requires(Data& data) {
    data.index;
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.index();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
