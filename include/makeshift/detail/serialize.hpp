
#ifndef INCLUDED_MAKESHIFT_DETAIL_SERIALIZE_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_SERIALIZE_HPP_


#include <gsl-lite/gsl-lite.hpp>  // for gsl_Expects(), gsl_Assert(), gsl_FailFast()

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


template <typename T, typename ReflectorT>
constexpr std::string_view
description_or_name_or_empty()
{
    auto name = metadata::name<T, ReflectorT>();
    auto desc = metadata::description<T, ReflectorT>();
    if constexpr (metadata::is_available_v<decltype(desc)>) return desc;  // TODO: could we use `is_available()` here?
    else if constexpr (metadata::is_available(name)) return name;
    else return { };
}


constexpr inline std::string_view enum_forbidden_chars = "+| \t\n\r,[]{}():/\\";
static auto enum_forbidden_char_set = char_bitset(enum_forbidden_chars);

template <typename T, std::size_t N>
struct enum_metadata
{
    std::string_view description_;
    std::array<T, N> values_;
    std::array<std::string_view, N> names_;
};

template <typename T, typename ReflectorT>
constexpr auto
make_enum_metadata()
{
    auto const& values = metadata::values<T, ReflectorT>();
    auto const& value_names = metadata::value_names<T, ReflectorT>();
    gsl_Expects(metadata::is_available(values) && metadata::is_available(value_names));

    for (std::string_view name : value_names)
    {
        gsl_Expects(!name.empty());
        gsl_Expects(name.find_first_of(enum_forbidden_chars) == std::string_view::npos);
    }

    std::string_view desc = detail::description_or_name_or_empty<T, ReflectorT>();

    constexpr std::size_t N = std::tuple_size_v<std::decay_t<decltype(values)>>;
    return enum_metadata<T, N>{ desc, values, value_names };
}

template <typename T, typename ReflectorT>
struct static_enum_metadata
{
    static constexpr inline auto value = detail::make_enum_metadata<T, ReflectorT>();
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
enum_from_string_error(std::string_view str, enum_metadata<T, N> const& md, bool flags = false)
{
    std::string msg;
    if (!md.description_.empty())
    {
        msg += md.description_;
        msg += ": ";
    }
    msg += "unknown value '";
    msg += str;
    msg += "'; supported values: ";
    if (flags)
    {
        msg += "a '+'-delimited subset of ";
    }
    msg += "{ ";
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


constexpr inline std::string_view flags_forbidden_chars = ",[]{}():/\\";
static auto flags_forbidden_char_set = char_bitset(flags_forbidden_chars);

template <typename T>
constexpr bool
is_power_of_2(T value) noexcept
{
    return value > 0 && (value & (value - 1)) == 0;
}

template <typename T, std::size_t N>
struct flags_metadata : enum_metadata<T, N>
{
    T all_defined_flags_;
    std::string_view none_name_;
    std::size_t num_individual_names_;
};

template <typename T, typename ReflectorT>
constexpr auto
make_flags_metadata()
{
    using UU = std::make_unsigned_t<std::underlying_type_t<T>>;

    auto const& values = metadata::values<T, ReflectorT>();
    auto const& value_names = metadata::value_names<T, ReflectorT>();
    gsl_Expects(metadata::is_available(values) && metadata::is_available(value_names));

    for (std::string_view name : value_names)
    {
        gsl_Expects(!name.empty());
        gsl_Expects(name.find_first_of(flags_forbidden_chars) == std::string_view::npos);
    }

    constexpr std::size_t N = std::tuple_size_v<std::decay_t<decltype(values)>>;
    auto flags = std::array<T, N>{ };
    auto names = std::array<std::string_view, N>{ };
    std::size_t j = 0;
    auto allDefinedFlags = T{ };
    auto allIndividuallyDefinedFlags = T{ };
    auto noneName = std::string_view{ };
    for (std::size_t i = 0; i != values.size(); ++i)
    {
        auto flag = values[i];
        auto name = value_names[i];
        allDefinedFlags |= flag;
        if (detail::is_power_of_2(static_cast<UU>(flag)))
        {
            allIndividuallyDefinedFlags |= flag;
            flags[j] = flag;
            names[j] = name;
            ++j;
        }
        else if (flag == T{ } && noneName.empty())
        {
            noneName = name;
        }
    }
    gsl_Assert(allDefinedFlags == allIndividuallyDefinedFlags);
    std::size_t numIndividualNames = j;
    for (std::size_t i = 0; i != values.size(); ++i)
    {
        auto flag = values[i];
        auto name = value_names[i];
        if (!detail::is_power_of_2(static_cast<UU>(flag)))
        {
            flags[j] = flag;
            names[j] = name;
            ++j;
        }
    }

    auto desc = detail::description_or_name_or_empty<T, ReflectorT>();

    return flags_metadata<T, N>{ { desc, flags, names }, allDefinedFlags, noneName, numIndividualNames };
}

template <typename T, typename ReflectorT>
struct static_flags_metadata
{
    static constexpr inline auto value = detail::make_flags_metadata<T, ReflectorT>();
};

template <typename T, std::size_t N>
std::string
flags_to_string(T flags, flags_metadata<T, N> const& md)
{
    gsl_Expects((flags & ~md.all_defined_flags_) == T{ });

    if (flags == T{ })
    {
        return std::string(md.none_name_);
    }
    std::string result;
    auto flagsSet = T{ };
    for (std::size_t i = 0; i != md.num_individual_names_; ++i)
    {
        T flag = md.values_[i];
        if ((flags & flag) != T{ } && (flagsSet & flag) == T{ })
        {
            if (!result.empty())
            {
                result += '+';
            }
            result += md.names_[i];
            flagsSet |= flag;
        }
    }
    return result;
}
template <typename T, std::size_t N>
std::string
enum_from_string_error(std::string_view token, flags_metadata<T, N> const& md)
{
    std::string msg;
    if (!md.description_.empty())
    {
        msg += md.description_;
        msg += ": ";
    }
    msg += "unknown value '";
    msg += token;
    msg += "'; supported values: '+'-delimited list of { ";
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
constexpr int
flags_from_string(T& value, std::string_view str, enum_metadata<T, N> const& md, bool raise = true)
{
    auto result = T{ };
    while (!str.empty())
    {
        std::size_t pos = str.find_first_of("+|");
        std::string_view token;
        if (pos != std::string_view::npos)
        {
            token = detail::trim(str.substr(0, pos));
            str.remove_prefix(pos + 1);
        }
        else
        {
            token = detail::trim(str);
            if (token.empty()) break;
            str = { };
        }
        bool found = false;
        for (std::size_t i = 0; i != N; ++i)
        {
            if (token == md.names_[i])
            {
                result |= md.values_[i];
                found = true;
                break;
            }
        }
        if (!found)
        {
            if (raise)
            {
                throw std::runtime_error(detail::enum_from_string_error(token, md, true));
            }
            else return -1;
        }
    }
    value = result;
    return 0;
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_SERIALIZE_HPP_
