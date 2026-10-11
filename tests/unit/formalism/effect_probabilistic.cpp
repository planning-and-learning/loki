#include "loki/formalism/effect_probabilistic_data.hpp"
#include "loki/formalism/effect_probabilistic_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

using Index = ygg::Index<loki::formalism::Effect<loki::formalism::ProbabilisticTag>>;
using Data = ygg::Data<loki::formalism::Effect<loki::formalism::ProbabilisticTag>>;
using View = ygg::View<Index, loki::formalism::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, loki::formalism::EntityView<loki::formalism::Effect<loki::formalism::ProbabilisticTag>>>);
static_assert(requires(Data& data) {
    data.index;
    data.alternatives;
    data.clear();
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.get_index();
    view.get_alternatives();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
