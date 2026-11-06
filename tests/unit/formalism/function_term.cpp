#include "loki/formalism/function_term_data.hpp"
#include "loki/formalism/function_term_view.hpp"
#include "loki/formalism/repository.hpp"

#include <concepts>

using Index = urd::index<loki::formalism::FunctionTerm>;
using Data = urd::data<loki::formalism::FunctionTerm>;
using View = urd::view<loki::formalism::FunctionTerm, loki::formalism::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, loki::formalism::FunctionTermView>);
static_assert(requires(Data& data) {
    data.index;
    data.function;
    data.terms;
    { data == data } -> std::same_as<bool>;
});
static_assert(requires(const View& view) {
    view.index();
    view.get_function();
    view.get_terms();
    { view == view } -> std::same_as<bool>;
    { view < view } -> std::same_as<bool>;
});
