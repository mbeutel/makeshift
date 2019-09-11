
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
template <typename... Ts> struct conjunction : makeshift::detail::conjunction<Ts...> { };

    //ᅟ
    // Returns the short-circuiting conjunction of the given Boolean traits.
    // Equivalent to `std::conjunction_v<>` in C++17.
    //
template <typename... Ts> constexpr bool conjunction_v = conjunction<Ts...>::value;

    //ᅟ
    // Returns the short-circuiting disjunction of the given Boolean traits.
    // Equivalent to `std::disjunction<>` in C++17.
    //
template <typename... Ts> struct disjunction : makeshift::detail::disjunction<Ts...> { };

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
template <std::size_t N, typename... Ts> struct nth_type : makeshift::detail::nth_type_<N, Ts...> { };

    //ᅟ
    // Determines the `N`-th type in the variadic type sequence.
    //
template <std::size_t N, typename... Ts> using nth_type_t = typename nth_type<N, Ts...>::type;


    //ᅟ
    // Determines the index of the type `T` in the variadic type sequence.
    // Returns `size_t(-1)` if `T` does not appear in the type sequence, or if it appears more than once.
    //
template <typename T, typename... Ts> struct try_index_of_type : makeshift::detail::try_index_of_type<T, Ts...> { };

    //ᅟ
    // Determines the index of the type `T` in the variadic type sequence.
    // Returns `size_t(-1)` if `T` does not appear in the type sequence, or if it appears more than once.
    //
template <typename T, typename... Ts> constexpr std::size_t try_index_of_type_v = try_index_of_type<T, Ts...>::value;


    //ᅟ
    // Determines the index of the type `T` in the variadic type sequence.
    //
template <typename T, typename... Ts> struct index_of_type : std::integral_constant<std::size_t, makeshift::detail::type_pack_index_<T, Ts...>::value> { };

    //ᅟ
    // Determines the index of the type `T` in the variadic type sequence.
    //
template <typename T, typename... Ts> constexpr std::size_t index_of_type_v = index_of_type<T, Ts...>::value;
    //ᅟ
    // Determines whether the template instantiation `Z<Ts...>` would be valid.
    //
template <template <typename...> class Z, typename... Ts> struct can_instantiate : makeshift::detail::can_instantiate_<Z, void, Ts...> { };

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
    // Determines whether the given type is iterable, i.e. functions `begin()` and `end()` are well-defined for arguments of type `T`.
    //
template <typename T> struct is_iterable : can_instantiate<makeshift::detail::is_iterable_ns::is_iterable_r, T> { }; // TODO: remove?

    //ᅟ
    // Determines whether the given type is iterable, i.e. functions `begin()` and `end()` are well-defined for arguments of type `T`.
    //
template <typename T> constexpr bool is_iterable_v = is_iterable<T>::value; // TODO: remove?


    //ᅟ
    // Determines whether the given type is a bitmask type, i.e. bitmask operations with `|`, `^`, `~`, and `&` are well-formed.
    //
template <typename T> struct is_bitmask_type : makeshift::detail::is_bitmask_type<T> { };

    //ᅟ
    // Determines whether the given type is a bitmask type, i.e. bitmask operations with `|`, `^`, `~`, and `&` are well-formed.
    //
template <typename T> constexpr bool is_bitmask_type_v = is_bitmask_type<T>::value;


    //ᅟ
    // Helper alias for SFINAE in specializations.
    // Equivalent to `std::void_t<>` in C++17.
    //
template <typename...> using void_t = void;


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
template <typename T> struct is_type_enum : std::is_base_of<makeshift::detail::type_enum_base, T> { };

    //ᅟ
    // Determines whether the given type is a type enum.
    //
template <typename T> constexpr bool is_type_enum_v = is_type_enum<T>::value;


    //ᅟ
    // Determines whether a type has a tuple-like interface.
    //ᅟ
    // A type `T` has a tuple-like interface if `std::tuple_size<T>::value`, `std::tuple_element_t<I, T>`, and `get<I>(t)` are well-formed. This trait only checks `std::tuple_size<T>::value`.
    //
template <typename T> struct is_tuple_like : makeshift::detail::is_tuple_like<T> { };

    //ᅟ
    // Determines whether a type has a tuple-like interface.
    //ᅟ
    // A type `T` has a tuple-like interface if `std::tuple_size<T>::value`, `std::tuple_element_t<I, T>`, and `get<I>(t)` are well-formed. This trait only checks `std::tuple_size<T>::value`.
    //
template <typename T> constexpr bool is_tuple_like_v = is_tuple_like<T>::value;


    //ᅟ
    // Determines whether the given type is a constval.
    //
template <typename T> struct is_constval : makeshift::detail::is_constval_<T> { };

    //ᅟ
    // Determines whether the given type is a constval.
    //
template <typename T> constexpr bool is_constval_v = is_constval<T>::value;


    //ᅟ
    // Determines whether the given type `T` is a constval returning the type `R`.
    //
template <typename T, typename R> struct is_constval_of_type : makeshift::detail::is_constval_of_type_<T, R> { };

    //ᅟ
    // Determines whether the given type `T` is a constval returning the type `R`.
    //
template <typename T, typename R> constexpr bool is_constval_of_type_v = is_constval_of_type<T, R>::value;


    //ᅟ
    // Retrieves the given type as a dependent type. This can be useful to suppress type inference.
    //
template <typename T> using as_dependent_type = typename makeshift::detail::as_dependent_type_<T>::type;


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_TYPE_TRAITS_HPP_
