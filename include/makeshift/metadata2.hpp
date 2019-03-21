
#ifndef INCLUDED_MAKESHIFT_METADATA2_HPP_
#define INCLUDED_MAKESHIFT_METADATA2_HPP_


#include <string_view>
#include <type_traits> // for decay<>

#include <makeshift/version.hpp> // for MAKESHIFT_NODISCARD

#include <makeshift/detail/metadata2.hpp>


namespace makeshift
{

inline namespace metadata
{


constexpr makeshift::detail::parameter<makeshift::detail::name_t> name = { };

template <typename T> constexpr inline makeshift::detail::array_parameter<makeshift::detail::values_t, T> values = { };

template <typename T> constexpr inline makeshift::detail::array_parameter<makeshift::detail::named_values_t, makeshift::detail::named_t<T>> named_values = { };


template <typename... ParamsT>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::metadata_t<ParamsT...> define_metadata(ParamsT... params)
{
    return { params... };
};


} // inline namespace metadata

} // namespace makeshift


#endif // INCLUDED_MAKEHIFT_METADATA2_HPP_
