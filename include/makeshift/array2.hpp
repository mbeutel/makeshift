
#ifndef INCLUDED_MAKESHIFT_ARRAY2_HPP_
#define INCLUDED_MAKESHIFT_ARRAY2_HPP_


#include <array>

#include <makeshift/detail/array2.hpp>


namespace makeshift
{


    //ᅟ
    // Pass `array_index` to `array_transform()`, `tuple_foreach()`, or `tuple_transform()` to have the array element index passed as a functor argument.
    // The argument is of type `index`.
    //ᅟ
    //ᅟ    auto indices = array_transform<3>(
    //ᅟ        [](index i) { return i; },
    //ᅟ        array_index);
    //ᅟ    // returns std::array<index, 3>{ 0, 1, 2 }
    //
using array_index_t = makeshift::detail::array_index_t;


    //ᅟ
    // Pass `array_index` to `array_transform()`, `tuple_foreach()`, or `tuple_transform()` to have the array element index passed as a functor argument.
    // The argument is of type `index`.
    //ᅟ
    //ᅟ    auto indices = array_transform<3>(
    //ᅟ        [](index i) { return i; },
    //ᅟ        array_index);
    //ᅟ    // returns std::array<index, 3>{ 0, 1, 2 }
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
    constexpr std::size_t size = makeshift::detail::tuple_transform_size<-1, Ts...>();
    return makeshift::detail::array_transform_impl<std::array>(std::make_index_sequence<size>{ }, std::forward<F>(func), std::forward<Ts>(args)...);
}


    //ᅟ
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns an array of the results of the function applied to the tuple elements.
    //ᅟ
    //ᅟ    auto intSquares = array_transform<MyArray>(
    //ᅟ        [](auto x) { return int(x*x); },
    //ᅟ        std::make_tuple(2, 3.0f));
    //ᅟ    // returns MyArray<int, 2>{ 4, 9 }
    //
template <template <typename, std::size_t> class ArrayT, typename F, typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto
    array_transform(F&& func, Ts&&... args)
{
    static_assert(makeshift::detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = makeshift::detail::tuple_transform_size<-1, Ts...>();
    return makeshift::detail::array_transform_impl<ArrayT>(std::make_index_sequence<size>{ }, std::forward<F>(func), std::forward<Ts>(args)...);
}


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
    constexpr std::size_t size = makeshift::detail::tuple_transform_size<N, Ts...>();
    return makeshift::detail::array_transform_impl<std::array>(std::make_index_sequence<size>{ }, std::forward<F>(func), std::forward<Ts>(args)...);
}


    //ᅟ
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns an array of the results of the function applied to the tuple elements.
    //ᅟ
    //ᅟ    auto indices = array_transform<MyArray, 3>(
    //ᅟ        [](std::size_t i) { return i; },
    //ᅟ        array_index);
    //ᅟ    // returns MyArray<int, 3>{ 0, 1, 2 }
    //
template <template <typename, std::size_t> class ArrayT, std::size_t N, typename F, typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto
    array_transform(F&& func, Ts&&... args)
{
    static_assert(makeshift::detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = makeshift::detail::tuple_transform_size<N, Ts...>();
    return makeshift::detail::array_transform_impl<ArrayT>(std::make_index_sequence<size>{ }, std::forward<F>(func), std::forward<Ts>(args)...);
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
    constexpr std::size_t size = makeshift::detail::tuple_transform_size<-1, Ts...>();
    return makeshift::detail::array_transform_to_impl<std::array, T>(std::make_index_sequence<size>{ }, std::forward<F>(func), std::forward<Ts>(args)...);
}


    //ᅟ
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns an array of the results of the function applied to the tuple elements.
    //ᅟ
    //ᅟ    auto intSquares = array_transform<MyArray, double>(
    //ᅟ        [](auto x) { return x*x; },
    //ᅟ        std::make_tuple(2.0, 3.0f));
    //ᅟ    // returns MyArray<double, 2>{ 4.0, 9.0 }
    //
template <template <typename, std::size_t> class ArrayT, typename T, typename F, typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto
    array_transform(F&& func, Ts&&... args)
{
    static_assert(makeshift::detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = makeshift::detail::tuple_transform_size<-1, Ts...>();
    return makeshift::detail::array_transform_to_impl<ArrayT, T>(std::make_index_sequence<size>{ }, std::forward<F>(func), std::forward<Ts>(args)...);
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
    constexpr std::size_t size = makeshift::detail::tuple_transform_size<N, Ts...>();
    return makeshift::detail::array_transform_to_impl<std::array, T>(std::make_index_sequence<size>{ }, std::forward<F>(func), std::forward<Ts>(args)...);
}


    //ᅟ
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns an array of the results of the function applied to the tuple elements.
    //ᅟ
    //ᅟ    auto gridCoords = array_transform<MyArray, double, 3>(
    //ᅟ        [dx = 1.0](mk::index i) { return i*dx; },
    //ᅟ        array_index);
    //ᅟ    // returns MyArray<double, 3>{ 0.0, 1.0, 2.0 }
    //
template <template <typename, std::size_t> class ArrayT, typename T, std::size_t N, typename F, typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto
    array_transform(F&& func, Ts&&... args)
{
    static_assert(makeshift::detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = makeshift::detail::tuple_transform_size<N, Ts...>();
    return makeshift::detail::array_transform_impl<ArrayT, T>(std::make_index_sequence<size>{ }, std::forward<F>(func), std::forward<Ts>(args)...);
}


    //ᅟ
    // Takes a list of tuples and returns an array of concatenated elements.
    //ᅟ
    //ᅟ    auto numbers = std::tuple{ 2, 3 };
    //ᅟ    auto moreNumbers = std::array{ 6, 8 };
    //ᅟ    auto allNumbers = array_cat<int>(numbers, moreNumbers);
    //ᅟ    // returns std::array{ 2, 3, 6, 8 }
    //
template <typename T, typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto
    array_cat(Ts&&... tuples)
{
    static_assert(makeshift::detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    using Indices = makeshift::detail::indices_2d_<std::tuple_size<std::decay_t<Ts>>::value...>;
    return makeshift::detail::array_cat_impl<std::array, T>(std::make_index_sequence<Indices::size>{ }, Indices{ }, std::tuple<Ts&&...>{ std::forward<Ts>(tuples)... });
}


    //ᅟ
    // Takes a list of tuples and returns an array of concatenated elements.
    // The array is constructed using the given array template.
    //ᅟ
    //ᅟ    auto numbers = std::tuple{ 2, 3 };
    //ᅟ    auto moreNumbers = std::array{ 6, 8 };
    //ᅟ    auto allNumbers = array_cat<MyArray, int>(numbers, moreNumbers);
    //ᅟ    // returns MyArray<int, 4>{ 2, 3, 6, 8 }
    //
template <template <typename, std::size_t> class ArrayT, typename T, typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto
    array_cat(Ts&&... tuples)
{
    static_assert(makeshift::detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    using Indices = makeshift::detail::indices_2d_<std::tuple_size<std::decay_t<Ts>>::value...>;
    return makeshift::detail::array_cat_impl<ArrayT, T>(std::make_index_sequence<Indices::size>{ }, Indices{ }, std::tuple<Ts&&...>{ std::forward<Ts>(tuples)... });
}


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_ARRAY2_HPP_
