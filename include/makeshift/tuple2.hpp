
#ifndef INCLUDED_MAKESHIFT_TUPLE2_HPP_
#define INCLUDED_MAKESHIFT_TUPLE2_HPP_


#include <array>
#include <tuple>       // for tuple<>, tuple_cat()
#include <cstddef>     // for size_t
#include <utility>     // for move(), forward<>(), tuple_element<>, tuple_size<>, get<>
#include <type_traits> // for decay<>, integral_constant<>, index_sequence<>, is_nothrow_default_constructible<>

#include <makeshift/type_traits.hpp> // for can_apply<>, none

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
    // Determines whether a type has a tuple-like interface (i.e. whether `std::tuple_size<T>::value` is well-formed).
    //
template <typename T> struct is_tuple_like2 : can_apply<makeshift::detail::is_tuple_like2_r, T> { };

    //ᅟ
    // Determines whether a type has a tuple-like interface (i.e. whether `std::tuple_size<T>::value` is well-formed).
    //
template <typename T> constexpr bool is_tuple_like2_v = is_tuple_like2<T>::value;


    //ᅟ
    // Type sequence (tuple without runtime value representation).
    //
template <typename... Ts>
    struct type_sequence2
{
    constexpr type_sequence2(void) noexcept = default;
    constexpr type_sequence2(const type_sequence2&) noexcept = default;
    constexpr type_sequence2& operator =(const type_sequence2&) noexcept { return *this; }
    constexpr type_sequence2(tag<Ts>...) noexcept { }
    constexpr void swap(type_sequence2&) noexcept { }
};
template <>
    struct type_sequence2<>
{
    constexpr type_sequence2(void) noexcept = default;
    constexpr type_sequence2(const type_sequence2&) noexcept = default;
    constexpr type_sequence2& operator =(const type_sequence2&) noexcept { return *this; }
    constexpr void swap(type_sequence2&) noexcept { }
};
template <typename... Ts>
    type_sequence2(tag<Ts>...) -> type_sequence2<Ts...>; // TODO: is this needed?


    //ᅟ
    // Return a type sequence that represents the types of the given values.
    //
template <typename... Ts>
    constexpr type_sequence2<Ts...> make_type_sequence2(tag<Ts>...) noexcept
{
    return { };
}


    //ᅟ
    // Returns the `I`-th element in the type sequence.
    //
template <std::size_t I, typename... Ts>
    constexpr tag<nth_type_t<I, Ts...>> get(const type_sequence2<Ts...>&) noexcept
{
    static_assert(I < sizeof...(Ts), "tuple index out of range");
    return { };
}

    //ᅟ
    // Returns the type sequence element of type `T`.
    //
template <typename T, typename... Ts>
    constexpr tag<T> get(const type_sequence2<Ts...>& ts) noexcept
{
	constexpr std::size_t index = try_index_of_type_v<T, Ts...>;
    static_assert(index != std::size_t(-1), "type T does not appear in type sequence");
    return { };
}


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

template <typename T> struct wrap_type_ { using type = tag<T>; };
template <typename T> struct wrap_type_<tag<T>> : wrap_type_<T> { };
template <typename T> struct unwrap_type_ { using type = T; };
template <typename T> struct unwrap_type_<tag<T>> : unwrap_type_<T> { };

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

} // namespace detail


inline namespace types
{


    //ᅟ
    // Takes a scalar procedure (i.e. a function of non-tuple arguments which returns nothing) and calls the procedure for every element in the given tuples.
    //ᅟ
    //ᅟ    tuple_foreach<3>(
    //ᅟ        [](std::size_t i) { std::cout << i << '\n'; },
    //ᅟ        tuple_index
    //ᅟ    ); // prints "0\n1\n2\n"
    //
template <std::size_t N, typename F, typename... Ts,
          typename = std::enable_if_t<makeshift::detail::are_tuple_args_v<Ts...>>>
    constexpr void
    tuple_foreach2(F&& func, Ts&&... args)
{
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
template <typename F, typename... Ts,
          typename = std::enable_if_t<makeshift::detail::are_tuple_args_v<Ts...>>>
    constexpr void
    tuple_foreach2(F&& func, Ts&&... args)
{
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
template <std::size_t N, typename F, typename... Ts,
          typename = std::enable_if_t<makeshift::detail::are_tuple_args_v<Ts...>>>
    constexpr auto
    tuple_transform2(F&& func, Ts&&... args)
{
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
template <typename F, typename... Ts,
          typename = std::enable_if_t<makeshift::detail::are_tuple_args_v<Ts...>>>
    constexpr auto
    tuple_transform2(F&& func, Ts&&... args)
{
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
template <std::size_t N, typename F, typename... Ts,
          typename = std::enable_if_t<makeshift::detail::are_tuple_args_v<Ts...>>>
    constexpr auto
    array_transform2(F&& func, Ts&&... args)
{
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
template <typename F, typename... Ts,
          typename = std::enable_if_t<makeshift::detail::are_tuple_args_v<Ts...>>>
    constexpr auto
    array_transform2(F&& func, Ts&&... args)
{
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
template <std::size_t N, typename F, typename... Ts,
          typename = std::enable_if_t<makeshift::detail::are_tuple_args_v<Ts...>>>
    constexpr auto
    type_sequence_transform2(F&& func, Ts&&... args)
{
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
template <typename F, typename... Ts,
          typename = std::enable_if_t<makeshift::detail::are_tuple_args_v<Ts...>>>
    constexpr auto
    type_sequence_transform2(F&& func, Ts&&... args)
{
    return makeshift::detail::tuple_transform_impl0<-1, makeshift::detail::transform_target::type_sequence>(std::forward<F>(func), std::forward<Ts>(args)...);
}


} // inline namespace types

} // namespace makeshift


namespace std
{


    // Specialize `tuple_size<>` and `tuple_element<>` for `type_sequence<>`.
template <typename... Ts> class tuple_size<makeshift::type_sequence2<Ts...>> : public std::integral_constant<std::size_t, sizeof...(Ts)> { };
template <std::size_t I, typename... Ts> class tuple_element<I, makeshift::type_sequence2<Ts...>> { using type = makeshift::tag<makeshift::nth_type_t<I, Ts...>>; };


} // namespace std


#endif // INCLUDED_MAKESHIFT_TUPLE2_HPP_
