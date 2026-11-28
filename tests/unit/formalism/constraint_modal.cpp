#include "loki/formalism/constraint_modal_data.hpp"
#include "loki/formalism/constraint_modal_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

using Index = urd::index<loki::formalism::Constraint<loki::formalism::ModalTag>>;
using Data = urd::data<loki::formalism::Constraint<loki::formalism::ModalTag>>;
using View = urd::index_view<loki::formalism::Constraint<loki::formalism::ModalTag>, loki::formalism::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, loki::formalism::EntityView<loki::formalism::Constraint<loki::formalism::ModalTag>>>);
static_assert(requires(Data& data) {
    data.index;
    data.operation;
    data.numbers;
    data.constraints;
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.index();
    view.operation();
    view.numbers();
    view.constraints();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
