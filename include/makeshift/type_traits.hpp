﻿
#ifndef INCLUDED_MAKESHIFT_TYPE_TRAITS_HPP_
#define INCLUDED_MAKESHIFT_TYPE_TRAITS_HPP_


#include <cstddef>     // for size_t
#include <type_traits> // for integral_constant<>, declval<>(), is_enum<>, is_same<>, is_base_of<>

#include <makeshift/detail/type_traits.hpp>


namespace makeshift
{


    //ᅟ
    // Represents an integral constval.
    //
//template <typename T, T Value> constexpr std::integral_constant<T, Value> integral_c{ };


    //ᅟ
    // Represents an integer constval.
    //
template <int Value> using int_constant = std::integral_constant<int, Value>;

    //ᅟ
    // Represents an integer constval.
    //
//template <int Value> constexpr int_constant<Value> int_c{ };


    //ᅟ
    // Represents a boolean constval.
    // Equivalent to `std::bool_constant<>` in C++17.
    //
template <bool Value> using bool_constant = std::integral_constant<bool, Value>;

    //ᅟ
    // Represents a boolean constval.
    //
//template <bool Value> constexpr bool_constant<Value> bool_c{ };

    //ᅟ
    // Represents the constval `false`.
    //
//constexpr inline bool_constant<false> false_c{ };

    //ᅟ
    // Represents the constval `true`.
    //
//constexpr inline bool_constant<true> true_c{ };


    //ᅟ
    // Returns the short-circuiting conjunction of the given Boolean traits.
    // Equivalent to `std::conjunction<>` in C++17.
    //
template <typename... Ts> struct conjunction : detail::conjunction<Ts...> { };

    //ᅟ
    // Returns the short-circuiting conjunction of the given Boolean traits.
    // Equivalent to `std::conjunction_v<>` in C++17.
    //
template <typename... Ts> constexpr bool conjunction_v = conjunction<Ts...>::value;

    //ᅟ
    // Returns the short-circuiting disjunction of the given Boolean traits.
    // Equivalent to `std::disjunction<>` in C++17.
    //
template <typename... Ts> struct disjunction : detail::disjunction<Ts...> { };

    //ᅟ
    // Returns the short-circuiting disjunction of the given Boolean traits.
    // Equivalent to `std::disjunction_v<>` in C++17.
    //
template <typename... Ts> constexpr bool disjunction_v = disjunction<Ts...>::value;

    //ᅟ
    // Returns the negation of the given Boolean trait.
    // Equivalent to `std::negation<>` in C++17.
    //
template <typename T> struct negation : std::integral_constant<bool, !T::value> { };

    //ᅟ
    // Returns the negation of the given Boolean trait.
    // Equivalent to `std::negation_v<>` in C++17.
    //
template <typename T> constexpr bool negation_v = negation<T>::value;


    //ᅟ
    // Determines the `N`-th type in the variadic type sequence.
    //
template <std::size_t N, typename... Ts> struct nth_type : detail::nth_type_<N, Ts...> { };

    //ᅟ
    // Determines the `N`-th type in the variadic type sequence.
    //
template <std::size_t N, typename... Ts> using nth_type_t = typename nth_type<N, Ts...>::type;


    //ᅟ
    // Determines the index of the type `T` in the variadic type sequence.
    // Returns `size_t(-1)` if `T` does not appear in the type sequence, or if it appears more than once.
    //
template <typename T, typename... Ts> struct try_index_of_type : detail::try_index_of_type<T, Ts...> { };

    //ᅟ
    // Determines the index of the type `T` in the variadic type sequence.
    // Returns `size_t(-1)` if `T` does not appear in the type sequence, or if it appears more than once.
    //
template <typename T, typename... Ts> constexpr std::size_t try_index_of_type_v = try_index_of_type<T, Ts...>::value;


    //ᅟ
    // Determines the index of the type `T` in the variadic type sequence.
    //
template <typename T, typename... Ts> struct index_of_type : std::integral_constant<std::size_t, detail::type_pack_index_<T, Ts...>::value> { };

    //ᅟ
    // Determines the index of the type `T` in the variadic type sequence.
    //
template <typename T, typename... Ts> constexpr std::size_t index_of_type_v = index_of_type<T, Ts...>::value;

    //ᅟ
    // Determines whether the template instantiation `Z<Ts...>` would be valid.
    //
template <template <typename...> class Z, typename... Ts> struct can_instantiate : detail::can_instantiate_<Z, void, Ts...> { };

    //ᅟ
    // Determines whether the template instantiation `Z<Ts...>` would be valid.
    //
template <template <typename...> class Z, typename... Ts> constexpr bool can_instantiate_v = can_instantiate<Z, Ts...>::value;


