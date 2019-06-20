
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


    //ᅟ
    // Pass `array_index` to `array_transform()`, `tuple_foreach()`, or `tuple_transform()` to have the array element index passed as a functor argument.
    // The argument is of type `index`.
    //ᅟ
    //ᅟ    auto indices = array_transform<3>(
    //ᅟ        [](index i) { return i; },
    //ᅟ        array_index);
    //ᅟ    // returns std::array<index, 3>{ 0, 1, 2 }
    //
struct array_index_t { };


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


template <typename T> struct is_tuple_arg_0 : can_instantiate<is_tuple_like_r, T> { };
template <> struct is_tuple_arg_0<array_index_t> : std::true_type { };
template <> struct is_tuple_arg_0<tuple_index_t> : std::true_type { };
template <typename T> using is_tuple_arg = is_tuple_arg_0<std::decay_t<T>>;
template <typename T> constexpr bool is_tuple_arg_v = is_tuple_arg<T>::value;

template <typename... Ts> struct are_tuple_args : std::conjunction<is_tuple_arg<Ts>...> { };
template <typename... Ts> constexpr bool are_tuple_args_v = are_tuple_args<Ts...>::value;

template <typename T> struct maybe_tuple_size_ : std::tuple_size<T> { };
template <> struct maybe_tuple_size_<array_index_t> : std::integral_constant<std::ptrdiff_t, -1> { };
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
    constexpr std::ptrdiff_t get(array_index_t) noexcept
{
    return I;
}
template <std::size_t I>
    constexpr std::integral_constant<std::ptrdiff_t, I> get(tuple_index_t) noexcept
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


struct tuple_foreach_tag { };
template <template <typename, std::size_t> class ArrayT> struct transform_to_array_tag { };
template <template <typename, std::size_t> class ArrayT, typename T> struct transform_to_array_of_tag { };
struct transform_to_tuple_tag { };
template <template <typename...> class TupleT> struct transform_to_custom_tuple_tag { };

template <std::size_t I, typename... Ts, typename F>
    constexpr decltype(auto)
    tuple_transform_impl2(F&& func, Ts&&... args)
{
    return func(makeshift::detail::get_element<I>(std::forward<Ts>(args))...);
}

template <typename F, typename... Ts>
    constexpr void
    tuple_transform_impl1(tuple_foreach_tag, std::index_sequence<>, F&&, Ts&&...)
{
    // extra overload to avoid unused-parameter warning
}
template <std::size_t... Is, typename F, typename... Ts>
    constexpr void
    tuple_transform_impl1(tuple_foreach_tag, std::index_sequence<Is...>, F&& func, Ts&&... args)
{
    (void) func;
    using Swallow = int[];
    (void) Swallow{ 1,
        (makeshift::detail::tuple_transform_impl2<Is>(func, std::forward<Ts>(args)...), void(), int{ })...
    };
}

     // defined in makeshift/detail/tuple.hpp
template <std::size_t... Is, typename F, typename... Ts>
    constexpr auto
    tuple_transform_impl1(transform_to_tuple_tag, std::index_sequence<Is...>, F&& func, Ts&&... args);
template <template <typename...> class TupleT, std::size_t... Is, typename F, typename... Ts>
    constexpr auto
    tuple_transform_impl1(transform_to_custom_tuple_tag<TupleT>, std::index_sequence<Is...>, F&& func, Ts&&... args);

     // defined in makeshift/detail/array.hpp
template <template <typename, std::size_t> class ArrayT, std::size_t... Is, typename F, typename... Ts>
    constexpr auto
    tuple_transform_impl1(transform_to_array_tag<ArrayT>, std::index_sequence<Is...>, F&& func, Ts&&... args);
template <template <typename, std::size_t> class ArrayT, typename R, std::size_t... Is, typename F, typename... Ts>
    constexpr auto
    tuple_transform_impl1(transform_to_array_of_tag<ArrayT, R>, std::index_sequence<Is...>, F&& func, Ts&&... args);

template <std::ptrdiff_t N, typename TransformTypeTag, typename F, typename... Ts>
    constexpr auto tuple_transform_impl0(F&& func, Ts&&... args)
{
        // TODO: all forms of this function could use a loop iteration if:
        //  - all arguments have homogeneous elements (i.e. all are std::array<>; do we permit array_index? or do we make a distinction between array_index (runtime val) and tuple_index (constval)?)
        //  - the result type is void or is trivially default-constructible (or it is composed of trivially default-constructible things)
        // A loop iteration has the potential benefit of generating less code, with the compiler being free to unroll the loop for trivial operations.
        // TODO: It is not clear why we wouldn't want a loop iteration if the result type isn't trivially default-constructible. But this is challenging to solve in the general case. How good is the compiler
        // at avoiding zero-init if it proves unnecessary?
        // TODO: we might want this even if the objects are not default-constructible but are trivially movable. How good is the compiler at eliding the move? Not good at all, I'd guess...
        // Do we default-initialize arrays or tuples of trivially default-constructible things? Note that there might be compiler and runtime warnings in VC++ if we don't! (TODO: try & check this)

        // TODO: how about other types of arrays or tuples? E.g. Thrust's tuple type or type_sequence<>. Perhaps we could have an overload with a template template parameter in the interface function?

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
