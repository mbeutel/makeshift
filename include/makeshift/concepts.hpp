
#ifndef INCLUDED_MAKESHIFT_CONCEPTS_HPP_
#define INCLUDED_MAKESHIFT_CONCEPTS_HPP_


#include <cstddef> // for size_t
#include <utility> // for tuple_size<> (C++20)


#if defined(__has_include)
# if __has_include(<concepts>)
#  include <concepts> // for same_as<>, convertible_to<>
# endif // __has_include(<concepts>)
#endif // defined(__has_include)


namespace makeshift {


#if defined(__cpp_concepts) && defined(__cpp_lib_concepts)
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
    // The type must be a constval, i.e. like `std::integral_constant<>` but for an arbitrary value type.
    //ᅟ
    // Note that `constexpr`-ness is assumed but not checked (this is not feasible with current implementations).
    //
template <typename C>
concept constval = requires {
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
#endif // defined(__cpp_concepts) && defined(__cpp_lib_concepts)


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_CONCEPTS_HPP_