    //ᅟ
    // Determines whether a type is an instantiation of a particular class template.
    //
template <typename T, template <typename...> class U> struct is_instantiation_of : std::false_type { };
template <template <typename...> class U, typename... Ts> struct is_instantiation_of<U<Ts...>, U> : std::true_type { };

    //ᅟ
    // Determines whether a type is an instantiation of a particular class template.
    //
template <typename T, template <typename...> class U> constexpr bool is_instantiation_of_v = is_instantiation_of<T, U>::value;


    //ᅟ
    // Determines whether the given type is a bitmask type, i.e. bitmask operations with `|`, `^`, `~`, and `&` are well-formed.
    //
template <typename T> struct is_bitmask_type : detail::is_bitmask_type<T> { };

    //ᅟ
    // Determines whether the given type is a bitmask type, i.e. bitmask operations with `|`, `^`, `~`, and `&` are well-formed.
    //
template <typename T> constexpr bool is_bitmask_type_v = is_bitmask_type<T>::value;


    //ᅟ
    // Determines the underlying type of the given enum type.
    // This trait is like `std::underlying_type<>` but permits specialization with SFINAE. It also supports type enums.
    //
template <typename T, typename = void> struct underlying_type : std::underlying_type<T> { };

    //ᅟ
    // Determines the underlying type of the given enum type.
    // This alias is like `std::underlying_type_t<>` but permits specialization with SFINAE. It also supports type enums.
    //
template <typename T> using underlying_type_t = typename underlying_type<T>::type;


    //ᅟ
    // Determines whether the given type is a type enum.
    //
template <typename T> struct is_type_enum : std::is_base_of<detail::type_enum_base, T> { };

    //ᅟ
    // Determines whether the given type is a type enum.
    //
template <typename T> constexpr bool is_type_enum_v = is_type_enum<T>::value;


    //ᅟ
    // Determines whether the given type is in the list of types.
    //
template <typename T, typename... Ts> struct is_in : detail::is_in_<T, Ts...> { };

    //ᅟ
    // Determines whether the given type is in the list of types.
    //
template <typename T, typename... Ts> constexpr bool is_in_v = is_in<T>::value;


    //ᅟ
    // Given a type sequence `T<Ts...>`, returns a type sequence `T<Us...>` such that every type `Us` is in `Ts...` and no type in `Us...` appears more than once.
    //
template <typename Ts> struct unique_sequence : detail::unique_sequence_<Ts> { };

    //ᅟ
    // Given a type sequence `T<Ts...>`, returns a type sequence `T<Us...>` such that every type `Us` is in `Ts...` and no type in `Us...` appears more than once.
    //
template <typename Ts> using unique_sequence_t = typename unique_sequence<Ts>::type;


    //ᅟ
    // Determines whether a type has a tuple-like interface.
    //ᅟ
    // A type `T` has a tuple-like interface if `std::tuple_size<T>::value`, `std::tuple_element_t<I, T>`, and `get<I>(t)` are well-formed. This trait only checks `std::tuple_size<T>::value`.
    //
template <typename T> struct is_tuple_like : detail::is_tuple_like<T> { };

    //ᅟ
    // Determines whether a type has a tuple-like interface.
    //ᅟ
    // A type `T` has a tuple-like interface if `std::tuple_size<T>::value`, `std::tuple_element_t<I, T>`, and `get<I>(t)` are well-formed. This trait only checks `std::tuple_size<T>::value`.
    //
template <typename T> constexpr bool is_tuple_like_v = is_tuple_like<T>::value;


    //ᅟ
    // Determines whether the given type is a constval.
    //
template <typename T> struct is_constval : detail::is_constval_<T> { };

    //ᅟ
    // Determines whether the given type is a constval.
    //
template <typename T> constexpr bool is_constval_v = is_constval<T>::value;


    //ᅟ
    // Determines whether the given type `T` is a constval returning the type `R`.
    //
template <typename T, typename R> struct is_constval_of_type : detail::is_constval_of_type_<T, R> { };

    //ᅟ
    // Determines whether the given type `T` is a constval returning the type `R`.
    //
template <typename T, typename R> constexpr bool is_constval_of_type_v = is_constval_of_type<T, R>::value;


    //ᅟ
    // Retrieves the given type as a dependent type. This can be useful to suppress type inference.
    //
template <typename T> using as_dependent_type = typename detail::as_dependent_type_<T>::type; // TODO: rename type_identity_t<>, move to gsl-lite (also have identity<> there)


    //ᅟ
    // Contains a default-constructed `constexpr` instance of type `T`.
    // Useful to avoid ODR violations when defining constexpr objects in C++14 (without C++17 `constexpr inline` available).
    //ᅟ
    //ᅟ    static constexpr S const& singleton = static_const<S>;
    //
template <typename T> constexpr T static_const{ };


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_TYPE_TRAITS_HPP_
