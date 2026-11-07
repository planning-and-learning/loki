#include "loki/formalism/effect_probabilistic_alternative_data.hpp"
#include "loki/formalism/effect_probabilistic_alternative_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

using Index = urd::index<loki::formalism::EffectProbabilisticAlternative>;
using Data = urd::data<loki::formalism::EffectProbabilisticAlternative>;
using View = urd::index_view<loki::formalism::EffectProbabilisticAlternative, loki::formalism::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, loki::formalism::EffectProbabilisticAlternativeView>);
static_assert(requires(Data& data) {
    data.index;
    data.probability;
    data.effect;
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.index();
    view.get_probability();
    view.get_effect();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
