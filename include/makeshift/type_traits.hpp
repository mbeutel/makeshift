
#ifndef MAKESHIFT_TYPE_TRAITS_HPP_
#define MAKESHIFT_TYPE_TRAITS_HPP_


#include <type_traits> // for integral_constant<>


namespace makeshift
{

namespace detail
{

struct flags_base { };
struct flags_tag { };

struct universally_convertible
{
    template <typename T> operator T(void) const;
};

template <typename...> using void_t = void; // ICC doesn't have std::void_t<> yet
template <template <typename...> class, typename, typename...> struct can_apply_1_ : std::false_type { };
template <template <typename...> class Z, typename... Ts> struct can_apply_1_<Z, void_t<Z<Ts...>>, Ts...> : std::true_type { };

} // namespace detail


inline namespace types
{

    // Determines whether the template instantiation `Z<Ts...>` would be valid. Useful for expression SFINAE.
template <template <typename...> class Z, typename... Ts> using can_apply = makeshift::detail::can_apply_1_<Z, void, Ts...>;
template <template <typename...> class Z, typename... Ts> constexpr bool can_apply_v = can_apply<Z, Ts...>::value;


    // Type sequence (strictly for compile-time purposes).
template <typename... Ts> struct type_sequence { };


    // Helper for type dispatching.
template <typename T = void> struct tag_t { using type = T; };
template <typename T = void> constexpr tag_t<T> tag { };


    // Null type for tuple functions.
struct none_t { };
constexpr none_t none { };


    // Determines whether the given type is `none_t`.
template <typename T> struct is_none : std::false_type { };
template <> struct is_none<none_t> : std::true_type { };
template <typename T> constexpr bool is_none_v = is_none<T>::value;


    // Retrieves the flag type (i.e. the `struct` which inherits from `define_flags<>` and defines flag values) of a flags enum.
template <typename T> struct flag_type_of : decltype(flag_type_of_(std::declval<T>(), std::declval<makeshift::detail::flags_tag>())) { };
template <typename T> using flag_type_of_t = typename flag_type_of<T>::type;


    // Determines whether the given type is a flags enum.
template <typename T> struct is_flags_enum : std::conjunction<std::is_enum<T>, can_apply<flag_type_of, T>> { };
template <typename T> constexpr bool is_flags_enum_v = is_flags_enum<T>::value;


} // inline namespace types

} // namespace makeshift


namespace makeshift
{

namespace detail
{

template <typename T, template <typename...> class U> struct is_same_template_ : std::false_type { };
template <template <typename...> class U, typename... Ts> struct is_same_template_<U<Ts...>, U> : std::true_type { };

} // namespace detail


inline namespace types
{

    // Determines whether a type is an instantiation of a particular class template.
template <typename T, template <typename...> class U> using is_same_template = makeshift::detail::is_same_template_<T, U>;
template <typename T, template <typename...> class U> constexpr bool is_same_template_v = is_same_template<T, U>::value;


    // Removes an rvalue reference from a type.
template <typename T> struct remove_rvalue_reference { using type = T; };
template <typename T> struct remove_rvalue_reference<T&&> { using type = T; };
template <typename T> using remove_rvalue_reference_t = typename remove_rvalue_reference<T>::type;

} // inline namespace types

} // namespace makeshift

#endif // MAKESHIFT_TYPE_TRAITS_HPP_
