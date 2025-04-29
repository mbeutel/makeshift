
#ifndef INCLUDED_MAKESHIFT_CONCEPTS_HPP_
#define INCLUDED_MAKESHIFT_CONCEPTS_HPP_


#include <array>
#include <cstddef>   // for size_t
#include <utility>   // for tuple_size<>

#include <gsl-lite/gsl-lite.hpp>  // for gsl_CPP20_OR_GREATER

#if !gsl_CPP20_OR_GREATER
# error makeshift requires C++20 mode or higher
#endif // !gsl_CPP20_OR_GREATER

#include <concepts>  // for same_as<>, convertible_to<>

#include <makeshift/type_traits.hpp>  // for is_instantiation_of<>, is_type_transportable<>


namespace makeshift {


    //
    // The type must have a tuple-like interface.
    //ᅟ
    // A type `T` has a tuple-like interface if `std::tuple_size<T>::value`, `std::tuple_element_t<I, T>`, and `get<I>(t)` are
    // well-formed.
    // This concept only checks `std::tuple_size<T>::value`.
    //
template <typename T>
concept tuple_like = requires {
    { std::tuple_size<T>::value } -> std::same_as<std::size_t>;
};


    //
    // The type must be a bitmask type, i.e. bitmask operations with `|`, `^`, `~`, and `&` are well-formed.
    //
template <typename T>
concept bitmask = requires(T m) {
    { ~m } -> std::same_as<T>;
    { m | m } -> std::same_as<T>;
    { m & m } -> std::same_as<T>;
    { m ^ m } -> std::same_as<T>;
    { m |= m } -> std::same_as<T&>;
    { m &= m } -> std::same_as<T&>;
    { m ^= m } -> std::same_as<T&>;
};


    //
    // Determines whether an object of type `T` is type-transportable, that is, default-constructible and without state,
    // such that an equivalent object can be reconstructed as `T{ }`.
    // Examples for type-transportable objects are constvals and lambdas without captures (with C++20).
    //
template <typename T>
concept type_transportable = is_type_transportable_v<T>;

    //
    // The type must be a constval, i.e. like `std::integral_constant<>` but for an arbitrary value type.
    //ᅟ
    // Note that `constexpr`-ness is assumed but not checked (this is not feasible with current implementations).
    //
template <typename C>
concept constval = type_transportable<C> && requires {
    typename C::value_type;
} && std::convertible_to<C, typename C::value_type> &&
    requires(C c) {
    { C::value } -> std::same_as<typename C::value_type>;
    { c() } -> std::same_as<typename C::value_type>;
};

    //
    // The type needs to be a constval with an underlying value type convertible to `T`.
    //ᅟ
    // Note that `constexpr`-ness is assumed but not checked (this is not feasible with current implementations).
    //
template <typename C, typename T>
concept constval_of = constval<C> &&
std::convertible_to<typename C::value_type, T>;


    //
    // The type needs to be an instantiation of the given template `U<>`.
    //ᅟ
template <typename T, template <typename...> class U>
concept instantiation_of = is_instantiation_of_v<T, U>;


    //
    // The type is not equal to the given type `U`.
    //ᅟ
template <typename T, typename U>
concept distinct_from = !std::is_same_v<T, U>;


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_CONCEPTS_HPP_
