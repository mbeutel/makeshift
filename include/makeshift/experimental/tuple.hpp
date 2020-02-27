
#ifndef INCLUDED_MAKESHIFT_EXPERIMENTAL_TUPLE_HPP_
#define INCLUDED_MAKESHIFT_EXPERIMENTAL_TUPLE_HPP_


#include <cstddef>     // for size_t, ptrdiff_t
#include <utility>     // for forward<>()
#include <type_traits> // for decay<>, remove_reference<>, enable_if<>

#include <gsl-lite/gsl-lite.hpp> // for gsl_NODISCARD

#include <makeshift/type_traits.hpp> // for is_tuple_like<>

#include <makeshift/experimental/detail/tuple.hpp>


namespace makeshift {

namespace gsl = ::gsl_lite;


    //
    // Returns the tuple element for which the given constval predicate holds.
    //ᅟ
    //ᅟ    auto t = std::tuple{ 1, 2.0 };
    //ᅟ    int i = single(t, []<T>(T const&) { return std::is_integral<T>{ }; }); // returns 1
    //
template <typename TupleT, typename CPredT,
          std::enable_if_t<is_tuple_like_v<TupleT>, int> = 0>
gsl_NODISCARD constexpr decltype(auto)
single(TupleT&& tuple, CPredT /*where*/) noexcept
{
    constexpr std::ptrdiff_t index = detail::search_type_pack_index<std::remove_reference_t<TupleT>, CPredT>::value;
    static_assert(index != detail::element_not_found, "no element in the tuple matches the given predicate");
    static_assert(index != detail::element_not_unique, "more than one element in the tuple matches the given predicate");

    using std::get;
    return get<index>(std::forward<TupleT>(tuple));
}


    //
    // Returns the tuple element for which the given constval predicate holds, or a given default value if none exists.
    //ᅟ
    //ᅟ    auto t = std::tuple{ 1, 2.0 };
    //ᅟ    int i = single_or_default(t, []<T>(T) { return std::is_integral<T>{ }; }); // returns 1
    //
template <typename TupleT, typename CPredT, typename DefaultT,
          std::enable_if_t<is_tuple_like_v<TupleT>, int> = 0>
gsl_NODISCARD constexpr decltype(auto)
single_or_default(TupleT&& tuple, CPredT /*where*/, DefaultT&& _default) noexcept
{
    constexpr std::ptrdiff_t index = detail::search_type_pack_index<std::remove_reference_t<TupleT>, CPredT>::value;
    static_assert(index != detail::element_not_unique, "more than one element in the tuple matches the given predicate");

    return detail::single_or_default(std::forward<TupleT>(tuple), std::forward<DefaultT>(_default), std::integral_constant<std::ptrdiff_t, index>{ });
}


    //
    // Returns a new tuple where the `I`-th element was substituted with `newElement`.
    //ᅟ
    //ᅟ    auto t = std::tuple{ 1, 2.0 };
    //ᅟ    auto u = with<1>(t, 3.0f); // returns std::tuple{ 1, 3.0f }
    //
template <std::size_t I, typename TupleT, typename NewElementT,
          std::enable_if_t<is_tuple_like_v<TupleT>, int> = 0>
gsl_NODISCARD constexpr typename detail::with_element<I, std::decay_t<NewElementT>, std::decay_t<TupleT>>::type
with(TupleT&& tuple, NewElementT&& newElement)
{
    return detail::with<I>(std::forward<TupleT>(tuple), std::forward<NewElementT>(newElement), std::make_index_sequence<std::tuple_size<std::remove_reference_t<TupleT>>::value>{ });
}

    //
    // Returns a new tuple where the element of type `T` was substituted with `newElement`.
    //ᅟ
    //ᅟ    auto t = std::tuple{ 1, 2.0 };
    //ᅟ    auto u = with<int>(t, 3.0f); // returns std::tuple{ 1, 3.0f }
    //
template <typename T, typename TupleT, typename NewElementT,
          std::enable_if_t<is_tuple_like_v<TupleT>, int> = 0>
gsl_NODISCARD constexpr auto
with(TupleT&& tuple, NewElementT&& newElement)
{
    constexpr std::size_t index = tuple_element_index_v<T, std::remove_reference_t<TupleT>>;
    return makeshift::with<index>(std::forward<TupleT>(tuple), std::forward<NewElementT>(newElement));
}

    //
    // Returns a new tuple where the element for which the given constval predicate holds was substituted with `newElement`.
    //ᅟ
    //ᅟ    auto t = std::tuple{ 1, 2.0 };
    //ᅟ    auto u = with(t, []<T>(T) { return std::is_integral<T>{ }; }, 3.0f); // returns std::tuple{ 3.0f, 2.0 }
    //
template <typename TupleT, typename CPredT, typename T>
gsl_NODISCARD constexpr auto
with(TupleT&& tuple, CPredT /*where*/, T&& newElement)
{
    constexpr std::size_t index = detail::tuple_element_index<std::remove_reference_t<TupleT>, CPredT>::value;
    return makeshift::with<index>(std::forward<TupleT>(tuple), std::forward<T>(newElement));
}


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_EXPERIMENTAL_TUPLE_HPP_
