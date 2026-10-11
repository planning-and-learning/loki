#include "loki/formalism/metric_data.hpp"
#include "loki/formalism/metric_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

using Index = ygg::Index<loki::formalism::Metric>;
using Data = ygg::Data<loki::formalism::Metric>;
using View = ygg::View<Index, loki::formalism::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, loki::formalism::MetricView>);
static_assert(requires(Data& data) {
    data.index;
    data.optimization_direction;
    data.expression;
    data.clear();
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.get_index();
    view.get_optimization_direction();
    view.get_expression();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});

static_assert(std::totally_ordered<loki::formalism::OptimizationDirection>);
static_assert(loki::formalism::OptimizationDirection::Minimize < loki::formalism::OptimizationDirection::Maximize);
