
#ifndef INCLUDED_MAKESHIFT_STRING_HPP_
#define INCLUDED_MAKESHIFT_STRING_HPP_


#include <gsl-lite/gsl-lite.hpp>  // for gsl_CPP17_OR_GREATER

#if !gsl_CPP17_OR_GREATER
# error Header <makeshift/string.hpp> requires C++17 mode or higher.
#endif // !gsl_CPP17_OR_GREATER

#include <string>
#include <string_view>

#include <makeshift/metadata.hpp>

#include <makeshift/detail/serialize.hpp>


namespace makeshift {


template <typename T, typename MetadataC>
constexpr std::string
enum_to_string(T value, MetadataC)
{
    return std::string(detail::enum_to_string(value, detail::static_enum_metadata<T, MetadataC>::value));
}
template <typename T>
constexpr std::string
enum_to_string(T value)
{
    return std::string(detail::enum_to_string(value, detail::static_enum_metadata<T, decltype(metadata_c<T>)>::value));
}

template <typename T, typename MetadataC>
constexpr T
parse_enum(std::string_view str, MetadataC)
{
    return detail::enum_from_string(str, detail::static_enum_metadata<T, MetadataC>::value);
}
template <typename T>
constexpr T
parse_enum(std::string_view str)
{
    return detail::enum_from_string(str, detail::static_enum_metadata<T, decltype(metadata_c<T>)>::value);
}


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_IOMANIP_HPP_
