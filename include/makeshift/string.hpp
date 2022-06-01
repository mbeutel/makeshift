
#ifndef INCLUDED_MAKESHIFT_STRING_HPP_
#define INCLUDED_MAKESHIFT_STRING_HPP_


#include <string>
#include <string_view>

#include <gsl-lite/gsl-lite.hpp>  // for gsl_CPP17_OR_GREATER

#if !gsl_CPP17_OR_GREATER
# error makeshift requires C++17 mode or higher
#endif // !gsl_CPP17_OR_GREATER

#include <makeshift/metadata.hpp>

#include <makeshift/detail/serialize.hpp>


namespace makeshift {

namespace gsl = ::gsl_lite;


template <typename T, typename ReflectorT = reflector>
std::string
enum_to_string(T value, ReflectorT = { })
{
    return std::string(detail::enum_to_string(value, detail::static_enum_metadata<T, ReflectorT>::value));
}

template <typename T, typename ReflectorT = reflector>
constexpr T
parse_enum(std::string_view str, ReflectorT = { })
{
    return detail::enum_from_string(str, detail::static_enum_metadata<T, ReflectorT>::value);
}


template <typename T, typename ReflectorT = reflector>
std::string
flags_to_string(T value, ReflectorT = { })
{
    return detail::flags_to_string(value, detail::static_flags_metadata<T, ReflectorT>::value);
}

template <typename T, typename ReflectorT = reflector>
constexpr T
parse_flags(std::string_view str, ReflectorT = { })
{
    T result;
    detail::flags_from_string(result, str, detail::static_flags_metadata<T, ReflectorT>::value);
    return result;
}


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_STRING_HPP_
