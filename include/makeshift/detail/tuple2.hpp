﻿
#ifndef INCLUDED_MAKESHIFT_DETAIL_TUPLE2_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_TUPLE2_HPP_


#include <array>
#include <tuple>       // for tuple<>, tuple_cat()
#include <cstddef>     // for size_t, ptrdiff_t
#include <utility>     // for move(), forward<>(), integer_sequence<>, tuple_element<>, tuple_size<>, get<>()
#include <type_traits> // for decay<>, integral_constant<>, conjunction<>, disjunction<>

#include <makeshift/type_traits.hpp>  // for can_apply<>
#include <makeshift/type_traits2.hpp> // for type<>, type_sequence2<>

#include <makeshift/detail/workaround.hpp> // for cand()


namespace makeshift
{

namespace detail
{


template <typename T> using is_tuple_like2_r = std::integral_constant<std::size_t, std::tuple_size<T>::value>;


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


} // inline namespace types


namespace detail
{


template <typename T> struct is_tuple_arg : std::disjunction<std::is_same<std::decay_t<T>, tuple_index_t>, can_apply<is_tuple_like2_r, std::decay_t<T>>> { };
template <typename T> constexpr bool is_tuple_arg_v = is_tuple_arg<T>::value;

template <typename... Ts> struct are_tuple_args : std::conjunction<is_tuple_arg<Ts>...> { };
template <typename... Ts> constexpr bool are_tuple_args_v = are_tuple_args<Ts...>::value;

template <typename T> struct is_std_array_ : std::false_type { };
template <typename T, std::size_t N> struct is_std_array_<std::array<T, N>> : std::true_type { };

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

template <typename... Ts> struct equal_types_;
template <> struct equal_types_<> : std::true_type { };
template <typename T0> struct equal_types_<T0> : std::true_type { using value_type = T0; };
template <typename T0, typename T1, typename... Ts> struct equal_types_<T0, T1, Ts...> : std::conditional_t<std::is_same<T0, T1>::value, equal_types_<T1, Ts...>, std::false_type> { };

template <typename F, std::size_t I, typename... Ts> struct result_type_ { using type = decltype(std::declval<F>()(get_element<I>(std::declval<Ts>())...)); };

template <typename F, typename Is, typename... Ts>
    struct result_types_;
template <typename F, std::size_t... Is, typename... Ts>
    struct result_types_<F, std::index_sequence<Is...>, Ts...>
{
    using type = type_sequence2<typename result_type_<F, Is, Ts...>::type...>;
};

template <typename R, typename T> struct transfer_ref_ { using type = T; };
template <typename R, typename T> struct transfer_ref_<R&, T> { using type = T&; };
template <typename R, typename T> struct transfer_ref_<const R&, T> { using type = const T&; };
template <typename R, typename T> struct transfer_ref_<R&&, T> { using type = T; };

template <typename T> struct wrap_type_ { using type = makeshift::type<T>; };
template <typename T> struct wrap_type_<type<T>> : wrap_type_<T> { };
template <typename T> struct unwrap_type_ { using type = T; };
template <typename T> struct unwrap_type_<type<T>> : unwrap_type_<T> { };

template <std::ptrdiff_t N, bool HomogeneousArgs, typename F, typename... Ts>
    struct homogeneous_result_;
template <std::ptrdiff_t N, typename F, typename... Ts>
    struct homogeneous_result_<N, true, F, Ts...>
{
        // all arguments are array types, so we can just extract their value types
    using type = decltype(std::declval<F>()(std::declval<typename transfer_ref_<Ts, typename std::decay_t<Ts>::value_type>::type>()...));
};
template <typename F, typename Rs, typename... Ts>
    struct check_homogeneous_result_;
template <typename F, typename... Rs, typename... Ts>
    struct check_homogeneous_result_<F, type_sequence2<Rs...>, Ts...>
{
    using Eq = equal_types_<Rs...>;
    static_assert(Eq::value, "result types of functor must be identical for all sets of tuple elements");
    using type = typename Eq::value_type; // exists only if types are identical
};
template <std::ptrdiff_t N, typename F, typename... Ts>
    struct homogeneous_result_<N, false, F, Ts...>
        : check_homogeneous_result_<F, typename result_types_<F, std::make_index_sequence<N>, Ts...>::type, Ts...>
{
        // some arguments are non-homogeneous, so we need to compute return types for all combinations and verify that they match exactly
};

enum class transform_target { nothing, type_sequence, array, tuple };

template <std::size_t I, typename... Ts, typename F>
    constexpr decltype(auto)
    tuple_transform_impl2(F&& func, Ts&&... args)
{
    return func(get_element<I>(std::forward<Ts>(args))...);
}

template <std::size_t... Is, typename F, typename... Ts>
    constexpr void
    tuple_transform_impl1(std::integral_constant<transform_target, transform_target::nothing>, std::index_sequence<Is...>, F&& func, Ts&&... args)
{
    using Swallow = int[];
    (void) Swallow{ 1,
        (tuple_transform_impl2<Is>(std::forward<F>(func), std::forward<Ts>(args)...), void(), int{ })...
    };
}
template <std::size_t... Is, typename F, typename... Ts>
    constexpr auto
    tuple_transform_impl1(std::integral_constant<transform_target, transform_target::type_sequence>, std::index_sequence<Is...>, F&& func, Ts&&... args)
{
    return type_sequence2<typename unwrap_type_<decltype(tuple_transform_impl2<Is>(std::forward<F>(func), std::forward<Ts>(args)...))>::type...>{ };
}
template <std::size_t... Is, typename F, typename... Ts>
    constexpr auto
    tuple_transform_impl1(std::integral_constant<transform_target, transform_target::tuple>, std::index_sequence<Is...>, F&& func, Ts&&... args)
{
    return std::make_tuple(tuple_transform_impl2<Is>(std::forward<F>(func), std::forward<Ts>(args)...)...);
}
template <std::size_t... Is, typename F, typename... Ts>
    constexpr auto
    tuple_transform_impl1(std::integral_constant<transform_target, transform_target::array>, std::index_sequence<Is...>, F&& func, Ts&&... args)
{
    using R = typename homogeneous_result_<sizeof...(Is), cand(is_std_array_<std::decay_t<Ts>>::value...), F, Ts...>::type;
    return std::array<R, sizeof...(Is)>{ tuple_transform_impl2<Is>(std::forward<F>(func), std::forward<Ts>(args)...)... };
}

template <std::ptrdiff_t N, transform_target TransformTarget, typename F, typename... Ts>
    constexpr auto tuple_transform_impl0(F&& func, Ts&&... args)
{
    using Eq = equal_sizes_<std::decay_t<Ts>...>;
    static_assert(Eq::value, "sizes of tuple arguments do not match");
    static_assert(N != -1 || Eq::size != -1 || N == Eq::size, "given size argument does not match sizes of tuple arguments");
    static_assert(N != -1 || Eq::size != -1, "no tuple argument and no size given");
    static constexpr std::size_t size = std::size_t(N != -1 ? N : Eq::size);
    
    return tuple_transform_impl1(std::integral_constant<transform_target, TransformTarget>{ }, std::make_index_sequence<size>{ },
        std::forward<F>(func), std::forward<Ts>(args)...);
}

struct left_fold { };
struct right_fold { };
struct all_fold { };
struct any_fold { };

template <typename FoldT, typename TupleT, typename T, typename F>
    constexpr auto fold_impl(std::index_sequence<>, FoldT, TupleT&&, T&& initialValue, F&&)
{
    return std::forward<T>(initialValue);
}
template <std::size_t I0, std::size_t... Is, typename TupleT, typename T, typename F>
    constexpr auto
    fold_impl(std::index_sequence<I0, Is...>, left_fold, TupleT&& tuple, T&& initialValue, F&& func)
{
    return fold_impl(std::index_sequence<Is...>{ }, left_fold{ }, std::forward<TupleT>(tuple),
        func(std::forward<T>(initialValue), std::get<I0>(std::forward<TupleT>(tuple))),
        std::forward<F>(func));
}
template <std::size_t I0, std::size_t... Is, typename TupleT, typename T, typename F>
    constexpr auto
    fold_impl(std::index_sequence<I0, Is...>, right_fold, TupleT&& tuple, T&& initialValue, F&& func)
{
    return fold_impl(std::index_sequence<Is...>{ }, right_fold{ }, std::forward<TupleT>(tuple),
        func(std::get<std::tuple_size<std::decay_t<TupleT>>::value - 1 - I0>(std::forward<TupleT>(tuple)), std::forward<T>(initialValue)),
        std::forward<F>(func));
}
template <std::size_t I0, std::size_t... Is, typename TupleT, typename F>
    constexpr auto
    fold_impl(std::index_sequence<I0, Is...>, all_fold, TupleT&& tuple, bool, F&& func)
{
    return func(std::get<I0>(std::forward<TupleT>(tuple)))
        && fold_impl(std::index_sequence<Is...>{ }, all_fold{ }, std::forward<TupleT>(tuple), true, std::forward<F>(func));
}
template <std::size_t I0, std::size_t... Is, typename TupleT, typename F>
    constexpr auto
    fold_impl(std::index_sequence<I0, Is...>, any_fold, TupleT&& tuple, bool, F&& func)
{
    return func(std::get<I0>(std::forward<TupleT>(tuple)))
        || fold_impl(std::index_sequence<Is...>{ }, any_fold{ }, std::forward<TupleT>(tuple), false, std::forward<F>(func));
}
template <typename FoldT, typename TupleT, typename T, typename F>
    constexpr auto
    fold_impl(FoldT, TupleT&& tuple, T&& initialValue, F&& func)
{
    return fold_impl(std::make_index_sequence<std::tuple_size<std::decay_t<TupleT>>::value>{ }, FoldT{ },
        std::forward<TupleT>(tuple), std::forward<T>(initialValue), std::forward<F>(func));
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_TUPLE2_HPP_