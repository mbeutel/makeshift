
#ifndef INCLUDED_MAKESHIFT_DETAIL_SERIALIZE_ENUM_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_SERIALIZE_ENUM_HPP_


#include <string>      // for string, to_string()
#include <string_view>
#include <array>
#include <type_traits> // for decay<>, is_same<>, is_enum<>
#include <utility>     // for move(), forward<>()
#include <cstddef>     // for size_t
#include <cstdint>     // for uint64_t
#include <tuple>

#include <gsl/span>

#include <makeshift/type_traits.hpp> // for tag<>, flags_base
#include <makeshift/metadata.hpp>
#include <makeshift/tuple.hpp>
#include <makeshift/array.hpp>       // for array_cat<>, array_size<>

#include <makeshift/detail/export.hpp> // for MAKESHIFT_DLLFUNC


namespace makeshift
{

inline namespace serialize
{


struct enum_hint_options; // defined in hint.hpp
struct enum_serialization_options; // defined in serialize.hpp


} // inline namespace serialize


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

MAKESHIFT_DLLFUNC std::string enum_hint(const enum_serialization_data_ref& sdata, const enum_hint_options& options);
MAKESHIFT_DLLFUNC std::string flags_enum_hint(const flags_enum_serialization_data_ref& sdata, const enum_hint_options& options);

MAKESHIFT_DLLFUNC std::string_view enum_to_string(std::uint64_t enumValue, const enum_serialization_data_ref& sdata, const enum_serialization_options& options);
MAKESHIFT_DLLFUNC void enum_from_string(std::string_view string, std::uint64_t& enumValue, const enum_serialization_data_ref& sdata, const enum_serialization_options& options);

MAKESHIFT_DLLFUNC std::string flags_enum_to_string(std::uint64_t enumValue, const flags_enum_serialization_data_ref& sdata, const enum_serialization_options& options);
MAKESHIFT_DLLFUNC void flags_enum_from_string(std::string_view string, std::uint64_t& enumValue, const flags_enum_serialization_data_ref& sdata, const enum_serialization_options& options);


template <typename ValueMetadataT>
    constexpr auto make_enum_value_serialization_data(const ValueMetadataT& valueMetadata)
{
    static_assert(sizeof(ValueMetadataT::value) <= sizeof(std::uint64_t), "enums with an underlying type of more than 64 bits are not supported");

    return valueMetadata.attributes
        | tuple_filter(trait_v<std::is_same, std::string_view>)
        | tuple_map_to<enum_value_serialization_data>([](std::string_view lname) { return enum_value_serialization_data{ std::uint64_t(ValueMetadataT::value), lname }; });
}

template <std::size_t N>
    struct enum_serialization_data
{
    std::array<enum_value_serialization_data, N> values;
    std::string_view typeName;
    std::string_view typeDesc;

    constexpr enum_serialization_data_ref data_ref(void) const noexcept { return { values, typeName, typeDesc }; }
};
template <typename TypeMetadataT>
    constexpr auto make_enum_serialization_data(const TypeMetadataT& enumMetadata)
{
    std::string_view typeName = get_or_default<std::string_view>(enumMetadata.attributes);
    std::string_view typeDesc = get_or_default<caption_metadata>(enumMetadata.attributes).value;
    auto values = enumMetadata.attributes
        | tuple_filter(template_trait_v<is_instantiation_of, value_metadata>)
        | tuple_map([](const auto& v) { return make_enum_value_serialization_data(v); })
        | array_cat<enum_value_serialization_data>();
    return enum_serialization_data<array_size_v<decltype(values)>> { values, typeName, typeDesc };
}
template <typename EnumT, std::size_t N>
    std::string_view to_string(EnumT value, const enum_serialization_data<N>& sdata, const enum_serialization_options& options)
{
    return enum_to_string(std::uint64_t(value), sdata.data_ref(), options);
}
template <typename EnumT, std::size_t N>
    EnumT from_string(tag<EnumT>, std::string_view string, const enum_serialization_data<N>& sdata, const enum_serialization_options& options)
{
    std::uint64_t value;
    enum_from_string(string, value, sdata.data_ref(), options);
    return EnumT(value);
}
template <std::size_t N>
    std::string get_hint(const enum_serialization_data<N>& sdata, const enum_hint_options& options)
{
    return enum_hint(sdata.data_ref(), options);
}

template <std::size_t N>
    struct flags_enum_serialization_data
{
    std::array<enum_value_serialization_data, N> values;
    std::string_view flagTypeName; // name of the enum type
    std::string_view defTypeName; // name of the struct which defines the constants
    std::string_view typeDesc;

