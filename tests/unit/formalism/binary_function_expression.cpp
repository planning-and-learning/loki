#include "loki/formalism/binary_function_expression_data.hpp"
#include "loki/formalism/binary_function_expression_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

using Index = ygg::Index<loki::formalism::FunctionExpression<loki::formalism::BinaryTag>>;
using Data = ygg::Data<loki::formalism::FunctionExpression<loki::formalism::BinaryTag>>;
using View = ygg::View<Index, loki::formalism::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, loki::formalism::EntityView<loki::formalism::FunctionExpression<loki::formalism::BinaryTag>>>);
static_assert(requires(Data& data) {
    data.index;
    data.op;
    data.left;
    data.right;
    data.clear();
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.get_index();
    view.get_operator();
    view.get_left();
    view.get_right();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
