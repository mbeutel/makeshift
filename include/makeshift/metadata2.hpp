
#ifndef INCLUDED_MAKESHIFT_METADATA2_HPP_
#define INCLUDED_MAKESHIFT_METADATA2_HPP_


#include <type_traits> // for decay<>

#include <makeshift/version.hpp> // for MAKESHIFT_NODISCARD

#include <makeshift/detail/string_view.hpp>
#include <makeshift/detail/metadata2.hpp>


namespace makeshift
{

inline namespace metadata
{


constexpr inline makeshift::detail::name_parameter_name name = { };

template <typename T> constexpr inline makeshift::detail::values_parameter_name<T> values = { };

constexpr inline makeshift::detail::value_names_parameter_name value_names = { };

template <typename T> constexpr inline makeshift::detail::named_values_parameter_name<T> named_values = { };


template <typename... ParamsT>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::parameter_set<ParamsT...> define_metadata(ParamsT... params)
{
    return { params... };
}


} // inline namespace metadata

} // namespace makeshift


#endif // INCLUDED_MAKEHIFT_METADATA2_HPP_
