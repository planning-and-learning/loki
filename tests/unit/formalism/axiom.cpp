#include "loki/formalism/axiom_data.hpp"
#include "loki/formalism/axiom_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

using Index = urd::index<loki::formalism::Axiom>;
using Data = urd::data<loki::formalism::Axiom>;
using View = urd::view<loki::formalism::Axiom, loki::formalism::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, loki::formalism::AxiomView>);
static_assert(requires(Data& data) {
    data.index;
    data.parameters;
    data.original_arity;
    data.head;
    data.condition;
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.index();
    view.get_parameters();
    view.get_arity();
    view.get_original_arity();
    view.get_head();
    view.get_condition();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
