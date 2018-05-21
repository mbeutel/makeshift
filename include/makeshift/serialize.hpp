
#ifndef MAKESHIFT_SERIALIZE_HPP_
#define MAKESHIFT_SERIALIZE_HPP_


#include <string>
#include <string_view>
#include <array>
#include <type_traits> // for decay<>, is_same<>
#include <utility>     // for move(), forward<>()
#include <iosfwd>      // for istream, ostream
#include <cstddef>     // for size_t
#include <cstdint>     // for uint64_t

#include <makeshift/metadata.hpp>
#include <makeshift/types.hpp>    // for tag<>

#include <makeshift/detail/cfg.hpp> // for MAKESHIFT_SYS_DLLFUNC

#include <gsl/span>



namespace makeshift
{

namespace detail
{

struct enum_value_stringdata
{
    std::uint64_t value;
    std::string_view string;

    constexpr enum_value_stringdata(void) noexcept
        : value(0), string{ }
    {
    }
    constexpr enum_value_stringdata(std::uint64_t _value, std::string_view _string) noexcept
        : value(_value), string(_string)
    {
    }
};

MAKESHIFT_SYS_DLLFUNC std::string enum_to_string(gsl::span<const enum_value_stringdata> knownValues,
    std::uint64_t enumValue);
MAKESHIFT_SYS_DLLFUNC std::string flags_enum_to_string(gsl::span<const enum_value_stringdata> knownValues,
    std::uint64_t enumValue);
MAKESHIFT_SYS_DLLFUNC bool try_string_to_enum(std::uint64_t& enumValue,
    gsl::span<const enum_value_stringdata> knownValues,
    const std::string& string) noexcept;
MAKESHIFT_SYS_DLLFUNC std::uint64_t string_to_enum(
    gsl::span<const enum_value_stringdata> knownValues, std::string_view typeName, std::string_view typeDesc,
    const std::string& string);
MAKESHIFT_SYS_DLLFUNC bool try_string_to_flags_enum(std::uint64_t& enumValue,
    gsl::span<const enum_value_stringdata> knownValues,
    const std::string& string) noexcept;
MAKESHIFT_SYS_DLLFUNC std::uint64_t string_to_flags_enum(
    gsl::span<const enum_value_stringdata> knownValues, std::string_view typeName, std::string_view typeDesc,
    const std::string& string);

template <typename ValC, typename... AttributesT>
    constexpr enum_value_stringdata make_enum_value_stringdata(const value_metadata<ValC, std::tuple<AttributesT...>>& valueMetadata)
{
    static_assert(sizeof(ValC::value) <= sizeof(std::uint64_t), "enums with an underlying type of more than 64 bits are not supported");

    std::string_view name;
    tuple_foreach(valueMetadata.attributes, overload(
        [&](std::string_view s) { name = s; },
        otherwise(ignore)
    ));
    return { std::uint64_t(ValC::value), name };
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
    return enum_to_string(stringdata.values, std::uint64_t(value));
}
template <std::size_t N, typename EnumT>
    std::string to_string(const enum_stringdata<N, true>& stringdata, EnumT value)
{
    return flags_enum_to_string(stringdata.values, std::uint64_t(value));
}
template <std::size_t N, typename EnumT>
    EnumT from_string(tag<EnumT>, const enum_stringdata<N, false>& stringdata, const std::string& string)
{
    return EnumT(string_to_enum(stringdata.values, stringdata.typeName, stringdata.typeDesc, string));
}
template <std::size_t N, typename EnumT>
    EnumT from_string(tag<EnumT>, const enum_stringdata<N, true>& stringdata, const std::string& string)
{
    return EnumT(string_to_flags_enum(stringdata.values, stringdata.typeName, stringdata.typeDesc, string));
}

template <typename EnumT, std::size_t N, bool IsFlagsEnum, typename AttributesT>
    constexpr enum_stringdata<N, IsFlagsEnum> make_enum_stringdata_impl(const type_metadata<EnumT, AttributesT>& enumMetadata)
{
    std::array<enum_value_stringdata, N> values { };
    std::size_t index = 0;
    std::string_view typeName;
    std::string_view typeDesc;
    tuple_foreach(enumMetadata.attributes, overload(
        [&](std::string_view s) { typeName = s; },
        [&](description_t desc) { typeDesc = desc.value; },
        match_template<value_metadata>([&](const auto& val) { values.at(index++) = make_enum_value_stringdata(val); }),
        otherwise(ignore)
    ));
    return { values, typeName, typeDesc };
}

template <typename T, typename... AttributesT>
    constexpr auto make_stringdata(const type_metadata<T, std::tuple<AttributesT...>>& typeMetadata)
{
    if constexpr (std::is_enum<T>::value)
    {
        constexpr bool isFlagsEnum = (std::is_same<AttributesT, flags_t>::value || ...);
        constexpr std::size_t numValues = ((is_template<AttributesT, value_metadata> ? 1 : 0) + ... + 0);
        return make_enum_stringdata_impl<T, numValues, isFlagsEnum>(typeMetadata);
    }
    else
    {
        static_assert(sizeof(T) == ~0, "unsupported type");
    }
}

template <typename T>
    /*constexpr*/ inline auto stringdata { make_stringdata(metadata_of<T>) };
    
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
        value.value_ = from_string(tag<T>{ }, stringdata<T>, str);
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
template <typename T>
    std::string to_string(const T& value)
{
    return to_string(makeshift::detail::stringdata<T>, value);
}
template <typename T>
    T from_string(const std::string& string)
{
    return from_string(tag<T>{ }, makeshift::detail::stringdata<T>, string);
}

} // inline namespace serialize

} // namespace makeshift

#endif // MAKESHIFT_SERIALIZE_HPP_
