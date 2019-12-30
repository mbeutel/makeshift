
#ifndef INCLUDED_MAKESHIFT_EXPERIMENTAL_TYPE_TRAITS_HPP_
#define INCLUDED_MAKESHIFT_EXPERIMENTAL_TYPE_TRAITS_HPP_


#include <type_traits> // for is_base_of<>

#include <makeshift/detail/type_traits.hpp>


namespace makeshift
{


    //
    // Determines whether the given type is a type enum.
    //
template <typename T> struct is_type_enum : std::is_base_of<detail::type_enum_base, T> { };

    //
    // Determines whether the given type is a type enum.
    //
template <typename T> constexpr bool is_type_enum_v = is_type_enum<T>::value;


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_EXPERIMENTAL_TYPE_TRAITS_HPP_
