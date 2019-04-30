
#ifndef INCLUDED_MAKESHIFT_DETAIL_TUPLE2_TRANSFORM_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_TUPLE2_TRANSFORM_HPP_


#include <cstddef>     // for size_t, ptrdiff_t
#include <utility>     // for forward<>(), integer_sequence<>, tuple_size<>, get<>()
#include <type_traits> // for decay<>, integral_constant<>, conjunction<>, disjunction<>

#include <makeshift/type_traits2.hpp> // for can_instantiate<>


namespace makeshift
{

namespace detail
{


template <typename T> using is_tuple_like_r = std::integral_constant<std::size_t, std::tuple_size<T>::value>;


} // namespace detail


inline namespace types
{


    //ᅟ
    // Pass `tuple_index` to `tuple_foreach()` or `tuple_transform()` to have the tuple element index passed as a functor argument.
    // The argument is of type `integral_constant<std::size_t, I>` and implicitly converts to `std::size_t`.
    //ᅟ
    //ᅟ    tuple_foreach(
    //ᅟ        [](auto element, std::size_t idx) { std::cout << idx << ": " << element << '\n'; },
    //ᅟ        std::make_tuple(42, 1.41421), tuple_index);
    //ᅟ    // prints "0: 42\n1: 1.41421"
    //
struct tuple_index_t { };


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


} // inline namespace types


namespace detail
{


template <typename T> struct is_tuple_arg : std::disjunction<std::is_same<std::decay_t<T>, tuple_index_t>, can_instantiate<is_tuple_like_r, std::decay_t<T>>> { };
template <typename T> constexpr bool is_tuple_arg_v = is_tuple_arg<T>::value;

template <typename... Ts> struct are_tuple_args : std::conjunction<is_tuple_arg<Ts>...> { };
template <typename... Ts> constexpr bool are_tuple_args_v = are_tuple_args<Ts...>::value;

template <typename T> struct maybe_tuple_size_ : std::tuple_size<T> { };
template <> struct maybe_tuple_size_<tuple_index_t> : std::integral_constant<std::ptrdiff_t, -1> { };

template <bool Mismatch, std::ptrdiff_t N, typename... Ts> struct equal_sizes_0_;
template <std::ptrdiff_t N, typename... Ts> struct equal_sizes_0_<true, N, Ts...> : std::false_type { static constexpr std::ptrdiff_t size = -1; };
template <std::ptrdiff_t N> struct equal_sizes_0_<false, N> : std::true_type { static constexpr std::ptrdiff_t size = N; };
template <std::ptrdiff_t N, typename T0, typename... Ts>
    struct equal_sizes_0_<false, N, T0, Ts...>
        : equal_sizes_0_<
              N != -1 && maybe_tuple_size_<T0>::value != -1 && N != maybe_tuple_size_<T0>::value,
              N != -1 ? N : maybe_tuple_size_<T0>::value,
              Ts...>
{
};
template <typename... Ts> struct equal_sizes_ : equal_sizes_0_<false, -1, Ts...> { };


namespace adl
{


template <std::size_t I>
    constexpr std::integral_constant<std::size_t, I> get(tuple_index_t) noexcept
{
    return { };
}


} // namespace adl


template <std::size_t I, typename T>
    constexpr decltype(auto) get_element(T&& arg) noexcept
{
    using std::get; // make std::get<>() visible to enable ADL for template methods named get<>()
    using makeshift::detail::adl::get;
    return get<I>(std::forward<T>(arg));
}


struct foreach_tag { };
struct transform_to_array_tag { };
template <typename T> struct transform_to_array_of_tag { };
struct transform_to_tuple_tag { };

template <std::size_t I, typename... Ts, typename F>
    constexpr decltype(auto)
    tuple_transform_impl2(F&& func, Ts&&... args)
{
    return func(makeshift::detail::get_element<I>(std::forward<Ts>(args))...);
}

template <std::size_t... Is, typename F, typename... Ts>
    constexpr void
    tuple_transform_impl1(foreach_tag, std::index_sequence<Is...>, F&& func, Ts&&... args)
{
    (void) func;
    using Swallow = int[];
    (void) Swallow{ 1,
        (makeshift::detail::tuple_transform_impl2<Is>(func, std::forward<Ts>(args)...), void(), int{ })...
    };
}
template <std::size_t... Is, typename F, typename... Ts>
    constexpr auto
    tuple_transform_impl1(transform_to_tuple_tag, std::index_sequence<Is...>, F&& func, Ts&&... args); // defined in makeshift/detail/tuple.hpp
template <std::size_t... Is, typename F, typename... Ts>
    constexpr auto
    tuple_transform_impl1(transform_to_array_tag, std::index_sequence<Is...>, F&& func, Ts&&... args); // defined in makeshift/detail/array.hpp
template <typename R, std::size_t... Is, typename F, typename... Ts>
    constexpr auto
    tuple_transform_impl1(transform_to_array_of_tag<R>, std::index_sequence<Is...>, F&& func, Ts&&... args); // defined in makeshift/detail/array.hpp

template <std::ptrdiff_t N, typename TransformTypeTag, typename F, typename... Ts>
    constexpr auto tuple_transform_impl0(F&& func, Ts&&... args)
{
    using Eq = equal_sizes_<std::decay_t<Ts>...>;
    static_assert(Eq::value, "sizes of tuple arguments do not match");
    static_assert(N != -1 || Eq::size != -1 || N == Eq::size, "given size argument does not match sizes of tuple arguments");
    static_assert(N != -1 || Eq::size != -1, "no tuple argument and no size given");
    constexpr std::size_t size = std::size_t(N != -1 ? N : Eq::size);
    
    (void) func;
    return makeshift::detail::tuple_transform_impl1(TransformTypeTag{ }, std::make_index_sequence<size>{ },
        std::forward<F>(func), std::forward<Ts>(args)...);
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_TUPLE2_TRANSFORM_HPP_
