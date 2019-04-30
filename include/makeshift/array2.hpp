
#ifndef INCLUDED_MAKESHIFT_ARRAY2_HPP_
#define INCLUDED_MAKESHIFT_ARRAY2_HPP_


#include <array>

#include <makeshift/detail/array2.hpp>


namespace makeshift
{

namespace detail
{


template <typename T> struct array_0_ { using type = T; };
template <typename T, std::size_t N> struct array_0_<T[N]> { using type = std::array<typename array_0_<T>::type, N>; };
template <typename ArrayT> struct array_;
template <typename T, std::size_t N> struct array_<T[N]> : array_0_<T[N]> { };

template <typename ArrayT>
    struct array_type_;
template <typename T, std::size_t N>
    struct array_type_<T[N]>
{
    static constexpr std::ptrdiff_t size = ptrdiff_t(N);
    using element_type = T;
};
template <typename T>
    struct array_type_<T[]>
{
    static constexpr std::ptrdiff_t size = -1;
    using element_type = T;
};


} // namespace detail


inline namespace types
{


    //ᅟ
    // `array<T[N]>` is an alias for `std::array<T, N>`.
    //
template <typename ArrayT> using array = typename makeshift::detail::array_<ArrayT>::type;


    //ᅟ
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns an array of the results of the function applied to the tuple elements.
    //ᅟ
    //ᅟ    auto indices = array_transform<3>(
    //ᅟ        [](std::size_t i) { return i; },
    //ᅟ        tuple_index);
    //ᅟ    // returns std::array{ 0, 1, 2 }
    //
template <std::size_t N, typename F, typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto
    array_transform2(F&& func, Ts&&... args)
{
    static_assert(makeshift::detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    return makeshift::detail::tuple_transform_impl0<N, makeshift::detail::transform_to_array_tag>(std::forward<F>(func), std::forward<Ts>(args)...);
}


    //ᅟ
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns an array of the results of the function applied to the tuple elements.
    //ᅟ
    //ᅟ    auto gridCoords = array_transform<double[3]>(
    //ᅟ        [dx = 1.0](mk::index i) { return i*dx; },
    //ᅟ        tuple_index);
    //ᅟ    // returns std::array{ 0.0, 1.0, 2.0 }
    //ᅟ
    //ᅟ    auto intSquares = array_transform<double[]>(
    //ᅟ        [](auto x) { return x*x; },
    //ᅟ        std::make_tuple(2.0, 3.0f));
    //ᅟ    // returns std::array{ 4.0, 9.0 }
    //
template <typename ArrayT, typename F, typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto
    array_transform2(F&& func, Ts&&... args)
{
    static_assert(makeshift::detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    return makeshift::detail::tuple_transform_impl0<makeshift::detail::array_type_<ArrayT>::size, makeshift::detail::transform_to_array_of_tag<typename makeshift::detail::array_type_<ArrayT>::element_type>>(
        std::forward<F>(func), std::forward<Ts>(args)...);
}


    //ᅟ
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns an array of the results of the function applied to the tuple elements.
    //ᅟ
    //ᅟ    auto intSquares = array_transform(
    //ᅟ        [](auto x) { return int(x*x); },
    //ᅟ        std::make_tuple(2, 3.0f));
    //ᅟ    // returns std::array{ 4, 9 }
    //
template <typename F, typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto
    array_transform2(F&& func, Ts&&... args)
{
    static_assert(makeshift::detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    return makeshift::detail::tuple_transform_impl0<-1, makeshift::detail::transform_to_array_tag>(std::forward<F>(func), std::forward<Ts>(args)...);
}


    //ᅟ
    // Takes a list of tuples and returns an array of concatenated elements.
    //ᅟ
    //ᅟ    auto numbers = std::tuple{ 2, 3 };
    //ᅟ    auto moreNumbers = std::array{ 6, 8 };
    //ᅟ    auto allNumbers = array_cat<int>(numbers, moreNumbers);
    //ᅟ    // returns { 2, 3, 6, 8 }
    //
template <typename T, typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto
    array_cat(Ts&&... tuples)
{
    static_assert(makeshift::detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    using Indices = makeshift::detail::indices_2d_<std::tuple_size<std::decay_t<Ts>>::value...>;
    return makeshift::detail::array_cat_impl<T>(std::make_index_sequence<Indices::size>{ }, Indices{ }, std::tuple<Ts&&...>{ std::forward<Ts>(tuples)... });
}


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_ARRAY2_HPP_
