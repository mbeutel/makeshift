
#ifndef INCLUDED_MAKESHIFT_IOMANIP_HPP_
#define INCLUDED_MAKESHIFT_IOMANIP_HPP_


#include <gsl-lite/gsl-lite.hpp>  // for gsl_CPP17_OR_GREATER

#if !gsl_CPP17_OR_GREATER
# error Header <makeshift/iomanip.hpp> requires C++17 mode or higher.
#endif // !gsl_CPP17_OR_GREATER

#include <type_traits>  // for remove_reference<>

#include <makeshift/metadata.hpp>

#include <makeshift/detail/iomanip.hpp>


namespace makeshift {


template <typename T, typename MetadataC>
constexpr detail::enum_manipulator<T, MetadataC>
as_enum(T&& value, MetadataC)
{
    return { value };
}
template <typename T>
constexpr detail::enum_manipulator<T, decltype(metadata_c<std::remove_reference_t<T>>)>
as_enum(T&& value)
{
    return { value };
}


template <typename T, typename MetadataC>
constexpr detail::flags_manipulator<T, MetadataC>
as_flags(T&& value, MetadataC)
{
    return { value };
}
template <typename T>
constexpr detail::flags_manipulator<T, decltype(metadata_c<std::remove_reference_t<T>>)>
as_flags(T&& value)
{
    return { value };
}


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_IOMANIP_HPP_
