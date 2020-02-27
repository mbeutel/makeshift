
#ifndef INCLUDED_MAKESHIFT_ARRAY_HPP_
#define INCLUDED_MAKESHIFT_ARRAY_HPP_


#include <gsl-lite/gsl-lite.hpp> // for gsl_NODISCARD, gsl_CPP17_OR_GREATER

#if !gsl_CPP17_OR_GREATER
# include <tuple>      // for tuple_size<>
#endif // !gsl_CPP17_OR_GREATER

#include <array>
#include <cstddef> // for size_t
#include <utility> // for integer_sequence<>, get<>(), tuple_size<> (C++17)

#include <makeshift/detail/array.hpp>


namespace makeshift {

namespace gsl = ::gsl_lite;


    //
    // `mdarray<T, N1, ..., Nd>` is an alias for `std::array<...std::array<T, Nd>..., N1>`, i.e. the modern equivalent of `T[N1]...[Nd]`.
    //ᅟ
    // Note that `mdarray<>` is defined as a dependent type, i.e. the type arguments of `mdarray<>` cannot be deduced.
    // TODO: remove, or move to experimental/?
    //
template <typename T, std::size_t... Dims>
using mdarray = typename detail::mdarray_<T, Dims...>::type;


    //
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns an array of the results of the function
    // applied to the tuple elements.
    //ᅟ
    //ᅟ    auto intSquares = array_transform(
    //ᅟ        [](auto x) { return int(x*x); },
    //ᅟ        std::make_tuple(2, 3.0f));
    //ᅟ    // returns std::array{ 4, 9 }
    //
template <typename F, typename... Ts>
gsl_NODISCARD constexpr auto
array_transform(F&& func, Ts&&... args)
{
    static_assert(detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = detail::tuple_transform_size<-1, Ts...>();
    return detail::array_transform_impl<std::array>(std::make_index_sequence<size>{ }, std::forward<F>(func), std::forward<Ts>(args)...);
}

    //
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns an array of the results of the function
    // applied to the tuple elements.
    //ᅟ
    //ᅟ    auto intSquares = array_transform<MyArray>(
    //ᅟ        [](auto x) { return int(x*x); },
    //ᅟ        std::make_tuple(2, 3.0f));
    //ᅟ    // returns MyArray<int, 2>{ 4, 9 }
    //
template <template <typename, std::size_t> class ArrayT, typename F, typename... Ts>
gsl_NODISCARD constexpr auto
array_transform(F&& func, Ts&&... args)
{
    static_assert(detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = detail::tuple_transform_size<-1, Ts...>();
    return detail::array_transform_impl<ArrayT>(std::make_index_sequence<size>{ }, std::forward<F>(func), std::forward<Ts>(args)...);
}

    //
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns an array of the results of the function
    // applied to the tuple elements.
    //ᅟ
    //ᅟ    auto intSquares = array_transform<double>(
    //ᅟ        [](auto x) { return x*x; },
    //ᅟ        std::make_tuple(2.0, 3.0f));
    //ᅟ    // returns std::array{ 4.0, 9.0 }
    //
template <typename T, typename F, typename... Ts>
gsl_NODISCARD constexpr auto
array_transform(F&& func, Ts&&... args)
{
    static_assert(detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = detail::tuple_transform_size<-1, Ts...>();
    return detail::array_transform_to_impl<std::array, T>(std::make_index_sequence<size>{ }, std::forward<F>(func), std::forward<Ts>(args)...);
}

    //
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns an array of the results of the function
    // applied to the tuple elements.
    //ᅟ
    //ᅟ    auto intSquares = array_transform<MyArray, double>(
    //ᅟ        [](auto x) { return x*x; },
    //ᅟ        std::make_tuple(2.0, 3.0f));
    //ᅟ    // returns MyArray<double, 2>{ 4.0, 9.0 }
    //
template <template <typename, std::size_t> class ArrayT, typename T, typename F, typename... Ts>
gsl_NODISCARD constexpr auto
array_transform(F&& func, Ts&&... args)
{
    static_assert(detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = detail::tuple_transform_size<-1, Ts...>();
    return detail::array_transform_to_impl<ArrayT, T>(std::make_index_sequence<size>{ }, std::forward<F>(func), std::forward<Ts>(args)...);
}


    //
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns an array of the results of the function
    // applied to the tuple elements.
    //ᅟ
    //ᅟ    auto indices = array_transform<3>(
    //ᅟ        [](std::size_t i) { return i; },
    //ᅟ        range_index);
    //ᅟ    // returns std::array{ 0, 1, 2 }
    //
template <std::size_t N, typename F, typename... Ts>
gsl_NODISCARD constexpr auto
array_transform(F&& func, Ts&&... args)
{
    static_assert(detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = detail::tuple_transform_size<N, Ts...>();
    return detail::array_transform_impl<std::array>(std::make_index_sequence<size>{ }, std::forward<F>(func), std::forward<Ts>(args)...);
}

    //
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns an array of the results of the function
    // applied to the tuple elements.
    //ᅟ
    //ᅟ    auto indices = array_transform<3, MyArray>(
    //ᅟ        [](std::size_t i) { return i; },
    //ᅟ        range_index);
    //ᅟ    // returns MyArray<int, 3>{ 0, 1, 2 }
    //
template <std::size_t N, template <typename, std::size_t> class ArrayT, typename F, typename... Ts>
gsl_NODISCARD constexpr auto
array_transform(F&& func, Ts&&... args)
{
    static_assert(detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = detail::tuple_transform_size<N, Ts...>();
    return detail::array_transform_impl<ArrayT>(std::make_index_sequence<size>{ }, std::forward<F>(func), std::forward<Ts>(args)...);
}

    //
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns an array of the results of the function
    // applied to the tuple elements.
    //ᅟ
    //ᅟ    auto gridCoords = array_transform<3, double>(
    //ᅟ        [dx = 1.0](mk::index i) { return i*dx; },
    //ᅟ        range_index);
    //ᅟ    // returns std::array{ 0.0, 1.0, 2.0 }
    //
template <std::size_t N, typename T, typename F, typename... Ts>
gsl_NODISCARD constexpr auto
array_transform(F&& func, Ts&&... args)
{
    static_assert(detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = detail::tuple_transform_size<N, Ts...>();
    return detail::array_transform_to_impl<std::array, T>(std::make_index_sequence<size>{ }, std::forward<F>(func), std::forward<Ts>(args)...);
}

    //
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns an array of the results of the function
    // applied to the tuple elements.
    //ᅟ
    //ᅟ    auto gridCoords = array_transform<3, MyArray, double>(
    //ᅟ        [dx = 1.0](mk::index i) { return i*dx; },
    //ᅟ        range_index);
    //ᅟ    // returns MyArray<double, 3>{ 0.0, 1.0, 2.0 }
    //
template <std::size_t N, template <typename, std::size_t> class ArrayT, typename T, typename F, typename... Ts>
gsl_NODISCARD constexpr auto
array_transform(F&& func, Ts&&... args)
{
    static_assert(detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = detail::tuple_transform_size<N, Ts...>();
    return detail::array_transform_to_impl<ArrayT, T>(std::make_index_sequence<size>{ }, std::forward<F>(func), std::forward<Ts>(args)...);
}


    //
    // Returns an array of size `N` filled with the given value.
    //ᅟ
    //ᅟ    auto indices = array_fill<3>(1);
    //ᅟ    // returns std::array{ 1, 1, 1 }
    //
template <std::size_t N, typename T>
gsl_NODISCARD constexpr std::array<T, N>
array_fill(T const& value)
{
    return detail::array_transform_to_impl<std::array, T>(std::make_index_sequence<N>{ }, detail::fill_fn<T>{ value });
}

    //
    // Returns an array of size `N` filled with the given value.
    //ᅟ
    //ᅟ    auto indices = array_fill<3, MyArray>(1);
    //ᅟ    // returns MyArray<int, 3>{ 1, 1, 1 }
    //
template <std::size_t N, template <typename, std::size_t> class ArrayT, typename T>
gsl_NODISCARD constexpr auto
array_fill(T const& value)
{
    return detail::array_transform_to_impl<ArrayT, T>(std::make_index_sequence<N>{ }, detail::fill_fn<T>{ value });
}


    //
    // Takes a list of tuples and returns an array of concatenated elements.
    //ᅟ
    //ᅟ    auto numbers = std::tuple{ 2, 3 };
    //ᅟ    auto moreNumbers = std::array{ 6, 8 };
    //ᅟ    auto allNumbers = array_cat<int>(numbers, moreNumbers);
    //ᅟ    // returns std::array{ 2, 3, 6, 8 }
    //
template <typename T, typename... Ts>
gsl_NODISCARD constexpr auto
array_cat(Ts&&... tuples)
{
    static_assert(detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");

    using Indices = detail::indices_2d_<std::tuple_size<std::decay_t<Ts>>::value...>;
    return detail::array_cat_impl<std::array, T, Indices>(std::make_index_sequence<Indices::size>{ }, std::tuple<Ts&&...>{ std::forward<Ts>(tuples)... });
}

    //
    // Takes a list of tuples and returns an array of concatenated elements.
    // The array is constructed using the given array template.
    //ᅟ
    //ᅟ    auto numbers = std::tuple{ 2, 3 };
    //ᅟ    auto moreNumbers = std::array{ 6, 8 };
    //ᅟ    auto allNumbers = array_cat<MyArray, int>(numbers, moreNumbers);
    //ᅟ    // returns MyArray<int, 4>{ 2, 3, 6, 8 }
    //
template <template <typename, std::size_t> class ArrayT, typename T, typename... Ts>
gsl_NODISCARD constexpr auto
array_cat(Ts&&... tuples)
{
    static_assert(detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");

    using Indices = detail::indices_2d_<std::tuple_size<std::decay_t<Ts>>::value...>;
    return detail::array_cat_impl<ArrayT, T, Indices>(std::make_index_sequence<Indices::size>{ }, std::tuple<Ts&&...>{ std::forward<Ts>(tuples)... });
}


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_ARRAY_HPP_
