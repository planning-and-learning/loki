#include "loki/formalism/predicate_data.hpp"
#include "loki/formalism/predicate_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

namespace f = loki::formalism;
using Index = ygg::Index<f::Predicate>;
using Data = ygg::Data<f::Predicate>;
using View = ygg::View<Index, f::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, f::PredicateView>);
static_assert(requires(Data& data) {
    data.index;
    data.name;
    data.parameters;
    data.clear();
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.get_index();
    view.get_name();
    view.get_parameters();
    view.get_arity();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
