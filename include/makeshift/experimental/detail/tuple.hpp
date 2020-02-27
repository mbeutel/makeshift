
#ifndef INCLUDED_MAKESHIFT_EXPERIMENTAL_DETAIL_TUPLE_HPP_
#define INCLUDED_MAKESHIFT_EXPERIMENTAL_DETAIL_TUPLE_HPP_


#include <tuple>       // for make_tuple()
#include <cstddef>     // for size_t
#include <utility>     // for forward<>(), integer_sequence<>, get<>()
#include <type_traits> // for decay<>, integral_constant<>


namespace makeshift {

namespace detail {


template <typename TupleT, typename DefaultT, std::size_t I>
constexpr decltype(auto)
single_or_default(TupleT&& tuple, DefaultT&&, std::integral_constant<std::ptrdiff_t, I>) noexcept
{
    using std::get;
    return get<I>(std::forward<TupleT>(tuple));
}
template <typename TupleT, typename DefaultT>
constexpr DefaultT&&
single_or_default(TupleT&&, DefaultT&& _default, std::integral_constant<std::ptrdiff_t, element_not_found>) noexcept
{
    return std::forward<DefaultT>(_default);
}


template <std::size_t I, typename TupleT, typename T, std::size_t J>
constexpr decltype(auto) get_or_replace(TupleT&& tuple, T&&, std::integral_constant<std::size_t, J>) noexcept
{
    using std::get;
    return get<J>(std::forward<TupleT>(tuple));
}
template <std::size_t I, typename TupleT, typename T>
constexpr T&& get_or_replace(TupleT&&, T&& newElement, std::integral_constant<std::size_t, I>) noexcept
{
    return std::forward<T>(newElement);
}

template <std::size_t I, typename NewT, std::size_t J, typename TupleT> struct replace_element : std::tuple_element<I, TupleT> { };
template <std::size_t I, typename NewT, typename TupleT> struct replace_element<I, NewT, I, TupleT> { using type = NewT; };

template <std::size_t I, typename NewT, typename TupleT, typename Is> struct with_element_0;
template <std::size_t I, typename NewT, template <typename...> class TupleT, typename... Ts, std::size_t... Is> struct with_element_0<I, NewT, TupleT<Ts...>, std::index_sequence<Is...>> { using type = TupleT<typename replace_element<I, NewT, Is, TupleT<Ts...>>::type...>; };
template <std::size_t I, typename NewT, typename TupleT> struct with_element : with_element_0<I, NewT, TupleT, std::make_index_sequence<std::tuple_size<TupleT>::value>> { };

template <std::size_t I, typename TupleT, typename T, std::size_t... Is>
constexpr typename with_element<I, std::decay_t<T>, std::decay_t<TupleT>>::type
with(TupleT&& tuple, T&& newElement, std::index_sequence<Is...>)
{
    using RTuple = typename with_element<I, std::decay_t<T>, std::decay_t<TupleT>>::type;
    return RTuple{ detail::get_or_replace<I>(std::forward<TupleT>(tuple), std::forward<T>(newElement), std::integral_constant<std::size_t, Is>{ })... };
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_EXPERIMENTAL_DETAIL_TUPLE_HPP_
