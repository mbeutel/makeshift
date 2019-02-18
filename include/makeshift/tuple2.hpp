
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


template <typename T> using is_tuple_like_r = std::integral_constant<std::size_t, std::tuple_size<T>::value>;


} // namespace detail


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
	constexpr index = try_index_of_type_v<T, Ts...>;
    static_assert(index != std::size_t(-1), "type T does not appear in type sequence");
    return { };
}


    //ᅟ
    // Pass `tuple_index` to `tuple_foreach()` or `tuple_map()` to have the tuple element index passed as a functor argument.
    // The argument is of type `integral_constant<std::size_t, I>` and implicitly converts to `std::size_t`.
    //ᅟ
    //ᅟ    auto print_tuple = tuple_foreach([](auto element, std::size_t idx) { std::cout << idx << ": " << element << '\n'; });
    //ᅟ    auto tuple = std::tuple{ 42, 1.41421 };
    //ᅟ    print_tuple(tuple, tuple_index); // prints "0: 42\n1: 1.41421"
    //
struct tuple_index_t { };

    //ᅟ
    // Pass `tuple_index` to `tuple_foreach()` or `tuple_map()` to have the tuple element index passed as a functor argument.
    // The argument is of type `integral_constant<std::size_t, I>` and implicitly converts to `std::size_t`.
    //ᅟ
    //ᅟ    auto print_tuple = tuple_foreach([](auto element, std::size_t idx) { std::cout << idx << ": " << element << '\n'; });
    //ᅟ    auto tuple = std::tuple{ 42, 1.41421 };
    //ᅟ    print_tuple(tuple, tuple_index); // prints "0: 42\n1: 1.41421"
    //
constexpr tuple_index_t tuple_index{ };


} // inline namespace types