    constexpr flags_enum_serialization_data_ref data_ref(void) const noexcept { return { values, flagTypeName, defTypeName, typeDesc }; }
};
template <typename TypeMetadataT>
    constexpr std::string_view get_flag_type_name(const TypeMetadataT& enumMetadata)
{
    auto maybeFlags = enumMetadata.attributes
        | tuple_filter(template_trait_v<is_instantiation_of, flags_metadata>)
        | single_or_none();
    if constexpr (!std::is_same<decltype(maybeFlags), none>::value)
        return get_or_default<std::string_view>(maybeFlags.value.attributes);
    else
        return { };
}
template <typename TypeMetadataT>
    constexpr auto make_flags_enum_serialization_data(const TypeMetadataT& enumMetadata)
{
    std::string_view defTypeName = get_or_default<std::string_view>(enumMetadata.attributes);
    std::string_view typeDesc = get_or_default<caption_metadata>(enumMetadata.attributes).value;
    std::string_view flagTypeName = get_flag_type_name(enumMetadata);
    auto values = enumMetadata.attributes
        | tuple_filter(template_trait_v<is_instantiation_of, value_metadata>)
        | tuple_map([](const auto& v) { return make_enum_value_serialization_data(v); })
        | array_cat<enum_value_serialization_data>();
    return flags_enum_serialization_data<array_size_v<decltype(values)>>{ values, flagTypeName, defTypeName, typeDesc };
}
template <typename EnumT, std::size_t N>
    std::string to_string(EnumT value, const flags_enum_serialization_data<N>& sdata, const enum_serialization_options& options)
{
    return flags_enum_to_string(std::uint64_t(value), sdata.data_ref(), options);
}
template <typename EnumT, std::size_t N>
    EnumT from_string(tag<EnumT>, std::string_view string, const flags_enum_serialization_data<N>& sdata, const enum_serialization_options& options)
{
    std::uint64_t value;
    flags_enum_from_string(string, value, sdata.data_ref(), options);
    return EnumT(value);
}
template <std::size_t N>
    std::string get_hint(const flags_enum_serialization_data<N>& sdata, const enum_hint_options& options)
{
    return flags_enum_hint(sdata.data_ref(), options);
}


template <typename TypeMetadataT>
    constexpr auto make_serialization_data(const TypeMetadataT& typeMetadata)
{
    using T = typename TypeMetadataT::type;

    if constexpr (std::is_enum<T>::value)
        return makeshift::detail::make_enum_serialization_data(typeMetadata);
    else if constexpr (std::is_base_of<makeshift::detail::flags_base, T>::value)
        return makeshift::detail::make_flags_enum_serialization_data(typeMetadata);
    else
        static_assert(sizeof(T) == ~0, "unsupported type");
}
template <typename T, typename MetadataTagT = serialization_tag>
    /*constexpr*/ auto serialization_data = make_serialization_data(metadata_of<T, MetadataTagT>);


template <typename MetadataTagT, typename T> struct is_enum_with_metadata : std::conjunction<have_metadata<T, MetadataTagT>, std::is_enum<T>> { };


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_SERIALIZE_ENUM_HPP_
