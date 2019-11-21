
#ifndef INCLUDED_MAKESHIFT_REFLECT_HPP_
#define INCLUDED_MAKESHIFT_REFLECT_HPP_


#include <makeshift/constval.hpp>    // for make_constval_t<>
#include <makeshift/type_traits.hpp> // for can_instantiate<>

#include <makeshift/detail/reflect.hpp>


namespace makeshift
{


    //ᅟ
    // Specialize `values_provider<>` with `enable_if_t<>` or `void_t<>` to support retrieving values for all types matching a condition.
    //ᅟ
    //ᅟ    struct TagBase { };
    //ᅟ    template <typename T> struct values_provider<T, std::enable_if_t<std::is_base_of_v<TagBase, T>>> {
    //ᅟ        constexpr auto operator ()(void) const {
    //ᅟ            return std::array{ T{ } }; // all tags are stateless and have only one possible value
    //ᅟ        };
    //ᅟ    };
    //
template <typename T, typename = void> struct values_provider { };

    //ᅟ
    // Determines if a `std::array<>` of possible values of type `T` can be retrieved with `values_of<>`.
    //
template <typename T> struct have_values_of : can_instantiate<makeshift::detail::values_of_r, T> { };

    //ᅟ
    // Determines if a `std::array<>` of possible values of type `T` can be retrieved with `values_of<>`.
    //
template <typename T> constexpr bool have_values_of_v = have_values_of<T>::value;

    //ᅟ
    // Retrieves a `std::array<>` of possible values of type `T`.
    //ᅟ
    // User-defined types can provide a list of admissible values by defining a `reflect_values()` function in the same namespace:
    //ᅟ
    //ᅟ    enum Color { red, green, blue };
    //ᅟ    constexpr auto reflect_values(type<Color>) { return std::array{ red, green, blue }; }
    //
template <typename T> constexpr makeshift::detail::values_of_r<T> values_of = makeshift::detail::values_of_<T>{ }();


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_REFLECT_HPP_