namespace detail
{


template <typename T> struct is_tuple_arg : disjunction<std::is_same<std::decay_t<T>, tuple_index_t>, can_apply<is_tuple_like_r, std::decay_t<T>>> { };
template <typename T> constexpr bool is_tuple_arg_v = is_tuple_arg<T>::value;

constexpr unsigned tfSizeMismatch = 0b00000001;
constexpr unsigned tfArray        = 0b00000011;
constexpr unsigned tfTypeSeq      = 0b00000101;
constexpr unsigned tfEmpty        = 0b00001110;

template <typename T> struct tuple_flag_ : std::integral_constant<unsigned, 0> { };
template <typename T, std::size_t N> struct tuple_flag_<std::array<T, N>> : std::integral_constant<unsigned, tfArray> { };
template <typename... Ts> struct tuple_flag_<type_sequence2<Ts...>> : std::integral_constant<unsigned, tfTypeSeq> { };

template <typename T> struct arg_val_;
template <typename T> struct arg_val_<tuple_index_t> { using type = tuple_index_t; }; // TODO: are these really needed?
template <typename T> struct arg_val_<tuple_index_t&&> { using type = tuple_index_t; };
template <typename T> struct arg_val_<tuple_index_t&> { using type = tuple_index_t; };
template <typename T> struct arg_val_<const tuple_index_t&> { using type = tuple_index_t; };
template <typename T, std::size_t N> struct arg_val_<std::array<T, N>> { using type = T; };
template <typename T, std::size_t N> struct arg_val_<std::array<T, N>&&> { using type = T; };
template <typename T, std::size_t N> struct arg_val_<std::array<T, N>&> { using type = T&; };
template <typename T, std::size_t N> struct arg_val_<const std::array<T, N>&> { using type = const T&; };

template <unsigned Flags, std::size_t N, typename... Ts> struct tuple_params_0_;
template <unsigned Flags, std::size_t N>
    struct tuple_params_0_<Flags, N>
{
    static constexpr bool no_args = Flags == tfEmpty;
    static constexpr bool homogeneous = (Flags | tfArray) != 0;
    std::size_t size = (Flags | tfSizeMismatch) != 0 ? std::size_t(-1) : N;
};
template <unsigned Flags, std::size_t N, tuple_index_t, typename... Ts> struct tuple_params_0_ : tuple_params_0_<Flags, N, Ts...> { };
template <unsigned Flags, std::size_t N, typename T0, typename... Ts> struct tuple_params_0_
    : tuple_params_0_<(Flags & tuple_flag_<T0>::value) | ((N != std::size_t(-1) && N != std::tuple_size<T0>::value) ? tfSizeMismatch : 0), std::tuple_size<T0>::value, Ts...>
{
};
template <std::size_t N, typename... Ts> struct tuple_params_ : tuple_params_0_<tfEmpty, N, Ts...> { };


template <std::size_t I, typename T>
    constexpr std::integral_constant<std::size_t, I> get_element(tuple_index_t) noexcept
{
    return { };
}
template <std::size_t I, typename T> // TODO: need SFINAE here to exclude tuple_index_t<>?
    constexpr decltype(auto) get_element(T&& arg) noexcept
{
    using std::get; // make std::get<>() visible to enable ADL for template methods named get<>()
    return get<I>(std::forward<T>(arg));
}

template <std::size_t I, typename Ts, typename F>
    constexpr decltype(auto) tuple_transform_impl2(F&& func, Ts&&... tuples)
{
    return func(get_element<I>(std::forward<Ts>(tuples))...);
}

// implementations: void, tag, (void & homogeneous,) tag & homogeneous, any, any & homogeneous

unsigned rtkVoid        = 0;
unsigned rtkTag         = 1;
unsigned rtkAny         = 2;
unsigned rtkHomogeneous = 3;

template <typename Ts, typename F, std::size_t... Is>
    constexpr void tuple_transform_impl1(std::integral_constant<unsigned, trfVoid>, std::index_sequence<Is...>, F&& func, Ts&&... args)
{
    int _[] = { (tuple_transform_impl2<Is>(std::forward<F>(func), std::forward<Ts>(args)...), void, 0)... };
    (void) _;
}
template <typename Ts, typename F, std::size_t... Is>
    constexpr auto tuple_transform_impl1(std::integral_constant<unsigned, trfTag>, std::index_sequence<Is...>, F&& func, Ts&&... args)
{
    return make_type_sequence2(tuple_transform_impl2<Is>(std::forward<F>(func), std::forward<Ts>(args)...)...);
}
template <typename Ts, typename F, std::size_t... Is>
    constexpr auto tuple_transform_impl1(std::integral_constant<unsigned, trfAny>, std::index_sequence<Is...>, F&& func, Ts&&... args)
{
    return std::make_tuple(tuple_transform_impl2<Is>(std::forward<F>(func), std::forward<Ts>(args)...)...);
}
template <typename Ts, typename F, std::size_t... Is>
    constexpr auto tuple_transform_impl1(std::integral_constant<unsigned, trfHomogeneous>, std::index_sequence<Is...>, F&& func, Ts&&... args)
        ->std::array<std::decay_t<decltype(func(std::declval<typename arg_val_<Ts>::type>()...))>, sizeof...(Is)>
{
    return { tuple_transform_impl2<Is>(std::forward<F>(func), std::forward<Ts>(args)...)... };
}

template <std::size_t N, unsigned ReturnTypeKind, typename... Ts>
    constexpr auto tuple_transform_impl0(F&& func, Ts&&... args)
{
    using Params = tuple_params_<N, std::decay_t<Ts>...>;
    static_assert(N != std::size_t(-1) || !Params::no_args, "no tuple argument and no size given");
    static_assert(Params::size != std::size_t(-1), "sizes of tuple arguments do not match");
    
    return tuple_transform_impl1(std::integral_constant<unsigned, ReturnTypeKind>{ }, std::make_index_sequence<Params::size>{ },
        std::forward<F>(func), std::forward<Ts>(args)...);
}

} // namespace detail


