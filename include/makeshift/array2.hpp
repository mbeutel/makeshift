
#ifndef INCLUDED_MAKESHIFT_ARRAY2_HPP_
#define INCLUDED_MAKESHIFT_ARRAY2_HPP_


#include <array>

#include <makeshift/detail/array2.hpp>


namespace makeshift
{


    //ᅟ
    // Pass `array_index` to `array_transform()` to have the tuple element index passed as a functor argument.
    // The argument is of type `integral_constant<std::size_t, I>` and implicitly converts to `std::size_t`.
    // Note that `tuple_index_t` and `array_index_t` are aliases to the same type.
    //ᅟ
    //ᅟ    auto indices = array_transform<3>(
    //ᅟ        [](std::size_t i) { return i; },
    //ᅟ        array_index);
    //ᅟ    // returns std::array{ 0, 1, 2 }
    //
using array_index_t = makeshift::detail::tuple_index_t;


    //ᅟ
    // Pass `array_index` to `array_transform()` to have the tuple element index passed as a functor argument.
    // The argument is of type `integral_constant<std::size_t, I>` and implicitly converts to `std::size_t`.
    // Note that `tuple_index_t` and `array_index_t` are aliases to the same type.
    //ᅟ
    //ᅟ    auto indices = array_transform<3>(
    //ᅟ        [](std::size_t i) { return i; },
    //ᅟ        array_index);
    //ᅟ    // returns std::array{ 0, 1, 2 }
    //
constexpr array_index_t array_index{ };



    //ᅟ
    // `array<T, N1, ..., Nd>` is an alias for `std::array<...std::array<T, Nd>..., N1>`, i.e. the modern equivalent of `T[N1]...[Nd]`.
    // Note that `array<>` is defined such that the type arguments of `array<>` cannot be deduced.
    //
template <typename T, std::size_t... Dims> using array = typename makeshift::detail::array_<T, Dims...>::type;


    //ᅟ
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns an array of the results of the function applied to the tuple elements.
    //ᅟ
    //ᅟ    auto indices = array_transform<3>(
    //ᅟ        [](std::size_t i) { return i; },
    //ᅟ        array_index);
    //ᅟ    // returns std::array{ 0, 1, 2 }
    //
template <std::size_t N, typename F, typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto
    array_transform(F&& func, Ts&&... args)
{
    static_assert(makeshift::detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    return makeshift::detail::tuple_transform_impl0<N, makeshift::detail::transform_to_array_tag>(std::forward<F>(func), std::forward<Ts>(args)...);
}


    //ᅟ
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns an array of the results of the function applied to the tuple elements.
    //ᅟ
    //ᅟ    auto intSquares = array_transform<double>(
    //ᅟ        [](auto x) { return x*x; },
    //ᅟ        std::make_tuple(2.0, 3.0f));
    //ᅟ    // returns std::array{ 4.0, 9.0 }
    //
template <typename T, typename F, typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto
    array_transform(F&& func, Ts&&... args)
{
    static_assert(makeshift::detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    return makeshift::detail::tuple_transform_impl0<-1, makeshift::detail::transform_to_array_of_tag<T>>(std::forward<F>(func), std::forward<Ts>(args)...);
}


    //ᅟ
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns an array of the results of the function applied to the tuple elements.
    //ᅟ
    //ᅟ    auto gridCoords = array_transform<double, 3>(
    //ᅟ        [dx = 1.0](mk::index i) { return i*dx; },
    //ᅟ        array_index);
    //ᅟ    // returns std::array{ 0.0, 1.0, 2.0 }
    //
template <typename T, std::size_t N, typename F, typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto
    array_transform(F&& func, Ts&&... args)
{
    static_assert(makeshift::detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    return makeshift::detail::tuple_transform_impl0<N, makeshift::detail::transform_to_array_of_tag<T>>(std::forward<F>(func), std::forward<Ts>(args)...);
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
    array_transform(F&& func, Ts&&... args)
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


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_ARRAY2_HPP_
