#include "loki/formalism/atom_data.hpp"
#include "loki/formalism/atom_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

using Index = urd::index<loki::formalism::Atom>;
using Data = urd::data<loki::formalism::Atom>;
using View = urd::index_view<loki::formalism::Atom, loki::formalism::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, loki::formalism::AtomView>);
static_assert(requires(Data& data) {
    data.index;
    data.predicate;
    data.terms;
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.index();
    view.get_predicate();
    view.get_terms();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
