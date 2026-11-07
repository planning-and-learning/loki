#include "loki/formalism/domain_data.hpp"
#include "loki/formalism/domain_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

using Index = urd::index<loki::formalism::Domain>;
using Data = urd::data<loki::formalism::Domain>;
using View = urd::index_view<loki::formalism::Domain, loki::formalism::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, loki::formalism::DomainView>);
static_assert(requires(Data& data) {
    data.index;
    data.name;
    data.requirements;
    data.types;
    data.constants;
    data.predicates;
    data.functions;
    data.actions;
    data.axioms;
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.index();
    view.get_name();
    view.get_requirements();
    view.get_types();
    view.get_constants();
    view.get_predicates();
    view.get_functions();
    view.get_actions();
    view.get_axioms();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