inline namespace types
{


    //ᅟ
    // Takes a scalar procedure (i.e. a function with non-tuple arguments and with void return type) and calls the procedure for every
    // element in the given tuples.
    //ᅟ
    //ᅟ    tuple_foreach<3>(
    //ᅟ        [](std::size_t i) { std::cout << i << '\n'; },
    //ᅟ        tuple_index
    //ᅟ    ); // prints "1\n2.3\n"
    //
template <std::size_t N, typename F, typename Ts,
          typename = std::enable_if_t<std::conjunction<makeshift::detail::is_tuple_arg<Ts>...>>>
    constexpr void
    tuple_foreach2(F&& func, Ts&&... args)
{
    makeshift::detail::tuple_transform_impl0<N, makeshift::detail::rtkVoid>(std::forward<F>(func), std::forward<Ts>(args)...);
}


    //ᅟ
    // Takes a scalar procedure (i.e. a function with non-tuple arguments and with void return type) and returns a tuple of the results
    // of the function applied to the tuple elements.
    //ᅟ
    //ᅟ    auto indices = tuple_transform<3>(
    //ᅟ        [](std::size_t i) { return i; },
    //ᅟ        tuple_index
    //ᅟ    ); // returns (0, 1, 2)
    //
template <std::size_t N, typename F, typename Ts,
          typename = std::enable_if_t<std::conjunction<makeshift::detail::is_tuple_arg<Ts>...>>>
    constexpr void
    tuple_transform2(F&& func, Ts&&... args)
{
    makeshift::detail::tuple_transform_impl0<N, makeshift::detail::rtkAny>(std::forward<F>(func), std::forward<Ts>(args)...);
}


    //ᅟ
    // Takes a scalar procedure (i.e. a function with non-tuple arguments and with void return type) and returns a tuple of the results
    // of the function applied to the tuple elements.
    //ᅟ
    //ᅟ    auto indices = tuple_transform<3>(
    //ᅟ        [](std::size_t i) { return i; },
    //ᅟ        tuple_index
    //ᅟ    ); // returns (0, 1, 2)
    //
template <typename F, typename Ts,
          typename = std::enable_if_t<std::conjunction<makeshift::detail::is_tuple_arg<Ts>...>>>
    constexpr void
    tuple_transform2(F&& func, Ts&&... args)
{
    makeshift::detail::tuple_transform_impl0<std::size_t(-1), makeshift::detail::rtkAny>(std::forward<F>(func), std::forward<Ts>(args)...);
}


    //ᅟ
    // Takes a scalar procedure (i.e. a function with non-tuple arguments and with void return type) and calls the procedure for every
    // element in the given tuples.
    //ᅟ
    //ᅟ    tuple_foreach(
    //ᅟ        [](auto name, auto elem) { std::cout << name << ": " << elem << '\n'; },
    //ᅟ        std::tuple{ "a", "b" },
    //ᅟ        std::tuple{ 1, 2.3f }
    //ᅟ    ); // prints "1\n2.3\n"
    //
template <typename F, typename Ts,
          typename = std::enable_if_t<std::conjunction<makeshift::detail::is_tuple_arg<Ts>...>>>
    constexpr void
    tuple_foreach2(F&& func, Ts&&... args)
{
    makeshift::detail::tuple_transform_impl0<std::size_t(-1), makeshift::detail::rtkVoid>(std::forward<F>(func), std::forward<Ts>(args)...);
}


    //ᅟ
    // Takes a scalar procedure (i.e. a function with non-tuple arguments and with void return type) and returns an array of the results
    // of the function applied to the tuple elements.
    //ᅟ
    //ᅟ    auto indices = tuple_transform<3>(
    //ᅟ        [](std::size_t i) { return i; },
    //ᅟ        tuple_index
    //ᅟ    ); // returns (0, 1, 2)
    //
template <std::size_t N, typename F, typename Ts,
          typename = std::enable_if_t<std::conjunction<makeshift::detail::is_tuple_arg<Ts>...>>>
    constexpr void
    array_transform2(F&& func, Ts&&... args)
{
    makeshift::detail::tuple_transform_impl0<N, makeshift::detail::rtkHomogeneous>(std::forward<F>(func), std::forward<Ts>(args)...);
}


    //ᅟ
    // Takes a scalar procedure (i.e. a function with non-tuple arguments and with void return type) and returns an array of the results
    // of the function applied to the tuple elements.
    //ᅟ
    //ᅟ    auto indices = tuple_transform<3>(
    //ᅟ        [](std::size_t i) { return i; },
    //ᅟ        tuple_index
    //ᅟ    ); // returns (0, 1, 2)
    //
template <typename F, typename Ts,
          typename = std::enable_if_t<std::conjunction<makeshift::detail::is_tuple_arg<Ts>...>>>
    constexpr void
    array_transform2(F&& func, Ts&&... args)
{
    makeshift::detail::tuple_transform_impl0<std::size_t(-1), makeshift::detail::rtkHomogeneous>(std::forward<F>(func), std::forward<Ts>(args)...);
}


    //ᅟ
    // Takes a scalar procedure (i.e. a function with non-tuple arguments and with void return type) and returns a tuple of the results
    // of the function applied to the tuple elements.
    //ᅟ
    //ᅟ    auto indices = tuple_transform<3>(
    //ᅟ        [](std::size_t i) { return i; },
    //ᅟ        tuple_index
    //ᅟ    ); // returns (0, 1, 2)
    //
template <std::size_t N, typename F, typename Ts,
          typename = std::enable_if_t<std::conjunction<makeshift::detail::is_tuple_arg<Ts>...>>>
    constexpr void
    type_sequence_transform2(F&& func, Ts&&... args)
{
    makeshift::detail::tuple_transform_impl0<N, makeshift::detail::rtkTag>(std::forward<F>(func), std::forward<Ts>(args)...);
}


    //ᅟ
    // Takes a scalar procedure (i.e. a function with non-tuple arguments and with void return type) and returns a tuple of the results
    // of the function applied to the tuple elements.
    //ᅟ
    //ᅟ    auto indices = tuple_transform<3>(
    //ᅟ        [](std::size_t i) { return i; },
    //ᅟ        tuple_index
    //ᅟ    ); // returns (0, 1, 2)
    //
template <typename F, typename Ts,
          typename = std::enable_if_t<std::conjunction<makeshift::detail::is_tuple_arg<Ts>...>>>
    constexpr void
    type_sequence_transform2(F&& func, Ts&&... args)
{
    makeshift::detail::tuple_transform_impl0<std::size_t(-1), makeshift::detail::rtkTag>(std::forward<F>(func), std::forward<Ts>(args)...);
}


    //ᅟ
    // Takes a scalar function (i.e. a function with non-tuple arguments and non-tuple return type) and returns a function which can be called 
    // with tuples in some or all arguments, and whose result will be a tuple of the results of the function applied to the tuple elements.
    //ᅟ
    //ᅟ    auto numbers = std::tuple{ 2, 3.0f };
    //ᅟ    auto squaredNumbers = numbers
    //ᅟ        | tuple_map([](auto x) { return x*x; }); // returns (4, 9.0f)
    //
template <typename F>
    constexpr makeshift::detail::tuple_foreach_t<true, std::decay_t<F>>
    tuple_map(F&& func)
{
    return { std::forward<F>(func) };
}


    //ᅟ
    // Takes a tuple and a scalar function (i.e. a function with non-tuple arguments and non-tuple return type) and returns a tuple of the results
    // of the function applied to the tuple elements.
    //ᅟ
    //ᅟ    auto numbers = std::tuple{ 2, 3.0f };
    //ᅟ    auto squaredNumbers = tuple_map(numbers, [](auto x) { return x*x; }); // returns (4, 9.0f)
    //
template <typename TupleT, typename F,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr auto
    tuple_map(TupleT&& tuple, F&& func)
{
    return tuple_map(std::forward<F>(func))(std::forward<TupleT>(tuple));
}


    //ᅟ
    // Returns a functor that maps a tuple to an array of element type `T` that is initialized with result of the functor applied to the elements
    // in the tuple.
    //ᅟ
    //ᅟ    auto tuple = std::tuple{ 1, 2u, 3.0 };
    //ᅟ    auto array = tuple
    //ᅟ        | tuple_map_to<int>([](auto v) { return int(v*v); }); // returns {{ 1, 4, 9 }}
    //
template <typename T, typename F>
    constexpr makeshift::detail::tuple_map_to_t<std::remove_cv_t<T>, std::decay_t<F>>
    tuple_map_to(F&& func)
{
    return { std::forward<F>(func) };
}


    //ᅟ
    // Returns a `std::array<>` of element type `T` that is initialized with the result of the functor applied to the elements in the tuple.
    //ᅟ
    //ᅟ    auto tuple = std::tuple{ 1, 2u, 3.0 };
    //ᅟ    auto array = tuple_map_to<int>(tuple, [](auto v) { return int(v*v); }); // returns {{ 1, 2, 3 }}
    //
template <typename T, typename TupleT, typename F,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr std::array<std::remove_cv_t<T>, std::tuple_size<std::decay_t<TupleT>>::value>
    tuple_map_to(TupleT&& tuple, F&& func)
{
    return tuple_map_to<T>()(std::forward<TupleT>(tuple), std::forward<F>(func));
}


    //ᅟ
    // Returns a functor that maps a tuple to an array of element type `T` that is initialized with the elements in the tuple.
    //ᅟ
    //ᅟ    auto tuple = std::tuple{ 1, 2, 3 };
    //ᅟ    auto array = tuple
    //ᅟ        | tuple_to_array<int>(); // returns {{ 1, 2, 3 }}
    //
template <typename T>
    constexpr makeshift::detail::tuple_map_to_t<std::remove_cv_t<T>, makeshift::detail::implicit_conversion_transform<T>>
    tuple_to_array(void)
{
    return { { } };
}


    //ᅟ
    // Returns a `std::array<>` of element type `T` that is initialized with the elements in the tuple.
    //ᅟ
    //ᅟ    auto tuple = std::tuple{ 1, 2, 3 };
    //ᅟ    auto array = tuple_to_array<int>(tuple); // returns {{ 1, 2, 3 }}
    //
template <typename T, typename TupleT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr std::array<std::remove_cv_t<T>, std::tuple_size<std::decay_t<TupleT>>::value>
    tuple_to_array(TupleT&& tuple)
{
    return tuple_to_array<T>()(std::forward<TupleT>(tuple));
}


    //ᅟ
    // Takes a binary accumulator function (i.e. a function with non-tuple arguments and non-tuple return type) and returns a function which reduces
    // an initial value and a tuple to a scalar using the accumulator function by performing a left fold.
    //ᅟ
    //ᅟ    auto sumTuple = tuple_fold(std::plus<int>{ });
    //ᅟ    auto numbers = std::tuple{ 2, 3u };
    //ᅟ    int sum = sumTuple(0, numbers); // returns 5
    //
template <typename F>
    constexpr makeshift::detail::tuple_fold_t<true, std::decay_t<F>>
    tuple_fold(F&& func)
{
    return { std::forward<F>(func) };
}


    //ᅟ
    // Takes a binary accumulator function (i.e. a function with non-tuple arguments and non-tuple return type) and returns a function which reduces
    // an initial value and a tuple to a scalar using the accumulator function by performing a left fold.
    //ᅟ
    //ᅟ    auto sumTuple = tuple_fold_left(std::plus<int>{ });
    //ᅟ    auto numbers = std::tuple{ 2, 3u };
    //ᅟ    int sum = sumTuple(0, numbers); // returns 5
    //
template <typename F>
    constexpr makeshift::detail::tuple_fold_t<true, std::decay_t<F>>
    tuple_fold_left(F&& func)
{
    return { std::forward<F>(func) };
}


    //ᅟ
    // Takes a binary accumulator function (i.e. a function with non-tuple arguments and non-tuple return type) and returns a function which reduces
    // an initial value and a tuple to a scalar using the accumulator function by performing a right fold.
    //ᅟ
    //ᅟ    auto sumTuple = tuple_fold_right(std::plus<int>{ });
    //ᅟ    auto numbers = std::tuple{ 2, 3u };
    //ᅟ    int sum = sumTuple(0, numbers); // returns 5
    //
template <typename F>
    constexpr makeshift::detail::tuple_fold_t<false, std::decay_t<F>>
    tuple_fold_right(F&& func)
{
    return { std::forward<F>(func) };
}


    //ᅟ
    // Takes an initial value and a binary accumulator function (i.e. a function with non-tuple arguments and non-tuple type) and returns a function
    // which reduces a tuple to a scalar using the accumulator function by performing a left fold.
    //ᅟ
    //ᅟ    auto numbers = std::tuple{ 2, 3u };
    //ᅟ    int sum = numbers
    //ᅟ        | tuple_fold(0, std::plus<int>{ }); // returns 5
    //
template <typename ValT, typename F>
    constexpr makeshift::detail::tuple_bound_fold_t<true, std::decay_t<ValT>, std::decay_t<F>>
    tuple_fold(ValT&& initialValue, F&& func)
{
    return { std::forward<ValT>(initialValue), std::forward<F>(func) };
}


    //ᅟ
    // Takes an initial value and a binary accumulator function (i.e. a function with non-tuple arguments and non-tuple type) and returns a function
    // which reduces a tuple to a scalar using the accumulator function by performing a left fold.
    //ᅟ
    //ᅟ    auto numbers = std::tuple{ 2, 3u };
    //ᅟ    int sum = numbers
    //ᅟ        | tuple_fold_left(0, std::plus<int>{ }); // returns 5
    //
template <typename ValT, typename F>
    constexpr makeshift::detail::tuple_bound_fold_t<true, std::decay_t<ValT>, std::decay_t<F>>
    tuple_fold_left(ValT&& initialValue, F&& func)
{
    return { std::forward<ValT>(initialValue), std::forward<F>(func) };
}


    //ᅟ
    // Takes an initial value and a binary accumulator function (i.e. a function with non-tuple arguments and non-tuple type) and returns a function
    // which reduces a tuple to a scalar using the accumulator function by performing a right fold.
    //ᅟ
    //ᅟ    auto numbers = std::tuple{ 2, 3u };
    //ᅟ    int sum = numbers
    //ᅟ        | tuple_fold_right(0, std::plus<int>{ }); // returns 5
    //
template <typename ValT, typename F>
    constexpr makeshift::detail::tuple_bound_fold_t<false, std::decay_t<ValT>, std::decay_t<F>>
    tuple_fold_right(ValT&& initialValue, F&& func)
{
    return { std::forward<ValT>(initialValue), std::forward<F>(func) };
}


    //ᅟ
    // Takes a tuple, an initial value and a binary accumulator function (i.e. a function with non-tuple arguments and non-tuple type) and returns the
    // left fold of the tuple.
    //ᅟ
    //ᅟ    auto numbers = std::tuple{ 2, 3u };
    //ᅟ    int sum = tuple_fold(numbers, 0, std::plus<int>{ }); // returns 5
    //
template <typename TupleT, typename T, typename F,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr auto
    tuple_fold(TupleT&& tuple, T&& initialValue, F&& func)
{
    return tuple_fold_left(std::forward<F>(func))(std::forward<T>(initialValue), std::forward<TupleT>(tuple));
}


    //ᅟ
    // Takes a tuple, an initial value and a binary accumulator function (i.e. a function with non-tuple arguments and non-tuple type) and returns the
    // left fold of the tuple.
    //ᅟ
    //ᅟ    auto numbers = std::tuple{ 2, 3u };
    //ᅟ    int sum = tuple_fold_left(numbers, 0, std::plus<int>{ }); // returns 5
    //
template <typename TupleT, typename T, typename F,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr auto
    tuple_fold_left(TupleT&& tuple, T&& initialValue, F&& func)
{
    return tuple_fold_left(std::forward<F>(func))(std::forward<T>(initialValue), std::forward<TupleT>(tuple));
}


    //ᅟ
    // Takes a tuple, an initial value and a binary accumulator function (i.e. a function with non-tuple arguments and non-tuple type) and returns the
    // right fold of the tuple.
    //ᅟ
    //ᅟ    auto numbers = std::tuple{ 2, 3u };
    //ᅟ    int sum = tuple_fold_right(numbers, 0, std::plus<int>{ }); // returns 5
    //
template <typename TupleT, typename T, typename F,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr auto
    tuple_fold_right(TupleT&& tuple, T&& initialValue, F&& func)
{
    return tuple_fold_right(std::forward<F>(func))(std::forward<T>(initialValue), std::forward<TupleT>(tuple));
}


    //ᅟ
    // Takes a unary predicate function and returns a function which evaluates the short-circuited conjunction of the predicate applied to all tuple elements.
    //ᅟ
    //ᅟ    auto numbers = std::tuple{ 2, 3u };
    //ᅟ    auto allNumbersGreaterThanZero = numbers
    //ᅟ        | tuple_all([](auto v) { return v > 0; }); // returns true
    //
template <typename F>
    constexpr makeshift::detail::tuple_all_any_t<true, std::decay_t<F>>
    tuple_all(F&& func)
{
    return { std::forward<F>(func) };
}

    //ᅟ
    // Takes a unary predicate function and evaluates the short-circuited conjunction of the predicate applied to all tuple elements.
    //ᅟ
    //ᅟ    auto numbers = std::tuple{ 2, 3u };
    //ᅟ    auto allNumbersGreaterThanZero = tuple_all(numbers,
    //ᅟ        [](auto v) { return v > 0; })); // returns true
    //
template <typename TupleT, typename F,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr bool
    tuple_all(TupleT&& tuple, F&& func)
{
    return tuple_all(std::forward<F>(func))(std::forward<TupleT>(tuple));
}


    //ᅟ
    // Takes a unary predicate function and returns a function which evaluates the short-circuited disjunction of the predicate applied to all tuple elements.
    //ᅟ
    //ᅟ    auto numbers = std::tuple{ 2, 3u };
    //ᅟ    auto anyNumberGreaterThanZero = numbers
    //ᅟ        | tuple_any([](auto v) { return v > 0; }); // returns true
    //
template <typename F>
    constexpr makeshift::detail::tuple_all_any_t<false, std::decay_t<F>>
    tuple_any(F&& func)
{
    return { std::forward<F>(func) };
}

    //ᅟ
    // Takes a unary predicate function and evaluates the short-circuited disjunction of the predicate applied to all tuple elements.
    //ᅟ
    //ᅟ    auto numbers = std::tuple{ 2, 3u };
    //ᅟ    auto anyNumberGreaterThanZero = tuple_any(numbers,
    //ᅟ        [](auto v) { return v > 0; })); // returns true
    //
template <typename TupleT, typename F,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr bool
    tuple_any(TupleT&& tuple, F&& func)
{
    return tuple_any(std::forward<F>(func))(std::forward<TupleT>(tuple));
}


    //ᅟ
    // Returns a functor which retrieves the tuple element of the given type, or which returns the provided default value if the tuple does not contain
    // an element of the given type. The type of the default value does not need to match the desired element type.
    //ᅟ
    //ᅟ    auto tuple = std::tuple{ 42 };
    //ᅟ    auto str = tuple
    //ᅟ        | get_or_default<std::string>("bar"sv); // returns "bar"sv
    //
template <typename T, typename DefaultT,
          typename = std::enable_if_t<!is_tuple_like_v<std::decay_t<DefaultT>>>> // TODO: this is not optimal because we might have nested tuples...
    constexpr makeshift::detail::get_or_default_t<T, std::decay_t<DefaultT>>
    get_or_default(DefaultT&& defaultValue)
{
    return { std::forward<DefaultT>(defaultValue) };
}


    //ᅟ
    // Returns a functor which retrieves the tuple element of the given type, or which returns a default-constructed element if the tuple does not
    // contain an element of the given type.
    //ᅟ
    //ᅟ    auto tuple = std::tuple{ 42 };
    //ᅟ    auto str = tuple
    //ᅟ        | get_or_default<std::string>(); // returns ""s
    //
template <typename T>
    constexpr makeshift::detail::get_or_default_t<T, std::decay_t<T>>
    get_or_default(void)
{
    return {{ }};
}


    //ᅟ
    // Returns the tuple element of the given type, or the provided default value if the tuple does not contain an element of the given type.
    // The type of the default value does not need to match the desired element type.
    //ᅟ
    //ᅟ    auto tuple = std::tuple{ 42 };
    //ᅟ    auto str = get_or_default<std::string>(tuple, "bar"sv); // returns "bar"sv
    //
template <typename T, typename TupleT, typename DefaultT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr auto
    get_or_default(TupleT&& tuple, DefaultT&& defaultValue)
{
    return get_or_default<T>(std::forward<DefaultT>(defaultValue))(std::forward<TupleT>(tuple));
}


    //ᅟ
    // Returns the tuple element of the given type, or the provided default value if the tuple does not contain an element of the given type.
    // The type of the default value does not need to match the desired element type.
    //ᅟ
    //ᅟ    auto tuple = std::tuple{ 42 };
    //ᅟ    auto str = get_or_default<std::string>(tuple); // returns ""s
    //
template <typename T, typename TupleT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr T
    get_or_default(TupleT&& tuple)
{
    return get_or_default<T>()(std::forward<TupleT>(tuple));
}


    //ᅟ
    // Returns a functor which retrieves the tuple element of the given type, or `none` if the tuple does not contain an element of the given type.
    //ᅟ
    //ᅟ    auto tuple = std::tuple{ 42 };
    //ᅟ    auto str = tuple
    //ᅟ        | get_or_none<std::string>(); // returns none
    //
template <typename T>
    constexpr makeshift::detail::get_or_default_t<T, none>
    get_or_none(void)
{
    return {{ }};
}


    //ᅟ
    // Returns the tuple element of the given type, or `none` if the tuple does not contain an element of the given type.
    //ᅟ
    //ᅟ    auto tuple = std::tuple{ 42 };
    //ᅟ    auto str = get_or_none<std::string>(tuple); // returns none
    //
template <typename T, typename TupleT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr auto
    get_or_none(TupleT&& tuple)
{
    return get_or_none<T>()(std::forward<TupleT>(tuple));
}


    //ᅟ
    // Returns a functor which retrieves the tuple element of the given type.
    //ᅟ
    //ᅟ    auto tuple = std::tuple{ 42 };
    //ᅟ    auto elem = tuple
    //ᅟ        | get<int>(); // returns 42
    //
template <typename T>
    constexpr makeshift::detail::get_t<T>
    get(void)
{
    return { };
}


    //ᅟ
    // Returns a functor which retrieves the tuple element with the given index. Negative indices count from the end.
    //ᅟ
    //ᅟ    auto tuple = std::tuple{ 42 };
    //ᅟ    auto elem = tuple
    //ᅟ        | get<0>(); // returns 42
    //
template <int I>
    constexpr makeshift::detail::get_by_index_t<I>
    get(void)
{
    return { };
}


    //ᅟ
    // Returns a functor which retrieves the single element in a tuple, or which returns the provided default value if the tuple is empty.
    //ᅟ
    //ᅟ    auto tuple = std::tuple{ 12, 42 };
    //ᅟ    auto elem = tuple
    //ᅟ        | single_or_default(0); // returns 0
    //
template <typename DefaultT>
    constexpr makeshift::detail::single_or_default_t<std::decay_t<DefaultT>>
    single_or_default(DefaultT&& defaultValue)
{
    return { std::forward<DefaultT>(defaultValue) };
}


    //ᅟ
    // Returns the single element in a tuple, or the provided default value if the tuple is empty.
    //ᅟ
    //ᅟ    auto tuple = std::tuple{ 12, 42 };
    //ᅟ    auto elem = single_or_default(tuple, 0); // returns 0
    //
template <typename TupleT, typename DefaultT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr auto
    single_or_default(TupleT&& tuple, DefaultT&& defaultValue)
{
    return single_or_default(std::forward<DefaultT>(defaultValue))(std::forward<TupleT>(tuple));
}


    //ᅟ
    // Returns a functor which retrieves the single element in a tuple, or which returns `none` if the tuple is empty.
    //ᅟ
    //ᅟ    auto tuple = std::tuple{ 12, 42 };
    //ᅟ    auto elem = tuple
    //ᅟ        | single_or_none(); // returns none
    //
constexpr inline makeshift::detail::single_or_default_t<none>
single_or_none(void)
{
    return {{ }};
}


    //ᅟ
    // Returns the single element in a tuple, or `none` if the tuple is empty.
    //ᅟ
    //ᅟ    auto tuple = std::tuple{ 12, 42 };
    //ᅟ    auto elem = single_or_none(tuple); // returns 0
    //
template <typename TupleT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr auto
    single_or_none(TupleT&& tuple)
{
    return single_or_none()(std::forward<TupleT>(tuple));
}


    //ᅟ
    // Returns a functor which retrieves the single element in a tuple.
    //ᅟ
    //ᅟ    auto tuple = std::tuple{ 42 };
    //ᅟ    auto elem = tuple
    //ᅟ        | single(); // returns 42
    //
constexpr inline makeshift::detail::single_t
single(void)
{
    return { };
}


    //ᅟ
    // Returns the single element in a tuple.
    //ᅟ
    //ᅟ    auto tuple = std::tuple{ 42 };
    //ᅟ    auto elem = single(tuple); // returns 42
    //
template <typename TupleT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr auto
    single(TupleT&& tuple)
{
    return single()(std::forward<TupleT>(tuple));
}


    //ᅟ
    // Concatenates the tuples in a tuple of tuples.
    //ᅟ
    //ᅟ    auto tuple = std::tuple{ std::tuple{ 1 }, std::tuple{ 2 } };
    //ᅟ    auto flat_tuple = tuple
    //ᅟ        | tuple_cat(); // returns (1, 2)
    //
constexpr inline makeshift::detail::tuple_cat_t
tuple_cat(void)
{
    return { };
}


    //ᅟ
    // Concatenates the tuples in a tuple of tuples.
    //ᅟ
    //ᅟ    auto tuple = std::tuple{ std::tuple{ 1 }, std::tuple{ 2 } };
    //ᅟ    auto flat_tuple = tuple_cat(tuple); // returns (1, 2)
    //
template <typename TupleT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr auto
    tuple_cat(TupleT&& tuple)
{
    return tuple_cat()(std::forward<TupleT>(tuple));
}


} // inline namespace types

} // namespace makeshift


namespace std
{


    // Specialize `tuple_size<>` and `tuple_element<>` for `type_tuple<>`.
template <typename... Ts> class tuple_size<makeshift::type_tuple<Ts...>> : public std::integral_constant<std::size_t, sizeof...(Ts)> { };
template <std::size_t I, typename... Ts> class tuple_element<I, makeshift::type_tuple<Ts...>> : public makeshift::detail::nth_type_<I, Ts...> { };


} // namespace std


#ifdef INCLUDED_MAKESHIFT_DETAIL_UTILITY_KEYWORD_HPP_
 #include <makeshift/detail/utility_keyword-tuple.hpp>
#endif // INCLUDED_MAKESHIFT_DETAIL_UTILITY_KEYWORD_HPP_


#endif // INCLUDED_MAKESHIFT_TUPLE2_HPP_
