#include "loki/formalism/repository.hpp"
#include "loki/formalism/task_data.hpp"
#include "loki/formalism/task_view.hpp"

#include <concepts>

namespace f = loki::formalism;
using Index = ygg::Index<f::Task>;
using Data = ygg::Data<f::Task>;
using View = ygg::View<Index, f::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, f::TaskView>);
static_assert(requires(Data& data) {
    data.index;
    data.name;
    data.domain;
    data.requirements;
    data.objects;
    data.initial_literals;
    data.initial_function_values;
    data.goal;
    data.metric;
    data.predicates;
    data.axioms;
    data.clear();
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.get_index();
    view.get_name();
    view.get_domain();
    view.get_requirements();
    view.get_objects();
    view.get_initial_literals();
    view.get_initial_function_values();
    view.get_goal();
    view.get_metric();
    view.get_predicates();
    view.get_axioms();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
