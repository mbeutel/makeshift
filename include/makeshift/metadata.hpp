
#ifndef INCLUDED_MAKESHIFT_METADATA_HPP_
#define INCLUDED_MAKESHIFT_METADATA_HPP_


#include <array>
#include <tuple>
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
template <typename T, typename ReflectorT = reflector> constexpr auto metadata_v = ReflectorT{ }(gsl::type_identity<std::remove_cv_t<T>>{ });
template <typename T, typename ReflectorT = reflector> using metadata_t = std::remove_const_t<decltype(metadata_v<T, ReflectorT>)>;


namespace metadata {


template <typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr std::negation<std::is_same<metadata_t<T, ReflectorT>, std::nullopt_t>>
is_available_for(ReflectorT = { })
{
    return { };
}
template <typename T>
[[nodiscard]] constexpr std::negation<std::is_same<T, std::nullopt_t>>
is_available(T const&)
{
    return { };
}

template <typename T, typename ReflectorT = reflector>
constexpr bool is_available_for_v = !std::is_same_v<metadata_t<T, ReflectorT>, std::nullopt_t>;
template <typename T>
constexpr bool is_available_v = !std::is_same_v<T, std::nullopt_t>;


template <typename T, template <typename...> class PredT, typename... PredArgsT, typename ReflectorT = reflector>
[[nodiscard]] constexpr auto
extract(ReflectorT = { }, type_sequence<PredArgsT...> = { })
{
    return detail::extract_metadata<void, detail::predicate_adaptor<PredT>::template type, 0>(detail::get_metadata<T, ReflectorT>(), type_sequence<PredArgsT...>{ });
}
template <typename T, int Occurrence, template <typename...> class PredT, typename... PredArgsT, typename ReflectorT = reflector>
[[nodiscard]] constexpr auto
extract(ReflectorT = { }, type_sequence<PredArgsT...> = { })
{
    static_assert(Occurrence >= 0);
    return detail::extract_metadata<void, detail::predicate_adaptor<PredT>::template type, Occurrence>(detail::get_metadata<T, ReflectorT>(), type_sequence<PredArgsT...>{ });
}
template <typename T, typename V, typename ReflectorT = reflector>
[[nodiscard]] constexpr V
extract(ReflectorT = { })
{
    return detail::extract_metadata<void, detail::predicate_adaptor<std::is_same>::template type, 0>(detail::get_metadata<T, ReflectorT>(), type_sequence<V>{ });
}
template <typename T, int Occurrence, typename V, typename ReflectorT = reflector>
[[nodiscard]] constexpr V
extract(ReflectorT = { })
{
    static_assert(Occurrence >= 0);
    return detail::extract_metadata<void, detail::predicate_adaptor<std::is_same>::template type, Occurrence>(detail::get_metadata<T, ReflectorT>(), type_sequence<V>{ });
}

template <typename T, typename V, template <typename...> class PredT, typename... PredArgsT, typename ReflectorT = reflector>
[[nodiscard]] constexpr decltype(auto)
extract_for_values(ReflectorT = { }, type_sequence<PredArgsT...> = { })
{
    return detail::extract_value_metadata<std::remove_const_t<T>, V, detail::predicate_adaptor<PredT>::template type, 0, type_sequence<PredArgsT...>>(detail::get_metadata<T, ReflectorT>());
}
template <typename T, typename V, int Occurrence, template <typename...> class PredT, typename... PredArgsT, typename ReflectorT = reflector>
[[nodiscard]] constexpr decltype(auto)
extract_for_values(ReflectorT = { }, type_sequence<PredArgsT...> = { })
{
    static_assert(Occurrence >= 0);
    return detail::extract_value_metadata<std::remove_const_t<T>, V, detail::predicate_adaptor<PredT>::template type, Occurrence, type_sequence<PredArgsT...>>(detail::get_metadata<T, ReflectorT>());
}
template <typename T, typename V, typename ReflectorT = reflector>
[[nodiscard]] constexpr decltype(auto)
extract_for_values(ReflectorT = { })
{
    return detail::extract_value_metadata<std::remove_const_t<T>, V, detail::predicate_adaptor<std::is_same>::template type, 0, type_sequence<V>>(detail::get_metadata<T, ReflectorT>());
}
template <typename T, typename V, int Occurrence, typename ReflectorT = reflector>
[[nodiscard]] constexpr decltype(auto)
extract_for_values(ReflectorT = { })
{
    static_assert(Occurrence >= 0);
    return detail::extract_value_metadata<std::remove_const_t<T>, V, detail::predicate_adaptor<std::is_same>::template type, Occurrence, type_sequence<V>>(detail::get_metadata<T, ReflectorT>());
}

template <typename T, typename V, template <typename...> class PredT, typename... PredArgsT, typename ReflectorT = reflector>
[[nodiscard]] constexpr decltype(auto)
extract_for_exclusive_members(ReflectorT = { }, type_sequence<PredArgsT...> = { })
{
    return detail::extract_member_metadata<std::remove_const_t<T>, V, detail::predicate_adaptor<PredT>::template type, 0, type_sequence<PredArgsT...>>(detail::get_metadata<T, ReflectorT>());
}
template <typename T, typename V, int Occurrence, template <typename...> class PredT, typename... PredArgsT, typename ReflectorT = reflector>
[[nodiscard]] constexpr decltype(auto)
extract_for_exclusive_members(ReflectorT = { }, type_sequence<PredArgsT...> = { })
{
    static_assert(Occurrence >= 0);
    return detail::extract_member_metadata<std::remove_const_t<T>, V, detail::predicate_adaptor<PredT>::template type, Occurrence, type_sequence<PredArgsT...>>(detail::get_metadata<T, ReflectorT>());
}
template <typename T, typename V, typename ReflectorT = reflector>
[[nodiscard]] constexpr decltype(auto)
extract_for_exclusive_members(ReflectorT = { })
{
    return detail::extract_member_metadata<std::remove_const_t<T>, V, detail::predicate_adaptor<std::is_same>::template type, 0, type_sequence<V>>(detail::get_metadata<T, ReflectorT>());
}
template <typename T, typename V, int Occurrence, typename ReflectorT = reflector>
[[nodiscard]] constexpr decltype(auto)
extract_for_exclusive_members(ReflectorT = { })
{
    static_assert(Occurrence >= 0);
    return detail::extract_member_metadata<std::remove_const_t<T>, V, detail::predicate_adaptor<std::is_same>::template type, Occurrence, type_sequence<V>>(detail::get_metadata<T, ReflectorT>());
}

template <typename T, typename V, template <typename...> class PredT, typename... PredArgsT, typename ReflectorT = reflector>
[[nodiscard]] constexpr decltype(auto)
extract_for_members(ReflectorT = { }, type_sequence<PredArgsT...> = { })
{
    return detail::extract_all_member_metadata<std::remove_const_t<T>, V, detail::predicate_adaptor<PredT>::template type, 0, type_sequence<PredArgsT...>, ReflectorT>();
}
template <typename T, typename V, int Occurrence, template <typename...> class PredT, typename... PredArgsT, typename ReflectorT = reflector>
[[nodiscard]] constexpr decltype(auto)
extract_for_members(ReflectorT = { }, type_sequence<PredArgsT...> = { })
{
    static_assert(Occurrence >= 0);
    return detail::extract_all_member_metadata<std::remove_const_t<T>, V, detail::predicate_adaptor<PredT>::template type, Occurrence, type_sequence<PredArgsT...>, ReflectorT>();
}
template <typename T, typename V, typename ReflectorT = reflector>
[[nodiscard]] constexpr decltype(auto)
extract_for_members(ReflectorT = { })
{
    return detail::extract_all_member_metadata<std::remove_const_t<T>, V, detail::predicate_adaptor<std::is_same>::template type, 0, type_sequence<V>, ReflectorT>();
}
template <typename T, typename V, int Occurrence, typename ReflectorT = reflector>
[[nodiscard]] constexpr decltype(auto)
extract_for_members(ReflectorT = { })
{
    static_assert(Occurrence >= 0);
    return detail::extract_all_member_metadata<std::remove_const_t<T>, V, detail::predicate_adaptor<std::is_same>::template type, Occurrence, type_sequence<V>, ReflectorT>();
}


template <typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr auto
name(ReflectorT = { })
{
    return metadata::extract<T, std::is_convertible, std::string_view>(ReflectorT{ });
}

template <typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr auto
description(ReflectorT = { })
{
    return metadata::extract<T, 1, std::is_convertible, std::string_view>(ReflectorT{ });
}

template <typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr auto
bases(ReflectorT = { })
{
    return detail::extract_bases<std::remove_const_t<T>>(detail::get_metadata<T, ReflectorT>());
}

template <typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr decltype(auto)
values(ReflectorT = { })
{
    //return detail::extract_values<std::remove_const_t<T>>(detail::get_metadata<T, ReflectorT>());
    return detail::value_store<std::remove_const_t<T>, ReflectorT>::value;
}

template <typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr decltype(auto)
value_names(ReflectorT = { })
{
    //return metadata::extract_for_values<T, std::string_view, std::is_convertible, std::string_view>(ReflectorT{ });
    return detail::value_name_store<std::remove_const_t<T>, ReflectorT>::value;
}

template <typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr auto
value_descriptions(ReflectorT = { })
{
    return metadata::extract_for_values<T, std::string_view, 1, std::is_convertible, std::string_view>(ReflectorT{ });
}

template <typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr decltype(auto)
exclusive_members(ReflectorT = { })
{
    //return detail::extract_members<std::remove_const_t<T>>(detail::get_metadata<T, ReflectorT>());
    return detail::exclusive_member_store<std::remove_const_t<T>, ReflectorT>::value;
}

template <typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr decltype(auto)
exclusive_member_names(ReflectorT = { })
{
    //return metadata::extract_for_exclusive_members<T, std::string_view, std::is_convertible, std::string_view>(ReflectorT{ });
    return detail::exclusive_member_name_store<std::remove_const_t<T>, ReflectorT>::value;
}

template <typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr auto
exclusive_member_descriptions(ReflectorT = { })
{
    return metadata::extract_for_exclusive_members<T, std::string_view, 1, std::is_convertible, std::string_view>(ReflectorT{ });
}

template <template <typename...> class TupleT, typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr decltype(auto)
members(ReflectorT = { })
{
    //return detail::all_members<std::remove_const_t<T>, TupleT, ReflectorT>();
    return detail::member_store<TupleT, std::remove_const_t<T>, ReflectorT>::value;
}
template <typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr decltype(auto)
members(ReflectorT = { })
{
    //return detail::all_members<std::remove_const_t<T>, std::tuple, ReflectorT>();
    return detail::member_store<std::tuple, std::remove_const_t<T>, ReflectorT>::value;
}

template <typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr auto
member_names(ReflectorT = { })
{
    //return metadata::extract_for_members<T, std::string_view, std::is_convertible, std::string_view>(ReflectorT{ });
    return detail::member_name_store<std::remove_const_t<T>, ReflectorT>::value;
}

template <typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr auto
member_descriptions(ReflectorT = { })
{
    return metadata::extract_for_members<T, std::string_view, 1, std::is_convertible, std::string_view>(ReflectorT{ });
}


template <typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr gsl::index
search_value_index(T value, ReflectorT = { })
{
    constexpr auto const& values = detail::value_store<std::remove_const_t<T>, ReflectorT>::value;
    static_assert(metadata::is_available(values));
    return detail::search_index(value, values);
}
template <typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr gsl::index
find_value_index(T value, ReflectorT = { })
{
    gsl::index i = metadata::search_value_index<T, ReflectorT>(value);
    if (i >= 0) return i;
    gsl_FailFast();
}

template <typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr gsl::index
search_value_index_by_name(std::string_view name, ReflectorT = { })
{
    constexpr auto const& names = detail::value_name_store<std::remove_const_t<T>, ReflectorT>::value;
    static_assert(metadata::is_available(names));
    return detail::search_index(name, names);
}
template <typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr gsl::index
find_value_index_by_name(std::string_view name, ReflectorT = { })
{
    gsl::index i = metadata::search_value_index_by_name<T, ReflectorT>(name);
    if (i >= 0) return i;
    gsl_FailFast();
}

template <typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr gsl::index
search_exclusive_member_index_by_name(std::string_view name, ReflectorT = { })
{
    constexpr auto const& names = detail::exclusive_member_name_store<std::remove_const_t<T>, ReflectorT>::value;
    static_assert(metadata::is_available(names));
    return detail::search_index(name, names);
}
template <typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr gsl::index
find_exclusive_member_index_by_name(std::string_view name, ReflectorT = { })
{
    gsl::index i = metadata::search_exclusive_member_index_by_name<T, ReflectorT>(name);
    if (i >= 0) return i;
    gsl_FailFast();
}

template <typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr gsl::index
search_member_index_by_name(std::string_view name, ReflectorT = { })
{
    constexpr auto const& names = detail::member_name_store<std::remove_const_t<T>, ReflectorT>::value;
    static_assert(metadata::is_available(names));
    return detail::search_index(name, names);
}
template <typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr gsl::index
find_member_index_by_name(std::string_view name, ReflectorT = { })
{
    gsl::index i = metadata::search_member_index_by_name<T, ReflectorT>(name);
    if (i >= 0) return i;
    gsl_FailFast();
}


template <typename T, typename ReflectorT = reflector, typename NameC>
[[nodiscard]] constexpr auto
search_exclusive_member_by_name(NameC nameC, ReflectorT = { })
{
    constexpr auto const& members = detail::exclusive_member_store<std::remove_const_t<T>, ReflectorT>::value;
    static_assert(metadata::is_available(members));
    constexpr auto maybeIndex = metadata::search_exclusive_member_index_by_name<T, ReflectorT>(nameC());
    if constexpr (maybeIndex >= 0)
    {
        using std::get;
        return get<maybeIndex>(members);
    }
    else return std::nullopt;
}
template <typename T, typename ReflectorT = reflector, typename NameC>
[[nodiscard]] constexpr auto
find_exclusive_member_by_name(NameC nameC, ReflectorT = { })
{
    auto member = metadata::search_exclusive_member_by_name<T, ReflectorT>(nameC);
    static_assert(metadata::is_available(member));
    return member;
}

template <typename T, typename ReflectorT = reflector, typename NameC>
[[nodiscard]] constexpr auto
search_member_by_name(NameC nameC, ReflectorT = { })
{
    constexpr auto const& members = detail::member_store<std::tuple, std::remove_const_t<T>, ReflectorT>::value;
    static_assert(metadata::is_available(members));
    constexpr auto maybeIndex = metadata::search_member_index_by_name<T, ReflectorT>(nameC());
    if constexpr (maybeIndex >= 0)
    {
        using std::get;
        return get<maybeIndex>(members);
    }
    else return std::nullopt;
}
template <typename T, typename ReflectorT = reflector, typename NameC>
[[nodiscard]] constexpr auto
find_member_by_name(NameC nameC, ReflectorT = { })
{
    auto member = metadata::search_member_by_name<T, ReflectorT>(nameC);
    static_assert(metadata::is_available(member));
    return member;
}


} // namespace metadata

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_METADATA_HPP_
