#include "loki/formalism/function_skeleton_data.hpp"
#include "loki/formalism/function_skeleton_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

using Index = urd::index<loki::formalism::FunctionSkeleton>;
using Data = urd::data<loki::formalism::FunctionSkeleton>;
using View = urd::view<loki::formalism::FunctionSkeleton, loki::formalism::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, loki::formalism::FunctionSkeletonView>);
static_assert(requires(Data& data) {
    data.index;
    data.name;
    data.parameters;
    data.type;
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.index();
    view.get_name();
    view.get_parameters();
    view.get_arity();
    view.get_type();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
