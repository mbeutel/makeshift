
#ifndef INCLUDED_MAKESHIFT_DETAIL_METADATA_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_METADATA_HPP_


#include <array>
#include <tuple>
#include <cstddef>      // for size_t
#include <utility>      // for index_sequence<>, tuple_size<>
#include <optional>
#include <string_view>
#include <type_traits>  // for void_t<>

#include <gsl-lite/gsl-lite.hpp>


namespace makeshift {

namespace gsl = gsl_lite;

namespace detail {


struct convertible_from_type_identity
{
    template <typename T>
    constexpr convertible_from_type_identity(gsl::type_identity<T>)
    {
    }
};

constexpr std::nullopt_t
reflect(convertible_from_type_identity)
{
    return std::nullopt;
}
constexpr auto
reflect(gsl::type_identity<bool>)
{
    return std::array{ false, true };  // TODO: extend?
}

template <typename T>
struct reflector
{
    constexpr auto
    operator ()() const
    {
        return reflect(gsl::type_identity<T>{ });
    }
};


template <typename, typename M> struct is_string_like : std::is_convertible<M, std::string_view> { };

template <typename T, typename M> struct is_value_array : std::false_type { };
template <typename T, std::size_t N> struct is_value_array<T, std::array<T, N>> : std::true_type { };

template <typename T, typename M, typename R> struct is_value_record_1_ : std::false_type { };
template <typename T, typename M> struct is_value_record_1_<T, M, T> : std::true_type { };
template <typename T, typename M, bool IsNonEmptyTuple> struct is_value_record_0_;
template <typename T, typename M> struct is_value_record_0_<T, M, true> : is_value_record_1_<T, M, std::tuple_element_t<0, M>> { };
template <typename T, typename M> struct is_value_record_0_<T, M, false> : std::false_type { };
template <typename T, typename M, typename = void> struct is_value_record : std::false_type { };
template <typename T, typename M> struct is_value_record<T, M, std::void_t<typename std::tuple_size<M>::type>> : is_value_record_0_<T, M, (std::tuple_size_v<M> > 0)> { };

template <typename T, typename M> struct is_value_record_array : std::false_type { };
template <typename T, typename M, std::size_t N> struct is_value_record_array<T, std::array<M, N>> : is_value_record<T, M> { };

template <typename T, typename M, template <typename...> class PredT, std::size_t N, std::size_t I, int Occurrence>
struct record_index_0_;
template <typename T, typename M, template <typename...> class PredT, std::size_t N, std::size_t I, int Occurrence, bool Match>
struct record_index_1_;
template <typename T, typename M, template <typename...> class PredT, std::size_t N, std::size_t I>
struct record_index_1_<T, M, PredT, N, I, 0, true> : std::integral_constant<std::size_t, I> { };
template <typename T, typename M, template <typename...> class PredT, std::size_t N, std::size_t I, int Occurrence>
struct record_index_1_<T, M, PredT, N, I, Occurrence, true> : record_index_0_<T, M, PredT, N, I + 1, Occurrence - 1> { };
template <typename T, typename M, template <typename...> class PredT, std::size_t N, std::size_t I, int Occurrence>
struct record_index_1_<T, M, PredT, N, I, Occurrence, false> : record_index_0_<T, M, PredT, N, I + 1, Occurrence> { };
template <typename T, typename M, template <typename...> class PredT, std::size_t N, std::size_t I, int Occurrence>
struct record_index_0_ : record_index_1_<T, M, PredT, N, I, Occurrence, PredT<T, std::tuple_element_t<I, M>>::value> { };
template <typename T, typename M, template <typename...> class PredT, std::size_t N, int Occurrence>
struct record_index_0_<T, M, PredT, N, N, Occurrence> : std::integral_constant<std::size_t, std::size_t(-1)> { };
template <typename T, typename M, template <typename...> class PredT, int Occurrence = 0, typename = void>
struct record_index : std::integral_constant<std::size_t, std::size_t(-1)> { };
template <typename T, typename M, template <typename...> class PredT, int Occurrence>
struct record_index<T, M, PredT, Occurrence, std::void_t<typename std::tuple_size<M>::type>> : record_index_0_<T, M, PredT, std::tuple_size_v<M>, 0, Occurrence> { };

template <typename T>
constexpr decltype(auto)
promote(T const& arg)
{
    if constexpr (std::is_same_v<T, char const*>) return std::string_view(arg);
    else return arg;
}

template <std::size_t I, typename T, std::size_t N, std::size_t... Is>
constexpr auto
extract_column_0(std::array<T, N> const& array, std::index_sequence<Is...>)
{
    using std::get;
    return std::array{ promote(get<I>(array[Is]))... };
}
template <std::size_t I, typename T, std::size_t N>
constexpr auto
extract_column(std::array<T, N> const& array)
{
    return detail::extract_column_0<I>(array, std::make_index_sequence<N>{ });
}

template <std::size_t I>
struct extract_metadata_
{
    template <typename R>
    static constexpr auto
    invoke(R const& arg)
    {
        using std::get;
        return detail::promote(get<I>(arg));
    }
};
template <>
struct extract_metadata_<std::size_t(-1)>
{
    template <typename R>
    static constexpr std::nullopt_t
    invoke(R const&)
    {
        return std::nullopt;
    }
};

template <typename T, template <typename...> class PredT, int Occurrence, typename M>
constexpr decltype(auto)
extract_metadata([[maybe_unused]] M const& metadata)
{
    if constexpr (!std::is_same_v<M, std::nullopt_t>)
    {
        if constexpr (Occurrence == 0 && PredT<T, M>::value)
        {
            return detail::promote(metadata);
        }
        else
        {
            return extract_metadata_<record_index<T, M, PredT, Occurrence>::value>::invoke(metadata);
        }
    }
    else return std::nullopt;
}

template <typename T, typename M>
constexpr decltype(auto)
extract_values([[maybe_unused]] M const& metadata)
{
    if constexpr (!std::is_same_v<std::decay_t<decltype(detail::extract_metadata<T, is_value_record_array, 0>(metadata))>, std::nullopt_t>)
    {
        return detail::extract_column<0>(detail::extract_metadata<T, is_value_record_array, 0>(metadata));
    }
    else if constexpr (!std::is_same_v<std::decay_t<decltype(detail::extract_metadata<T, is_value_array, 0>(metadata))>, std::nullopt_t>)
    {
        return detail::extract_metadata<T, is_value_array, 0>(metadata);
    }
    else return std::nullopt;
}

template <typename T, template <typename...> class PredT, int Occurrence, typename R, std::size_t N>
constexpr decltype(auto)
try_extract_column([[maybe_unused]] std::array<R, N> const& arg)
{
    constexpr std::size_t j = record_index<T, R, PredT, Occurrence>::value;
    if constexpr (j != std::size_t(-1))
    {
        return detail::extract_column<j>(arg);
    }
    else return std::nullopt;
}

template <typename T, template <typename...> class PredT, int Occurrence, typename M>
constexpr decltype(auto)
extract_value_metadata([[maybe_unused]] M const& metadata)
{
    if constexpr (!std::is_same_v<std::decay_t<decltype(detail::extract_metadata<T, is_value_record_array, 0>(metadata))>, std::nullopt_t>)
    {
        return detail::try_extract_column<T, PredT, Occurrence>(detail::extract_metadata<T, is_value_record_array, 0>(metadata));
    }
    else return std::nullopt;
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_METADATA_HPP_
