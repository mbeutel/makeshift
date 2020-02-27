
#ifndef INCLUDED_MAKESHIFT_TYPE_TRAITS_HPP_
#define INCLUDED_MAKESHIFT_TYPE_TRAITS_HPP_


#include <cstddef>     // for size_t
#include <type_traits> // for integral_constant<>, declval<>(), is_enum<>, is_same<>

#include <makeshift/detail/type_traits.hpp>


namespace makeshift {


    //
    // Determines the `N`-th type in the variadic type sequence.
    //
template <std::size_t N, typename... Ts>
struct nth_type : detail::nth_type_<N, Ts...> { };

    //
    // Determines the `N`-th type in the variadic type sequence.
    //
template <std::size_t N, typename... Ts>
using nth_type_t = typename nth_type<N, Ts...>::type;


    //
    // Determines the index of the type `T` in the variadic type sequence.
    // Returns a value < 0 if `T` does not appear in the type sequence, or if it appears more than once.
    //
template <typename T, typename... Ts>
struct search_type_pack_index : detail::search_type_pack_index<T, Ts...> { };

    //
    // Determines the index of the type `T` in the variadic type sequence.
    // Returns a value < 0 if `T` does not appear in the type sequence, or if it appears more than once.
    //
template <typename T, typename... Ts>
constexpr std::ptrdiff_t
search_type_pack_index_v = search_type_pack_index<T, Ts...>::value;


    //
    // Determines the index of the element type `T` in the tuple.
    // Returns a value < 0 if `T` does not appear in the tuple, or if it appears more than once.
    //
template <typename T, typename TupleT>
struct search_tuple_element_index : detail::search_tuple_element_index<TupleT, detail::same_as_pred<T>> { };

    //
    // Determines the index of the element type `T` in the tuple.
    // Returns a value < 0 if `T` does not appear in the tuple, or if it appears more than once.
    //
template <typename T, typename TupleT>
constexpr std::ptrdiff_t
search_tuple_element_index_v = search_tuple_element_index<T, TupleT>::value;


    //
    // Determines the index of the type `T` in the variadic type sequence.
    //
template <typename T, typename... Ts>
struct type_pack_index : std::integral_constant<std::size_t, detail::type_pack_index_<T, Ts...>::value> { };

    //
    // Determines the index of the type `T` in the variadic type sequence.
    //
template <typename T, typename... Ts>
constexpr std::size_t
type_pack_index_v = type_pack_index<T, Ts...>::value;


    //
    // Determines the index of the element type `T` in the tuple.
    //
template <typename T, typename TupleT>
struct tuple_element_index : std::integral_constant<std::size_t, detail::tuple_element_index<TupleT, detail::same_as_pred<T>>::value> { };

    //
    // Determines the index of the element type `T` in the tuple.
    //
template <typename T, typename TupleT>
constexpr std::size_t
tuple_element_index_v = tuple_element_index<T, TupleT>::value;


    //
    // Determines whether the template instantiation `Z<Ts...>` would be valid.
    //
template <template <typename...> class Z, typename... Ts>
struct can_instantiate : detail::can_instantiate_<Z, void, Ts...> { };

    //
    // Determines whether the template instantiation `Z<Ts...>` would be valid.
    //
template <template <typename...> class Z, typename... Ts>
constexpr bool
can_instantiate_v = can_instantiate<Z, Ts...>::value;


    //
    // Determines whether a type is an instantiation of a particular class template.
    //
template <typename T, template <typename...> class U>
struct is_instantiation_of : std::false_type { };
template <template <typename...> class U, typename... Ts>
struct is_instantiation_of<U<Ts...>, U> : std::true_type { };

    //
    // Determines whether a type is an instantiation of a particular class template.
    //
template <typename T, template <typename...> class U>
constexpr bool
is_instantiation_of_v = is_instantiation_of<T, U>::value;


    //
    // Determines whether the given type is a bitmask type, i.e. bitmask operations with `|`, `^`, `~`, and `&` are well-formed.
    //
template <typename T>
struct is_bitmask : detail::is_bitmask<T> { };

    //
    // Determines whether the given type is a bitmask type, i.e. bitmask operations with `|`, `^`, `~`, and `&` are well-formed.
    //
template <typename T>
constexpr bool
is_bitmask_v = is_bitmask<T>::value;


    //
    // Determines whether the given type is in the list of types.
    //
template <typename T, typename... Ts>
struct is_in : detail::is_in_<T, Ts...> { };

    //
    // Determines whether the given type is in the list of types.
    //
template <typename T, typename... Ts>
constexpr bool
is_in_v = is_in<T>::value;


    //
    // Given a type sequence `T<Ts...>`, returns a type sequence `T<Us...>` such that every type `Us` is in `Ts...` and no type in `Us...` appears more than once.
    //
template <typename Ts>
struct unique_sequence : detail::unique_sequence_<Ts> { };

    //
    // Given a type sequence `T<Ts...>`, returns a type sequence `T<Us...>` such that every type `Us` is in `Ts...` and no type in `Us...` appears more than once.
    //
template <typename Ts>
using unique_sequence_t = typename unique_sequence<Ts>::type;


    //
    // Determines whether a type has a tuple-like interface.
    //ᅟ
    // A type `T` has a tuple-like interface if `std::tuple_size<T>::value`, `std::tuple_element_t<I, T>`, and `get<I>(t)` are well-formed. This trait only checks `std::tuple_size<T>::value`.
    //
template <typename T>
struct is_tuple_like : detail::is_tuple_like<T> { };

    //
    // Determines whether a type has a tuple-like interface.
    //ᅟ
    // A type `T` has a tuple-like interface if `std::tuple_size<T>::value`, `std::tuple_element_t<I, T>`, and `get<I>(t)` are well-formed. This trait only checks `std::tuple_size<T>::value`.
    //
template <typename T>
constexpr bool
is_tuple_like_v = is_tuple_like<T>::value;


    //
    // Determines whether the given type is a constval type.
    //
template <typename T>
struct is_constval : detail::is_constval_<T> { };

    //
    // Determines whether the given type is a constval type.
    //
template <typename T>
constexpr bool
is_constval_v = is_constval<T>::value;


    //
    // Determines whether the given type `T` is a constval returning the type `R`.
    //
template <typename T, typename R>
struct is_constval_of : detail::is_constval_of_<T, R> { };

    //
    // Determines whether the given type `T` is a constval returning the type `R`.
    //
template <typename T, typename R>
constexpr bool
is_constval_of_v = is_constval_of<T, R>::value;


    //
    // Contains a default-constructed `constexpr` instance of type `T`.
    //ᅟ
    // Useful to avoid ODR violations when defining constexpr objects in C++14 (without C++17 `constexpr inline` available).
    //ᅟ
    //ᅟ    static constexpr S const& singleton = static_const<S>;
    //
template <typename T>
constexpr T
static_const{ };


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_TYPE_TRAITS_HPP_
