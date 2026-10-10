#include "loki/formalism/condition_numeric_constraint_data.hpp"
#include "loki/formalism/condition_numeric_constraint_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

namespace f = loki::formalism;
using Index = ygg::Index<f::Condition<f::NumericConstraintTag>>;
using Data = ygg::Data<f::Condition<f::NumericConstraintTag>>;
using View = ygg::View<Index, f::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, f::EntityView<f::Condition<f::NumericConstraintTag>>>);
static_assert(requires(Data& data) {
    data.index;
    data.comparator;
    data.left;
    data.right;
    data.clear();
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.get_index();
    view.get_comparator();
    view.get_left();
    view.get_right();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
