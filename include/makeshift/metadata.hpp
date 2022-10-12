
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

template <typename T, template <typename...> class PredT, typename... PredArgsT, typename MetadataOrReflectorT = reflector>
[[nodiscard]] constexpr auto
extract(MetadataOrReflectorT md = { }, type_sequence<PredArgsT...> = { })
{
    return detail::extract_metadata<void, detail::predicate_adaptor<PredT>::template type, 0>(detail::unwrap_metadata<T>(md), type_sequence<PredArgsT...>{ });
}
template <typename T, int Occurrence, template <typename...> class PredT, typename... PredArgsT, typename MetadataOrReflectorT = reflector>
[[nodiscard]] constexpr auto
extract(MetadataOrReflectorT md = { }, type_sequence<PredArgsT...> = { })
{
    static_assert(Occurrence >= 0);
    return detail::extract_metadata<void, detail::predicate_adaptor<PredT>::template type, Occurrence>(detail::unwrap_metadata<T>(md), type_sequence<PredArgsT...>{ });
}

template <typename T, typename V, template <typename...> class PredT, typename... PredArgsT, typename MetadataOrReflectorT = reflector>
[[nodiscard]] constexpr decltype(auto)
extract_for_values(MetadataOrReflectorT md = { }, type_sequence<PredArgsT...> = { })
{
    return detail::extract_value_metadata<T, V, detail::predicate_adaptor<PredT>::template type, 0, type_sequence<PredArgsT...>>(detail::unwrap_metadata<T>(md));
}
template <typename T, typename V, int Occurrence, template <typename...> class PredT, typename... PredArgsT, typename MetadataOrReflectorT = reflector>
[[nodiscard]] constexpr decltype(auto)
extract_for_values(MetadataOrReflectorT md = { }, type_sequence<PredArgsT...> = { })
{
    static_assert(Occurrence >= 0);
    return detail::extract_value_metadata<T, V, detail::predicate_adaptor<PredT>::template type, Occurrence, type_sequence<PredArgsT...>>(detail::unwrap_metadata<T>(md));
}

template <typename T, typename V, template <typename...> class PredT, typename... PredArgsT, typename MetadataOrReflectorT = reflector>
[[nodiscard]] constexpr decltype(auto)
extract_for_members(MetadataOrReflectorT md = { }, type_sequence<PredArgsT...> = { })
{
    return detail::extract_member_metadata<T, V, detail::predicate_adaptor<PredT>::template type, 0, type_sequence<PredArgsT...>>(detail::unwrap_metadata<T>(md));
}
template <typename T, typename V, int Occurrence, template <typename...> class PredT, typename... PredArgsT, typename MetadataOrReflectorT = reflector>
[[nodiscard]] constexpr decltype(auto)
extract_for_members(MetadataOrReflectorT md = { }, type_sequence<PredArgsT...> = { })
{
    static_assert(Occurrence >= 0);
    return detail::extract_member_metadata<T, V, detail::predicate_adaptor<PredT>::template type, Occurrence, type_sequence<PredArgsT...>>(detail::unwrap_metadata<T>(md));
}


template <typename T>
constexpr bool is_available_v = !std::is_same_v<T, std::nullopt_t>;


template <typename T, typename MetadataOrReflectorT = reflector>
[[nodiscard]] constexpr auto
name(MetadataOrReflectorT md = { })
{
    return metadata::extract<T, std::is_convertible, std::string_view>(md);
}

template <typename T, typename MetadataOrReflectorT = reflector>
[[nodiscard]] constexpr auto
description(MetadataOrReflectorT md = { })
{
    return metadata::extract<T, 1, std::is_convertible, std::string_view>(md);
}

template <typename T, typename MetadataOrReflectorT = reflector>
[[nodiscard]] constexpr auto
bases(MetadataOrReflectorT md = { })
{
    return detail::extract_bases<T>(detail::unwrap_metadata<T>(md));
}

template <typename T, typename MetadataOrReflectorT = reflector>
[[nodiscard]] constexpr auto
values(MetadataOrReflectorT md = { })
{
    return detail::extract_values<T>(detail::unwrap_metadata<T>(md));
}

template <typename T, typename MetadataOrReflectorT = reflector>
[[nodiscard]] constexpr auto
value_names(MetadataOrReflectorT md = { })
{
    return metadata::extract_for_values<T, std::string_view, std::is_convertible, std::string_view>(md);
}

template <typename T, typename MetadataOrReflectorT = reflector>
[[nodiscard]] constexpr auto
value_descriptions(MetadataOrReflectorT md = { })
{
    return metadata::extract_for_values<T, std::string_view, 1, std::is_convertible, std::string_view>(md);
}

template <typename T, typename MetadataOrReflectorT = reflector>
[[nodiscard]] constexpr auto
members(MetadataOrReflectorT md = { })
{
    return detail::extract_members<T>(detail::unwrap_metadata<T>(md));
}

template <typename T, typename MetadataOrReflectorT = reflector>
[[nodiscard]] constexpr auto
member_names(MetadataOrReflectorT md = { })
{
    return metadata::extract_for_members<T, std::string_view, std::is_convertible, std::string_view>(md);
}

template <typename T, typename MetadataOrReflectorT = reflector>
[[nodiscard]] constexpr auto
member_descriptions(MetadataOrReflectorT md = { })
{
    return metadata::extract_for_members<T, std::string_view, 1, std::is_convertible, std::string_view>(md);
}


template <typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr gsl::index
search_value_index(T value, ReflectorT = { })
{
    static_assert(metadata::is_available(detail::value_store<T, ReflectorT>::value));
    return detail::search_index(value, detail::value_store<T, ReflectorT>::value);
}
template <typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr gsl::index
find_value_index(T value, ReflectorT = { })
{
    gsl::index i = metadata::search_value_index<T, ReflectorT>(value);
    if (i >= 0) return i;
    gsl_FailFast();
}


} // namespace metadata

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_METADATA_HPP_
