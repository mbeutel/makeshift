
#ifndef MAKESHIFT_ARRAY_HPP_
#define MAKESHIFT_ARRAY_HPP_


#include <type_traits> // for remove_cv<>, decay<>, index_sequence<>, integral_constant<>
#include <utility>     // for forward<>(), get<>, tuple_size<>
#include <cstddef>     // for size_t
#include <array>

#include <makeshift/tuple.hpp> // for is_tuple_like<>


namespace makeshift
{

namespace detail
{


template <typename T>
    struct to_array_t : stream_base<to_array_t<T>>
{
private:
    template <std::size_t... Is, typename TupleT>
        static constexpr std::array<T, sizeof...(Is)> invoke(std::index_sequence<Is...>, TupleT&& tuple)
    {
        using std::get; // make std::get<>(std::pair<>&&) visible to enable ADL for template methods named get<>()
        return {{ get<Is>(std::forward<TupleT>(tuple))... }};
    }
public:
    template <typename TupleT,
              typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
        constexpr auto operator ()(TupleT&& tuple) const
    {
        return invoke(std::make_index_sequence<std::tuple_size<std::decay_t<TupleT>>::value>{ }, std::forward<TupleT>(tuple));
    }
};
template <>
    struct to_array_t<void> : stream_base<to_array_t<void>>
{
private:
    template <typename... Ts, std::size_t... Is, typename TupleT>
        static constexpr std::array<std::common_type_t<Ts...>, sizeof...(Is)> invoke(type_sequence<Ts...>, std::index_sequence<Is...>, TupleT&& tuple)
    {
        using std::get; // make std::get<>(std::pair<>&&) visible to enable ADL for template methods named get<>()
        return {{ get<Is>(std::forward<TupleT>(tuple))... }};
    }
public:
    template <typename TupleT,
              typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
        constexpr auto operator ()(TupleT&& tuple) const
    {
        return invoke(args_sequence_of<std::decay_t<TupleT>>{ }, std::make_index_sequence<std::tuple_size<std::decay_t<TupleT>>::value>{ }, std::forward<TupleT>(tuple));
    }
};

template <std::size_t N, typename T, std::size_t... Is>
    constexpr std::array<std::remove_cv_t<T>, N> to_array_impl(T array[], std::index_sequence<Is...>)
{
    return {{ array[Is]... }};
}
template <typename T, typename TupleT, std::size_t... Is>
    constexpr std::array<std::remove_cv_t<T>, sizeof...(Is)> tuple_to_array_impl(TupleT&& tuple, std::index_sequence<Is...>)
{
    using std::get; // make std::get<>(std::pair<>&&) visible to enable ADL for template methods named get<>()
    return {{ get<Is>(std::forward<TupleT>(tuple))... }};
}

template <typename ArrayT> struct array_size_;
template <typename T, std::size_t N> struct array_size_<std::array<T, N>> : std::integral_constant<std::size_t, N> { };
template <typename T, std::size_t N> struct array_size_<T (&)[N]> : std::integral_constant<std::size_t, N> { };
template <typename T, std::size_t N> struct array_size_<T (&&)[N]> : std::integral_constant<std::size_t, N> { };


} // namespace detail


inline namespace types
{


    //ᅟ
    // Determines the size of the given array.
    //
template <typename ArrayT> struct array_size : makeshift::detail::array_size_<std::decay_t<ArrayT>> { };

    //ᅟ
    // Determines the size of the given array.
    //
template <typename ArrayT> static constexpr std::size_t array_size_v = array_size<ArrayT>::value;


    //ᅟ
    // Converts an array to `std::array<>`.
    //ᅟ
    //ᅟ    int nativeArray[] = { 1, 2, 3 };
    //ᅟ    auto array = to_array(nativeArray); // returns {{ 1, 2, 3 }}
    //
template <std::size_t N, typename T>
    constexpr std::array<std::remove_cv_t<T>, N>
    to_array(T (&array)[N])
{
    return makeshift::detail::to_array_impl(array, std::make_index_sequence<N>{ });
}


    //ᅟ
    // Returns a functor that maps a tuple to an array of element type `T` that is initialized with the elements in the tuple.
    // If `T` is not specified, the common type of the tuple element types is used.
    //ᅟ
    //ᅟ    auto tuple = std::make_tuple(1, 2, 3);
    //ᅟ    auto array = tuple
    //ᅟ        | to_array(); // returns {{ 1, 2, 3 }}
    //
template <typename T = void>
    constexpr makeshift::detail::to_array_t<std::remove_cv_t<T>>
    to_array(void)
{
    return { };
}


    //ᅟ
    // Returns a `std::array<>` of element type `T` that is initialized with the elements in the tuple.
    //ᅟ
    //ᅟ    auto tuple = std::make_tuple(1, 2, 3);
    //ᅟ    auto array = to_array(tuple); // returns {{ 1, 2, 3 }}
    //
template <typename T, typename TupleT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr std::array<std::remove_cv_t<T>, std::tuple_size<std::decay_t<TupleT>>::value>
    to_array(TupleT&& tuple)
{
    return to_array<T>()(std::forward<TupleT>(tuple));
}


} // inline namespace types

} // namespace makeshift


#endif // MAKESHIFT_ARRAY_HPP_
