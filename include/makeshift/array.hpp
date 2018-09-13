
#ifndef INCLUDED_MAKESHIFT_ARRAY_HPP_
#define INCLUDED_MAKESHIFT_ARRAY_HPP_


#include <type_traits> // for remove_cv<>, decay<>, index_sequence<>, integral_constant<>
#include <utility>     // for forward<>(), get<>, tuple_size<>
#include <cstddef>     // for size_t
#include <array>

#include <makeshift/tuple.hpp> // for is_tuple_like<>


namespace makeshift
{

namespace detail
{


template <typename ArrayT> struct array_size_;
template <typename T, std::size_t N> struct array_size_<std::array<T, N>> : std::integral_constant<std::size_t, N> { };
template <typename T, std::size_t N> struct array_size_<T (&)[N]> : std::integral_constant<std::size_t, N> { };
template <typename T, std::size_t N> struct array_size_<T (&&)[N]> : std::integral_constant<std::size_t, N> { };

template <typename ElemT, typename LArrayT, typename RArrayT, std::size_t... LIs, std::size_t... RIs>
    constexpr std::array<ElemT, sizeof...(LIs) + sizeof...(RIs)>
    concat_two_arrays(LArrayT&& lhs, RArrayT&& rhs, std::index_sequence<LIs...>, std::index_sequence<RIs...>)
{
    (void) lhs;
    (void) rhs;
    return {{ lhs[LIs]..., rhs[RIs]... }};
}

template <typename ElemT, typename ArrayT, typename TupleT, std::size_t N>
    constexpr auto concat_arrays(ArrayT&& initialValue, TupleT&&, std::integral_constant<std::size_t, N>, std::integral_constant<std::size_t, N>)
{
    return std::forward<ArrayT>(initialValue);
}
template <typename ElemT, typename ArrayT, typename TupleT, std::size_t I, std::size_t N>
    constexpr auto concat_arrays(ArrayT&& initialValue, TupleT&& tuple, std::integral_constant<std::size_t, I>, std::integral_constant<std::size_t, N>)
{
    using std::get; // make std::get<>(std::pair<>&&) visible to enable ADL for template methods named get<>()
    using LArray = std::decay_t<ArrayT>;
    using RArray = std::tuple_element_t<I, std::decay_t<TupleT>>;
    return concat_arrays<ElemT>(
        concat_two_arrays<ElemT>(std::forward<ArrayT>(initialValue), get<I>(std::forward<TupleT>(tuple)), std::make_index_sequence<array_size_<LArray>::value>{ }, std::make_index_sequence<array_size_<RArray>::value>{ }),
        std::forward<TupleT>(tuple),
        std::integral_constant<std::size_t, I + 1>{ }, std::integral_constant<std::size_t, N>{ }
    );
}

template <typename Is, typename TupleT> struct common_array_value_type_;
template <std::size_t... Is, typename TupleT> struct common_array_value_type_<std::index_sequence<Is...>, TupleT> : std::common_type<std::tuple_element_t<Is, TupleT>...> { };
template <typename TupleT> struct common_array_value_type : common_array_value_type_<std::make_index_sequence<std::tuple_size<TupleT>::value>, TupleT> { };
template <typename TupleT> using common_array_value_type_t = typename common_array_value_type<TupleT>::type;

template <typename T>
    struct array_cat_t : stream_base<array_cat_t<T>>
{
private:
    template <typename TupleT>
        static constexpr auto invoke(TupleT&& tuple)
    {
        constexpr std::size_t n = std::tuple_size<std::decay_t<TupleT>>::value;
        return concat_arrays<T>(std::array<T, 0>{ }, tuple, std::integral_constant<std::size_t, 0>{ }, std::integral_constant<std::size_t, n>{ });
    }
public:
    template <typename TupleT,
              typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
        constexpr auto operator ()(TupleT&& tuple) const
    {
        return invoke(std::forward<TupleT>(tuple));
    }
};
template <>
    struct array_cat_t<void> : stream_base<array_cat_t<void>>
{
private:
    template <typename TupleT>
        static constexpr auto invoke(TupleT&& tuple)
    {
        using Elem = common_array_value_type_t<std::decay_t<TupleT>>;
        constexpr std::size_t n = std::tuple_size<std::decay_t<TupleT>>::value;
        return concat_arrays<Elem>(std::array<Elem, 0>{ }, tuple, std::integral_constant<std::size_t, 0>{ }, std::integral_constant<std::size_t, n>{ });
    }
public:
    template <typename TupleT,
              typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
        constexpr auto operator ()(TupleT&& tuple) const
    {
        return invoke(std::forward<TupleT>(tuple));
    }
};

template <std::size_t N, typename T, std::size_t... Is>
    constexpr std::array<std::remove_cv_t<T>, N> to_array_impl(T array[], std::index_sequence<Is...>)
{
    (void) array;
    return {{ array[Is]... }};
}
template <typename T, typename TupleT, std::size_t... Is>
    constexpr std::array<std::remove_cv_t<T>, sizeof...(Is)> tuple_to_array_impl(TupleT&& tuple, std::index_sequence<Is...>)
{
    (void) tuple;
    using std::get; // make std::get<>(std::pair<>&&) visible to enable ADL for template methods named get<>()
    return {{ get<Is>(std::forward<TupleT>(tuple))... }};
}


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
    // Returns a functor that maps a tuple of arrays to an array of element type `T` that is initialized with the elements of the arrays in the tuple.
    // If `T` is not specified, the common type of the array element types is used.
    //ᅟ
    //ᅟ    auto tuple = std::make_tuple(std::array{ 1, 2 }, std::array{ 3, 4, 5 });
    //ᅟ    auto array = tuple
    //ᅟ        | array_cat(); // returns {{ 1, 2, 3, 4, 5 }}
    //
template <typename T = void>
    constexpr makeshift::detail::array_cat_t<std::remove_cv_t<T>>
    array_cat(void)
{
    return { };
}


    //ᅟ
    // Given a tuple of arrays, returns an array of the common type of the array element types that contains the concatenated values of the arrays in the tuple.
    //ᅟ
    //ᅟ    auto tuple = std::make_tuple(std::array{ 1, 2 }, std::array{ 3, 4, 5 });
    //ᅟ    auto array = array_cat(tuple); // returns {{ 1, 2, 3, 4, 5 }}
    //
template <typename TupleT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr auto array_cat(TupleT&& tuple)
{
    return array_cat()(std::forward<TupleT>(tuple));
}


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_ARRAY_HPP_
