#include "loki/formalism/repository.hpp"
#include "loki/formalism/requirement_data.hpp"
#include "loki/formalism/requirement_view.hpp"

#include <concepts>

using Index = urd::index<loki::formalism::Requirement>;
using Data = urd::data<loki::formalism::Requirement>;
using View = urd::view<loki::formalism::Requirement, loki::formalism::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, loki::formalism::RequirementView>);
static_assert(requires(Data& data) {
    data.index;
    data.kind;
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.index();
    view.get_kind();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
