
#ifndef INCLUDED_MAKESHIFT_METADATA_HPP_
#define INCLUDED_MAKESHIFT_METADATA_HPP_


#include <gsl-lite/gsl-lite.hpp>  // for gsl_CPP17_OR_GREATER

#if !gsl_CPP17_OR_GREATER
# error Header <makeshift/metadata.hpp> requires C++17 mode or higher.
#endif // !gsl_CPP17_OR_GREATER

#include <array>
#include <optional>
#include <string_view>
#include <type_traits>

#include <makeshift/detail/metadata.hpp>


namespace makeshift {


template <typename T> constexpr auto metadata_v = detail::reflector<T>{ }();
template <typename T> constexpr auto metadata_v<T const> = detail::reflector<T>{ }();
template <typename T> using metadata_t = decltype(metadata_v<T>);
//template <typename T> constexpr auto metadata_c = constval_t<detail::reflector<T>>{ };
//template <typename T> constexpr auto metadata_c<T const> = constval_t<detail::reflector<T>>{ };
template <typename T> constexpr auto metadata_c = detail::reflector<T>{ };
template <typename T> constexpr auto metadata_c<T const> = detail::reflector<T>{ };


namespace metadata {


template <typename T>
constexpr std::negation<std::is_same<T, std::nullopt_t>>
is_available(T const&)
{
    return { };
}

template <typename T>
constexpr bool is_available_v = !std::is_same_v<T, std::nullopt_t>;


template <typename MetadataT>
constexpr auto
name(MetadataT const& metadata)
{
    return detail::extract_metadata<void, detail::is_string_like, 0>(metadata);
}

template <typename MetadataT>
constexpr auto
description(MetadataT const& metadata)
{
    return detail::extract_metadata<void, detail::is_string_like, 1>(metadata);
}

template <typename T, typename MetadataT>
constexpr auto
values(MetadataT const& metadata)
{
    return detail::extract_values<T>(metadata);
}

template <typename T, typename MetadataT>
constexpr auto
value_names(MetadataT const& metadata)
{
    return detail::extract_value_metadata<T, detail::is_string_like, 0>(metadata);
}

template <typename T, typename MetadataT>
constexpr auto
value_descriptions(MetadataT const& metadata)
{
    return detail::extract_value_metadata<T, detail::is_string_like, 1>(metadata);
}


} // namespace metadata

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_METADATA_HPP_
