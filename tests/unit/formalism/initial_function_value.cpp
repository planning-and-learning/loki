#include "loki/formalism/initial_function_value_data.hpp"
#include "loki/formalism/initial_function_value_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

namespace f = loki::formalism;
using Index = ygg::Index<f::InitialFunctionValue>;
using Data = ygg::Data<f::InitialFunctionValue>;
using View = ygg::View<Index, f::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, f::InitialFunctionValueView>);
static_assert(requires(Data& data) {
    data.index;
    data.function;
    data.value;
    data.clear();
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.get_index();
    view.get_function();
    view.get_value();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
