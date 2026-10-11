#include "loki/formalism/action_data.hpp"
#include "loki/formalism/action_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

using Index = ygg::Index<loki::formalism::Action>;
using Data = ygg::Data<loki::formalism::Action>;
using View = ygg::View<Index, loki::formalism::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, loki::formalism::ActionView>);
static_assert(requires(Data& data) {
    data.index;
    data.name;
    data.original_name;
    data.parameters;
    data.original_arity;
    data.precondition;
    data.effect;
    data.clear();
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.get_index();
    view.get_name();
    view.get_original_name();
    view.get_parameters();
    view.get_arity();
    view.get_original_arity();
    view.get_precondition();
    view.get_effect();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
