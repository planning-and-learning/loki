#include "loki/formalism/durative_action_data.hpp"
#include "loki/formalism/durative_action_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

using Index = urd::index<loki::formalism::DurativeAction>;
using Data = urd::data<loki::formalism::DurativeAction>;
using View = urd::index_view<loki::formalism::DurativeAction, loki::formalism::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, loki::formalism::EntityView<loki::formalism::DurativeAction>>);
static_assert(requires(Data& data) {
    data.index;
    data.name;
    data.original_name;
    data.parameters;
    data.original_arity;
    data.duration;
    data.condition;
    data.effect;
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.index();
    view.name();
    view.original_name();
    view.parameters();
    view.original_arity();
    view.duration();
    view.condition();
    view.effect();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
