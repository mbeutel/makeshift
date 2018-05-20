
#ifndef MAKESHIFT_SERIALIZE_HPP_
#define MAKESHIFT_SERIALIZE_HPP_


#include <string_view>
#include <stdexcept>
#include <array>
#include <type_traits> // for decay_t<>
#include <utility> // for forward<>()

#include <iosfwd>

#include <cstddef> // for size_t
#include <cstdint> // for uint64_t

#include "metadata.hpp" // TODO: ?
#include "types.hpp" // TODO: ?

#include <gsl/span>


namespace makeshift
{

namespace detail
{

[[noreturn]] void raiseInvalidValueError(void)
{
    throw std::logic_error("invalid value");
}
[[noreturn]] void raiseInvalidStringError(const std::string& str, std::string_view typeDesc = { }, std::string_view typeName = { })
{
    std::string msg;
    if (!typeDesc.empty())
        msg = "'" + str + "' is not a valid " + std::string(typeDesc);
    else if (!typeName.empty())
        msg = "'" + str + "' is not a valid value of type '" + std::string(typeName) + "'";
    else
        msg = "unrecognized value '" + str + "'";
    throw std::runtime_error(msg); // TODO: use more appropriate exception class
}

struct enum_value_stringdata
{
    std::uint64_t value;
    std::string_view string;

    constexpr enum_value_stringdata(void)
        : value(0), string{ }
    {
    }
};

std::string_view enum_to_string(gsl::span<enum_value_stringdata> knownValues, std::uint64_t enumValue) noexcept
{
}
std::string flags_enum_to_string(gsl::span<enum_value_stringdata> knownValues, std::uint64_t enumValue) noexcept
{
}
bool try_string_to_enum(std::uint64_t& enumValue,
    gsl::span<enum_value_stringdata> knownValues,
    const std::string& string) noexcept
{
}
std::uint64_t string_to_enum(
    gsl::span<enum_value_stringdata> knownValues, std::string_view typeName, std::string_view typeDesc,
    const std::string& string)
{
    std::uint64_t enumValue;
    if (!try_string_to_enum(enumValue, knownValues, string))
        raiseInvalidStringError(string);
    return enumValue;
}
bool try_string_to_flags_enum(std::uint64_t& enumValue,
    gsl::span<enum_value_stringdata> knownValues,
    const std::string& string) noexcept
{
}
std::uint64_t string_to_flags_enum(
    gsl::span<enum_value_stringdata> knownValues, std::string_view typeName, std::string_view typeDesc,
    const std::string& string)
{
    std::uint64_t enumValue;
    if (!try_string_to_flags_enum(enumValue, knownValues, string))
        raiseInvalidStringError(string);
    return enumValue;
}

template <auto EnumVal, typename... AttributesT>
    constexpr enum_value_stringdata make_enum_value_stringdata(const value_metadata<EnumVal, std::tuple<AttributesT...>>& valueMetadata)
{
    static_assert(sizeof(EnumVal) <= sizeof(std::uint64_t), "enums with an underlying type of more than 64 bits are not supported");

    std::string_view name;
    tuple_foreach(valueMetadata.attributes, overload(
        [&](std::string_view s) { name = s; },
        otherwise(ignore)
    ));
    return { std::uint64_t(EnumVal), name };
}

template <std::size_t N, bool IsFlagsEnum>
    struct enum_stringdata
{
    static constexpr bool isFlagsEnum = IsFlagsEnum;

