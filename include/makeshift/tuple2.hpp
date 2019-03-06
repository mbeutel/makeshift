
#ifndef INCLUDED_MAKESHIFT_TUPLE2_HPP_
#define INCLUDED_MAKESHIFT_TUPLE2_HPP_


#include <array>
#include <tuple>       // for tuple<>, tuple_cat()
#include <cstddef>     // for size_t
#include <utility>     // for forward<>()
#include <type_traits> // for decay<>

#include <makeshift/type_traits2.hpp> // for can_apply<>
#include <makeshift/version.hpp>      // for MAKESHIFT_NODISCARD

#include <makeshift/detail/tuple2.hpp>


namespace makeshift
{

inline namespace types
{


    //ᅟ
    // Determines whether a type has a tuple-like interface (i.e. whether `std::tuple_size<T>::value` is well-formed).
    //
template <typename T> struct is_tuple_like : can_apply<makeshift::detail::is_tuple_like_r, T> { };

    //ᅟ
    // Determines whether a type has a tuple-like interface (i.e. whether `std::tuple_size<T>::value` is well-formed).
    //
template <typename T> constexpr bool is_tuple_like_v = is_tuple_like<T>::value;


    //ᅟ
    // Pass `tuple_index` to `tuple_foreach()` or `tuple_transform()` to have the tuple element index passed as a functor argument.
    // The argument is of type `integral_constant<std::size_t, I>` and implicitly converts to `std::size_t`.
    //ᅟ
    //ᅟ    tuple_foreach(
    //ᅟ        [](auto element, std::size_t idx) { std::cout << idx << ": " << element << '\n'; },
    //ᅟ        std::make_tuple(42, 1.41421), tuple_index);
    //ᅟ    // prints "0: 42\n1: 1.41421"
    //
constexpr tuple_index_t tuple_index{ };


    //ᅟ
    // Takes a scalar procedure (i.e. a function of non-tuple arguments which returns nothing) and calls the procedure for every element in the given tuples.
    //ᅟ
    //ᅟ    tuple_foreach<3>(
    //ᅟ        [](std::size_t i) { std::cout << i << '\n'; },
    //ᅟ        tuple_index
    //ᅟ    ); // prints "0\n1\n2\n"
    //
template <std::size_t N, typename F, typename... Ts>
    constexpr void
    tuple_foreach2(F&& func, Ts&&... args)
{
    static_assert(makeshift::detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    makeshift::detail::tuple_transform_impl0<N, makeshift::detail::transform_target::nothing>(std::forward<F>(func), std::forward<Ts>(args)...);
}


    //ᅟ
    // Takes a scalar procedure (i.e. a function of non-tuple arguments which returns nothing) and calls the procedure for every element in the given tuples.
    //ᅟ
    //ᅟ    tuple_foreach(
    //ᅟ        [](auto name, auto elem) { std::cout << name << ": " << elem << '\n'; },
    //ᅟ        std::make_tuple("a", "b"), std::make_tuple(1, 2.3f));
    //ᅟ    // prints "a: 1\nb: 2.3\n"
    //
template <typename F, typename... Ts>
    constexpr void
    tuple_foreach2(F&& func, Ts&&... args)
{
    static_assert(makeshift::detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    makeshift::detail::tuple_transform_impl0<-1, makeshift::detail::transform_target::nothing>(std::forward<F>(func), std::forward<Ts>(args)...);
}


    //ᅟ
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns a tuple of the results of the function applied to the tuple elements.
    //ᅟ
    //ᅟ    auto indices = tuple_transform<3>(
    //ᅟ        [](std::size_t i) { return i; },
    //ᅟ        tuple_index);
    //ᅟ    // returns std::tuple{ 0, 1, 2 }
    //
template <std::size_t N, typename F, typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto
    tuple_transform2(F&& func, Ts&&... args)
{
    static_assert(makeshift::detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    return makeshift::detail::tuple_transform_impl0<N, makeshift::detail::transform_target::tuple>(std::forward<F>(func), std::forward<Ts>(args)...);
}


    //ᅟ
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns a tuple of the results of the function applied to the tuple elements.
    //ᅟ
    //ᅟ    auto squares = tuple_transform(
    //ᅟ        [](auto x) { return x*x; },
    //ᅟ        std::make_tuple(2, 3.0f));
    //ᅟ    // returns std::tuple{ 4, 9.0f }
    //
template <typename F, typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto
    tuple_transform2(F&& func, Ts&&... args)
{
    static_assert(makeshift::detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    return makeshift::detail::tuple_transform_impl0<-1, makeshift::detail::transform_target::tuple>(std::forward<F>(func), std::forward<Ts>(args)...);
}


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
    return makeshift::detail::tuple_transform_impl0<N, makeshift::detail::transform_target::array>(std::forward<F>(func), std::forward<Ts>(args)...);
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
    return makeshift::detail::tuple_transform_impl0<-1, makeshift::detail::transform_target::array>(std::forward<F>(func), std::forward<Ts>(args)...);
}


    //ᅟ
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns a sequence of the result types of the function applied to the tuple elements.
    //ᅟ
    //ᅟ    auto indices = type_sequence_transform<3>(
    //ᅟ        [](auto i) { return i; },
    //ᅟ        tuple_index);
    //ᅟ    // returns type_sequence<integral_constant<std::size_t, 0>, integral_constant<std::size_t, 1>, integral_constant<std::size_t, 2>>
    //
template <std::size_t N, typename F, typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto
    type_sequence_transform2(F&& func, Ts&&... args)
{
    static_assert(makeshift::detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    return makeshift::detail::tuple_transform_impl0<N, makeshift::detail::transform_target::type_sequence>(std::forward<F>(func), std::forward<Ts>(args)...);
}


    //ᅟ
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns a sequence of the result types of the function applied to the tuple elements.
    //ᅟ
    //ᅟ    auto squares = type_sequence_transform(
    //ᅟ        [](auto x) { return x*x; },
    //ᅟ        std::make_tuple(2, 3.0f));
    //ᅟ    // returns type_sequence<int, float>
    //
template <typename F, typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto
    type_sequence_transform2(F&& func, Ts&&... args)
{
    static_assert(makeshift::detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    return makeshift::detail::tuple_transform_impl0<-1, makeshift::detail::transform_target::type_sequence>(std::forward<F>(func), std::forward<Ts>(args)...);
}


    //ᅟ
    // Takes a tuple, an initial value, and a binary accumulator function and returns the left fold of the tuple.
    //ᅟ
    //ᅟ    auto numbers = std::make_tuple(2, 3u);
    //ᅟ    int sum = tuple_reduce(numbers, 0, std::plus<int>{ }); // returns 5
    //
template <typename TupleT, typename T, typename F>
    MAKESHIFT_NODISCARD constexpr auto
    tuple_reduce(TupleT&& tuple, T&& initialValue, F&& func)
{
    static_assert(is_tuple_like_v<std::decay_t<TupleT>>, "first argument must be tuple or tuple-like type");
    return makeshift::detail::fold_impl(makeshift::detail::left_fold{ },
        std::forward<TupleT>(tuple), std::forward<T>(initialValue), std::forward<F>(func));
}


    //ᅟ
    // Takes a tuple, an initial value, and a binary accumulator function and returns the left fold of the tuple.
    //ᅟ
    //ᅟ    auto numbers = std::make_tuple(2, 3u);
    //ᅟ    int sum = tuple_reduce_left(numbers, 0, std::plus<int>{ }); // returns 5
    //
template <typename TupleT, typename T, typename F>
    MAKESHIFT_NODISCARD constexpr auto
    tuple_reduce_left(TupleT&& tuple, T&& initialValue, F&& func)
{
    static_assert(is_tuple_like_v<std::decay_t<TupleT>>, "first argument must be tuple or tuple-like type");
    return makeshift::detail::fold_impl(makeshift::detail::left_fold{ },
        std::forward<TupleT>(tuple), std::forward<T>(initialValue), std::forward<F>(func));
}


    //ᅟ
    // Takes a tuple, an initial value, and a binary accumulator function and returns the right fold of the tuple.
    //ᅟ
    //ᅟ    auto numbers = std::make_tuple(2, 3u);
    //ᅟ    int sum = tuple_reduce_right(numbers, 0, std::plus<int>{ }); // returns 5
    //
template <typename TupleT, typename T, typename F>
    MAKESHIFT_NODISCARD constexpr auto
    tuple_reduce_right(TupleT&& tuple, T&& initialValue, F&& func)
{
    static_assert(is_tuple_like_v<std::decay_t<TupleT>>, "first argument must be tuple or tuple-like type");
    return makeshift::detail::fold_impl(makeshift::detail::right_fold{ },
        std::forward<TupleT>(tuple), std::forward<T>(initialValue), std::forward<F>(func));
}


    //ᅟ
    // Takes a tuple and a unary predicate function and evaluates the short-circuited conjunction of the predicate applied to all tuple elements.
    //ᅟ
    //ᅟ    auto numbers = std::tuple{ 2, 3u };
    //ᅟ    auto allNumbersGreaterThanZero = tuple_all_of(numbers,
    //ᅟ        [](auto v) { return v > 0; }));
    //ᅟ    // returns true
    //
template <typename TupleT, typename P>
    MAKESHIFT_NODISCARD constexpr auto
    tuple_all_of(TupleT&& tuple, P&& pred)
{
    static_assert(is_tuple_like_v<std::decay_t<TupleT>>, "first argument must be tuple or tuple-like type");
    return makeshift::detail::fold_impl(makeshift::detail::all_fold{ },
        std::forward<TupleT>(tuple), true, std::forward<P>(pred));
}


    //ᅟ
    // Takes a tuple and a unary predicate function and evaluates the short-circuited disjunction of the predicate applied to all tuple elements.
    //ᅟ
    //ᅟ    auto numbers = std::tuple{ 2, 3u };
    //ᅟ    auto anyNumberGreaterThanZero = tuple_any_of(numbers,
    //ᅟ        [](auto v) { return v > 0; }));
    //ᅟ    // returns true
    //
template <typename TupleT, typename P>
    MAKESHIFT_NODISCARD constexpr auto
    tuple_any_of(TupleT&& tuple, P&& pred)
{
    static_assert(is_tuple_like_v<std::decay_t<TupleT>>, "first argument must be tuple or tuple-like type");
    return makeshift::detail::fold_impl(makeshift::detail::any_fold{ },
        std::forward<TupleT>(tuple), false, std::forward<P>(pred));
}


    //ᅟ
    // Takes a tuple and a unary predicate function and evaluates the short-circuited negated disjunction of the predicate applied to all tuple elements.
    //ᅟ
    //ᅟ    auto numbers = std::tuple{ 2, 3u };
    //ᅟ    auto noNumberGreaterThanZero = tuple_none_of(numbers,
    //ᅟ        [](auto v) { return v > 0; }));
    //ᅟ    // returns false
    //
template <typename TupleT, typename P>
    MAKESHIFT_NODISCARD constexpr auto
    tuple_none_of(TupleT&& tuple, P&& pred)
{
    static_assert(is_tuple_like_v<std::decay_t<TupleT>>, "first argument must be tuple or tuple-like type");
    return !makeshift::detail::fold_impl(makeshift::detail::any_fold{ },
        std::forward<TupleT>(tuple), false, std::forward<P>(pred));
}


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_TUPLE2_HPP_
