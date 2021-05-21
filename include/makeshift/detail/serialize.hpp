
#ifndef INCLUDED_MAKESHIFT_DETAIL_SERIALIZE_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_SERIALIZE_HPP_


#include <gsl-lite/gsl-lite.hpp>  // for gsl_CPP17_OR_GREATER, gsl_Assert(), gsl_FailFast()

#if !gsl_CPP17_OR_GREATER
# error Header <makeshift/iomanip.hpp> requires C++17 mode or higher.
#endif // !gsl_CPP17_OR_GREATER

#include <array>
#include <string>
#include <cstddef>      // for size_t
#include <climits>      // for CHAR_BIT
#include <optional>
#include <stdexcept>    // for runtime_error
#include <type_traits>
#include <string_view>

#include <makeshift/metadata.hpp>


namespace makeshift {

namespace gsl = gsl_lite;

namespace detail {


class char_bitset
{
    static constexpr std::size_t size_t_bits = sizeof(std::size_t)*CHAR_BIT;

private:
    std::size_t data_[((1 << CHAR_BIT) + size_t_bits - 1)/size_t_bits];

public:
    explicit constexpr char_bitset(std::string_view chars) noexcept
        : data_{ }
    {
        for (char ch : chars)
        {
            auto uc = static_cast<unsigned char>(ch);
            data_[uc / size_t_bits] |= std::size_t(1) << (uc % size_t_bits);
        }
    }
    constexpr bool contains(char ch) const noexcept
    {
        auto uc = static_cast<unsigned char>(ch);
        return (data_[uc / size_t_bits] & (std::size_t(1) << (uc % size_t_bits))) != 0;
    }
};

constexpr inline std::string_view enum_forbidden_chars = "+| \t\n\r,[]{}():/\\";
static auto enum_forbidden_char_set = char_bitset(enum_forbidden_chars);

constexpr inline std::string_view whitespace_chars = " \t\n\r";

constexpr std::string_view
trim(std::string_view str) noexcept
{
    std::size_t first = str.find_first_not_of(whitespace_chars);
    if (first == std::string_view::npos)
    {
        return { };
    }
    std::size_t last = str.find_last_not_of(whitespace_chars);
    return str.substr(first, last - first + 1);
}


template <typename T, std::size_t N>
struct enum_metadata
{
    std::string_view description_;
    std::array<T, N> values_;
    std::array<std::string_view, N> names_;
};

template <typename MetadataT>
constexpr std::string_view
description_or_name_or_empty(MetadataT const& md)
{
    auto name = metadata::name(md);
    auto desc = metadata::description(md);
    if constexpr (metadata::is_available_v<decltype(desc)>) return desc;
    else if constexpr (metadata::is_available(name)) return name;
    else return { };
}

template <typename T, typename MetadataT>
constexpr auto
make_enum_metadata(MetadataT const& md)
{
    auto const& values = metadata::values<T>(md);
    auto const& value_names = metadata::value_names<T>(md);
    gsl_Expects(metadata::is_available(values) && metadata::is_available(value_names));

    for (std::string_view name : value_names)
    {
        gsl_Expects(!name.empty());
        gsl_Expects(name.find_first_of(enum_forbidden_chars) == std::string_view::npos);
    }

    auto desc = detail::description_or_name_or_empty(md);

    constexpr std::size_t N = std::tuple_size_v<std::decay_t<decltype(values)>>;
    return enum_metadata<T, N>{ desc, values, value_names };
}

template <typename T, typename MetadataC>
struct static_enum_metadata
{
    static constexpr inline auto value = detail::make_enum_metadata<T>(MetadataC{ }());
};

template <typename T, std::size_t N>
constexpr std::string_view
enum_to_string(T value, enum_metadata<T, N> const& md)
{
    for (std::size_t i = 0; i != N; ++i)
    {
        if (value == md.values_[i])
        {
            return md.names_[i];
        }
    }
    gsl_FailFast();
}
template <typename T, std::size_t N>
constexpr int
try_enum_from_string(T& value, std::string_view str, enum_metadata<T, N> const& md)
{
    for (std::size_t i = 0; i != N; ++i)
    {
        if (str == md.names_[i])
        {
            value = md.values_[i];
            return 0;
        }
    }
    return -1;
}
template <typename T, std::size_t N>
std::string
enum_from_string_error(std::string_view str, enum_metadata<T, N> const& md)
{
    std::string msg;
    if (!md.description_.empty())
    {
        msg += md.description_;
        msg += ": ";
    }
    msg += "unknown value '";
    msg += str;
    msg += "'; supported values: { ";
    bool first = true;
    for (std::string_view name : md.names_)
    {
        if (!first)
        {
            msg += ", ";
        }
        first = false;
        msg += '\'';
        msg += name;
        msg += '\'';
    }
    msg += " }";
    return msg;
}
template <typename T, std::size_t N>
constexpr T
enum_from_string(std::string_view str, enum_metadata<T, N> const& md)
{
    auto result = T{ };
    if (detail::try_enum_from_string(result, detail::trim(str), md) != 0)
    {
        throw std::runtime_error(detail::enum_from_string_error(str, md));
    }
    return result;
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_SERIALIZE_HPP_
