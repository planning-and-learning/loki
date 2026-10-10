#include "loki/formalism/parameter_data.hpp"
#include "loki/formalism/parameter_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

namespace f = loki::formalism;
using Index = ygg::Index<f::Parameter>;
using Data = ygg::Data<f::Parameter>;
using View = ygg::View<Index, f::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, f::ParameterView>);
static_assert(requires(Data& data) {
    data.index;
    data.variable;
    data.types;
    data.clear();
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.get_index();
    view.get_variable();
    view.get_types();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
