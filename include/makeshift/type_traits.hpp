
#ifndef MAKESHIFT_TYPE_TRAITS_HPP_
#define MAKESHIFT_TYPE_TRAITS_HPP_


#include <type_traits> // for integral_constant<>, declval<>()


namespace makeshift
{

namespace detail
{


struct flags_base { };
struct flags_tag { };

template <typename...> using void_t = void; // ICC doesn't have std::void_t<> yet
template <template <typename...> class, typename, typename...> struct can_apply_1_ : std::false_type { };
template <template <typename...> class Z, typename... Ts> struct can_apply_1_<Z, void_t<Z<Ts...>>, Ts...> : std::true_type { };


} // namespace detail


inline namespace types
{


    // Use as an argument or return type if a type mismatch error is not desired (e.g. because it would be misleading and another error follows anyway),
    // or to enforce the lowest rank in overload resolution (e.g. to define the default behavior for a function in case no matching function is found
    // via argument-dependent lookup).
struct universally_convertible
{
    template <typename T> constexpr universally_convertible(const T&) noexcept { }
    template <typename T> constexpr operator T(void) const;
};

    // Use as an argument type to enforce lowest rank in overload resolution (e.g. to define the default behavior for a function in case no matching
    // function is found via argument-dependent lookup).
template <typename T>
    struct convertible_from
{
    T value;
    constexpr convertible_from(const T& _value) : value(_value) { }
};


    // Determines whether the template instantiation `Z<Ts...>` would be valid. Useful for expression SFINAE.
template <template <typename...> class Z, typename... Ts> struct can_apply : makeshift::detail::can_apply_1_<Z, void, Ts...> { };

    // Determines whether the template instantiation `Z<Ts...>` would be valid. Useful for expression SFINAE.
template <template <typename...> class Z, typename... Ts> constexpr bool can_apply_v = can_apply<Z, Ts...>::value;


    // Type sequence (strictly for compile-time purposes).
template <typename... Ts> struct type_sequence { };


    // Helper for type dispatching.
template <typename T = void> struct tag_t { using type = T; };

    // Helper for type dispatching.
template <typename T = void> constexpr tag_t<T> tag { };


    // Null type for tuple functions, flag enums and other purposes.
struct none_t { };

    // Null element for tuple functions, flag enums and other purposes.
constexpr none_t none { };


    // Determines whether the given type is `none_t`.
template <typename T> struct is_none : std::false_type { };
template <> struct is_none<none_t> : std::true_type { };

    // Determines whether the given type is `none_t`.
template <typename T> constexpr bool is_none_v = is_none<T>::value;


    // Retrieves the flag type (i.e. the `struct` which inherits from `define_flags<>` and defines flag values) of a flags enum.
template <typename T> struct flag_type_of : decltype(flag_type_of_(std::declval<T>(), std::declval<makeshift::detail::flags_tag>())) { };

    // Retrieves the flag type (i.e. the `struct` which inherits from `define_flags<>` and defines flag values) of a flags enum.
template <typename T> using flag_type_of_t = typename flag_type_of<T>::type;


    // Determines whether the given type is a flags enum.
template <typename T> struct is_flags_enum : std::conjunction<std::is_enum<T>, can_apply<flag_type_of, T>> { };

    // Determines whether the given type is a flags enum.
template <typename T> constexpr bool is_flags_enum_v = is_flags_enum<T>::value;


    // Determines whether a type is an instantiation of a particular class template.
template <typename T, template <typename...> class U> struct is_same_template : std::false_type { };
template <template <typename...> class U, typename... Ts> struct is_same_template<U<Ts...>, U> : std::true_type { };

    // Determines whether a type is an instantiation of a particular class template.
template <typename T, template <typename...> class U> constexpr bool is_same_template_v = is_same_template<T, U>::value;


    // Removes an rvalue reference from a type.
template <typename T> struct remove_rvalue_reference { using type = T; };
template <typename T> struct remove_rvalue_reference<T&&> { using type = T; };

    // Removes an rvalue reference from a type.
template <typename T> using remove_rvalue_reference_t = typename remove_rvalue_reference<T>::type;


} // inline namespace types

} // namespace makeshift


#endif // MAKESHIFT_TYPE_TRAITS_HPP_