    std::array<enum_value_stringdata, N> values;
    std::string_view typeName;
    std::string_view typeDesc;
};
template <std::size_t N, typename EnumT>
    std::string to_string(const enum_stringdata<N, false>& stringdata, EnumT value)
{
    return enum_to_string(stringdata.values, value);
}
template <std::size_t N, typename EnumT>
    std::string to_string(const enum_stringdata<N, true>& stringdata, EnumT value)
{
    return flags_enum_to_string(stringdata.values, value);
}
template <std::size_t N, typename EnumT>
    EnumT from_string(const enum_stringdata<N, false>& stringdata, const std::string& string)
{
    return string_to_enum(stringdata.values, stringdata.typeName, stringdata.typeDesc, string);
}
template <std::size_t N, typename EnumT>
    EnumT from_string(const enum_stringdata<N, true>& stringdata, const std::string& string)
{
    return string_to_flags_enum(stringdata.values, stringdata.typeName, stringdata.typeDesc, string);
}

template <typename EnumT, std::size_t N, bool IsFlagsEnum, typename... AttributesT>
    constexpr enum_stringdata<N, IsFlagsEnum> make_enum_stringdata_impl(const makeshift::type_metadata<EnumT, std::tuple<AttributesT...>>& enumMetadata)
{
    std::array<enum_value_stringdata, N> values { };
    std::string_view typeName;
    std::string_view typeDesc;
    std::size_t index = 0;
    tuple_foreach(enumMetadata.attributes, overload(
        [&](std::string_view s) { typeName = s; },
        [&](description_t desc) { typeDesc = desc.value; },
        [&](const auto& val) -> std::enable_if_t<is_value_metadata<decltype(val)>>
        {
            values.at(index++) = make_enum_value_stringdata<decltype(val)::value>(val);
        }
    ));
    return { values, typeName, typeDesc };
}

template <typename T, typename... AttributesT>
    constexpr auto make_stringdata(const type_metadata<T, std::tuple<AttributesT...>>& typeMetadata)
{
    if constexpr (std::is_enum<T>::value)
    {
        constexpr bool isFlagsEnum = (std::is_same<AttributesT, flags_t>::value || ...);
        constexpr std::size_t numValues = ((is_value_metadata<AttributesT> ? 1 : 0) + ... + 0);
        return make_enum_stringdata_impl<T, numValues, isFlagsEnum>(typeMetadata);
    }
    else
    {
        static_assert(sizeof(T) == ~0, "unsupported type");
    }
}

template <typename T>
    constexpr inline auto stringdata { make_stringdata(metadata_of<T>) };
    
template <typename T>
    struct rvalue_as_string
{
    rvalue_as_string(const rvalue_as_string&) = delete;
    rvalue_as_string(rvalue_as_string&&) = delete;
    rvalue_as_string& operator =(const rvalue_as_string&) = delete;
    rvalue_as_string& operator =(rvalue_as_string&&) = delete;

private:
    const T& value_;

public:
    constexpr rvalue_as_string(const T& _value) noexcept : value_(_value) { }

    friend std::ostream& operator <<(std::ostream& stream, const rvalue_as_string& value)
    {
        return stream << to_string(stringdata<T>, value.value_);
    }
};
template <typename T>
    struct lvalue_as_string
{
    lvalue_as_string(const lvalue_as_string&) = delete;
    lvalue_as_string(lvalue_as_string&&) = delete;
    lvalue_as_string& operator =(const lvalue_as_string&) = delete;
    lvalue_as_string& operator =(lvalue_as_string&&) = delete;

private:
    T& value_;

public:
    constexpr lvalue_as_string(T& _value) noexcept : value_(_value) { }

    friend std::ostream& operator <<(std::ostream& stream, const lvalue_as_string& value)
    {
        return stream << to_string(stringdata<T>, value.value_);
    }
    friend std::istream& operator >>(std::istream& stream, const lvalue_as_string& value)
    {
        std::string str;
        stream >> str;
        value.value_ = from_string(stringdata<T>, str);
        return stream;
    }
};

} // namespace detail


inline namespace serialize
{

template <typename T>
    makeshift::detail::rvalue_as_string<T> as_string(const T& value)
{
    return { value };
}
template <typename T>
    makeshift::detail::lvalue_as_string<T> as_string(T& value)
{
    return { value };
}

} // inline namespace serialize

} // namespace makeshift

#endif // MAKESHIFT_SERIALIZE_HPP_
