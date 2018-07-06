﻿
#ifndef MAKESHIFT_DETAIL_SERIALIZE_ENUM_HPP_
#define MAKESHIFT_DETAIL_SERIALIZE_ENUM_HPP_


#include <string>      // for string, to_string()
#include <string_view>
#include <array>
#include <type_traits> // for decay<>, is_same<>, is_enum<>
#include <utility>     // for move(), forward<>()
#include <cstddef>     // for size_t
#include <cstdint>     // for uint64_t

#include <makeshift/type_traits.hpp> // for tag<>, flags_base
#include <makeshift/metadata.hpp>
#include <makeshift/tuple.hpp>
#include <makeshift/array.hpp>       // for to_array()

#include <makeshift/detail/cfg.hpp>  // for MAKESHIFT_SYS_DLLFUNC

#include <gsl/span>


namespace makeshift
{

namespace detail
{


struct enum_value_serialization_data
{
    std::uint64_t value;
    std::string_view string;

    constexpr enum_value_serialization_data(void) noexcept : value(0), string{ } { }
    constexpr enum_value_serialization_data(std::uint64_t _value, std::string_view _string) noexcept : value(_value), string(_string) { }
};
struct enum_serialization_data_ref
{
    gsl::span<const enum_value_serialization_data> values;
    std::string_view typeName;
    std::string_view typeDesc;
};
struct flags_enum_serialization_data_ref
{
    gsl::span<const enum_value_serialization_data> values;
    std::string_view flagTypeName; // name of the enum type
    std::string_view defTypeName; // name of the struct which defines the constants
    std::string_view typeDesc;
};

MAKESHIFT_SYS_DLLFUNC std::string enum_to_string(std::uint64_t enumValue, const enum_serialization_data_ref& sdata);
MAKESHIFT_SYS_DLLFUNC bool try_string_to_enum(std::uint64_t& enumValue, const std::string& string, const enum_serialization_data_ref& sdata) noexcept;
MAKESHIFT_SYS_DLLFUNC std::uint64_t string_to_enum(const std::string& string, const enum_serialization_data_ref& sdata);

MAKESHIFT_SYS_DLLFUNC std::string flags_enum_to_string(std::uint64_t enumValue, const flags_enum_serialization_data_ref& sdata);
MAKESHIFT_SYS_DLLFUNC bool try_string_to_flags_enum(std::uint64_t& enumValue, const std::string& string, const flags_enum_serialization_data_ref& sdata) noexcept;
MAKESHIFT_SYS_DLLFUNC std::uint64_t string_to_flags_enum(const std::string& string, const flags_enum_serialization_data_ref& sdata);

template <typename ValC, typename... AttributesT>
    constexpr enum_value_serialization_data make_enum_value_serialization_data(const value_metadata<ValC, std::tuple<AttributesT...>>& valueMetadata)
{
    static_assert(sizeof(ValC::value) <= sizeof(std::uint64_t), "enums with an underlying type of more than 64 bits are not supported");

    std::string_view lname = std::get<std::string_view>(valueMetadata.attributes);
    return { std::uint64_t(ValC::value), lname };
}

template <std::size_t N>
    struct enum_serialization_data
{
    std::array<enum_value_serialization_data, N> values;
    std::string_view typeName;
    std::string_view typeDesc;

    constexpr enum_serialization_data_ref data(void) const noexcept { return { values, typeName, typeDesc }; }
};
template <typename T> using is_value_metadata = is_same_template<T, value_metadata>;
template <typename EnumT, typename AttributesT>
    constexpr auto make_enum_serialization_data(const type_metadata<EnumT, AttributesT>& enumMetadata)
{
    std::string_view typeName = get_or_default<std::string_view>(enumMetadata.attributes);
    std::string_view typeDesc = get_or_default<caption_t>(enumMetadata.attributes).value;
    auto values = enumMetadata.attributes
        | tuple_filter<is_value_metadata>()
        | tuple_map([](const auto& v) { return make_enum_value_serialization_data(v); })
        | to_array();
    return enum_serialization_data<array_size_v<decltype(values)>> { values, typeName, typeDesc };
}
template <typename EnumT, std::size_t N>
    std::string to_string(EnumT value, const enum_serialization_data<N>& sdata)
{
    return enum_to_string(std::uint64_t(value), sdata.data());
}
template <typename EnumT, std::size_t N>
    EnumT from_string(tag_t<EnumT>, const std::string& string, const enum_serialization_data<N>& sdata)
{
    return EnumT(string_to_enum(string, sdata.data()));
}

template <std::size_t N>
    struct flags_enum_serialization_data
{
    std::array<enum_value_serialization_data, N> values;
    std::string_view flagTypeName; // name of the enum type
    std::string_view defTypeName; // name of the struct which defines the constants
    std::string_view typeDesc;

    constexpr flags_enum_serialization_data_ref data(void) const noexcept { return { values, flagTypeName, defTypeName, typeDesc }; }
};
template <typename T> using is_flags = is_same_template<T, flags_t>;
template <typename DefT, typename AttributesT>
    constexpr std::string_view get_flag_type_name(const type_metadata<DefT, AttributesT>& enumMetadata)
{
    auto maybeFlags = enumMetadata.attributes
        | tuple_filter<is_flags>()
        | single_or_none();
    if constexpr (!std::is_same<decltype(maybeFlags), none_t>::value)
        return get_or_default<std::string_view>(maybeFlags.value.attributes);
    else
        return { };
}
template <typename DefT, typename AttributesT>
    constexpr auto make_flags_enum_serialization_data(const type_metadata<DefT, AttributesT>& enumMetadata)
{
    std::string_view defTypeName = get_or_default<std::string_view>(enumMetadata.attributes);
    std::string_view typeDesc = get_or_default<caption_t>(enumMetadata.attributes).value;
    std::string_view flagTypeName = get_flag_type_name(enumMetadata);
    auto values = enumMetadata.attributes
        | tuple_filter<is_value_metadata>()
        | tuple_map([](const auto& v) { return make_enum_value_serialization_data(v); })
        | to_array();
    return flags_enum_serialization_data<array_size_v<decltype(values)>>{ values, flagTypeName, defTypeName, typeDesc };
}
template <typename EnumT, std::size_t N>
    std::string to_string(EnumT value, const flags_enum_serialization_data<N>& sdata)
{
    return flags_enum_to_string(std::uint64_t(value), sdata.data());
}
template <typename EnumT, std::size_t N>
    EnumT from_string(tag_t<EnumT>, const std::string& string, const flags_enum_serialization_data<N>& sdata)
{
    return EnumT(string_to_flags_enum(string, sdata.data()));
}


template <typename T, typename... AttributesT>
    constexpr auto make_serialization_data(const type_metadata<T, std::tuple<AttributesT...>>& typeMetadata)
{
    if constexpr (std::is_enum<T>::value)
        return makeshift::detail::make_enum_serialization_data(typeMetadata);
    else if constexpr (std::is_base_of<makeshift::detail::flags_base, T>::value)
        return makeshift::detail::make_flags_enum_serialization_data(typeMetadata);
    else
        static_assert(sizeof(T) == ~0, "unsupported type");
}
template <typename T, typename MetadataTagT = serialization_metadata_tag>
    constexpr auto serialization_data = make_serialization_data(metadata_of<T, MetadataTagT>);


template <typename MetadataTagT, typename T> struct is_enum_with_metadata : std::conjunction<have_metadata<T, MetadataTagT>, std::is_enum<T>> { };


} // namespace detail

} // namespace makeshift


#endif // MAKESHIFT_DETAIL_SERIALIZE_ENUM_HPP_