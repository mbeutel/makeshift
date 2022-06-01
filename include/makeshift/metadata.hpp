
#ifndef INCLUDED_MAKESHIFT_METADATA_HPP_
#define INCLUDED_MAKESHIFT_METADATA_HPP_


#include <array>
#include <optional>
#include <string_view>
#include <type_traits>

#include <gsl-lite/gsl-lite.hpp>  // for gsl_CPP17_OR_GREATER

#if !gsl_CPP17_OR_GREATER
# error makeshift requires C++17 mode or higher
#endif // !gsl_CPP17_OR_GREATER

#include <makeshift/detail/metadata.hpp>


namespace makeshift {

namespace gsl = ::gsl_lite;


using reflector = detail::reflector;
constexpr auto reflector_c = reflector{ };
template <typename T> constexpr auto metadata_v = reflector_c(gsl::type_identity<std::remove_cv_t<T>>{ });
template <typename T> using metadata_t = std::remove_const_t<decltype(metadata_v<T>)>;


namespace metadata {


template <typename T>
constexpr std::negation<std::is_same<T, std::nullopt_t>>
is_available(T const&)
{
    return { };
}

template <typename T>
constexpr bool is_available_v = !std::is_same_v<T, std::nullopt_t>;


template <typename T, typename MetadataOrReflectorT = reflector>
constexpr auto
name(MetadataOrReflectorT&& md = { })
{
    return detail::extract_metadata<void, detail::is_string_like, 0>(detail::unwrap_metadata<T>(md));
}

template <typename T, typename MetadataOrReflectorT = reflector>
constexpr auto
description(MetadataOrReflectorT&& md = { })
{
    return detail::extract_metadata<void, detail::is_string_like, 1>(detail::unwrap_metadata<T>(md));
}

template <typename T, typename MetadataOrReflectorT = reflector>
constexpr auto
bases(MetadataOrReflectorT&& md = { })
{
    return detail::extract_bases<std::remove_const_t<T>>(detail::unwrap_metadata<T>(md));
}

template <typename T, typename MetadataOrReflectorT = reflector>
constexpr auto
values(MetadataOrReflectorT&& md = { })
{
    return detail::extract_values<std::remove_const_t<T>>(detail::unwrap_metadata<T>(md));
}

template <typename T, typename MetadataOrReflectorT = reflector>
constexpr auto
value_names(MetadataOrReflectorT&& md = { })
{
    return detail::extract_value_metadata<std::remove_const_t<T>, std::string_view, detail::is_string_like, 0>(detail::unwrap_metadata<T>(md));
}

template <typename T, typename MetadataOrReflectorT = reflector>
constexpr auto
value_descriptions(MetadataOrReflectorT&& md = { })
{
    return detail::extract_value_metadata<std::remove_const_t<T>, std::string_view, detail::is_string_like, 1>(detail::unwrap_metadata<T>(md));
}

template <typename T, typename MetadataOrReflectorT = reflector>
constexpr auto
members(MetadataOrReflectorT&& md = { })
{
    return detail::extract_members<std::remove_const_t<T>>(detail::unwrap_metadata<T>(md));
}

template <typename T, typename MetadataOrReflectorT = reflector>
constexpr auto
member_names(MetadataOrReflectorT&& md = { })
{
    return detail::extract_member_metadata<std::remove_const_t<T>, std::string_view, detail::is_string_like, 0>(detail::unwrap_metadata<T>(md));
}

template <typename T, typename MetadataOrReflectorT = reflector>
constexpr auto
member_descriptions(MetadataOrReflectorT&& md = { })
{
    return detail::extract_member_metadata<std::remove_const_t<T>, std::string_view, detail::is_string_like, 1>(detail::unwrap_metadata<T>(md));
}


} // namespace metadata

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_METADATA_HPP_
