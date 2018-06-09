
#ifndef MAKESHIFT_SERIALIZE_HPP_
#define MAKESHIFT_SERIALIZE_HPP_


#include <string>      // for string, to_string<>
#include <string_view>
#include <array>
#include <type_traits> // for decay<>, is_same<>
#include <utility>     // for move(), forward<>()
#include <iosfwd>      // for istream, ostream
#include <cstddef>     // for size_t
#include <cstdint>     // for uint64_t

#include <makeshift/type_traits.hpp> // for tag<>
#include <makeshift/metadata.hpp>
#include <makeshift/tuple.hpp>
#include <makeshift/array.hpp>       // for to_array()
#include <makeshift/arithmetic.hpp>  // for checked_cast<>()

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

    std::string_view name = std::get<std::string_view>(valueMetadata.attributes);
    return { std::uint64_t(ValC::value), name };
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
    std::string_view typeDesc = get_or_default<description_t>(enumMetadata.attributes).value;
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
template <typename EnumT, std::size_t N>
    void to_stream(EnumT value, std::ostream& stream, const enum_serialization_data<N>& sdata)
{
    stream << to_string(value, sdata);
}
template <typename EnumT, std::size_t N>
    void from_stream(EnumT& value, std::istream& stream, const enum_serialization_data<N>& sdata)
{
    std::string str;
    stream >> str;
    value = from_string(tag<EnumT>, str, sdata);
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
    std::string_view typeDesc = get_or_default<description_t>(enumMetadata.attributes).value;
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
template <typename EnumT, std::size_t N>
    void to_stream(EnumT value, std::ostream& stream, const flags_enum_serialization_data<N>& sdata)
{
    stream << to_string(value, sdata);
}
template <typename EnumT, std::size_t N>
    void from_stream(EnumT& value, std::istream& stream, const flags_enum_serialization_data<N>& sdata)
{
        // TODO: how to read flags from a string? how are they delimited? for now we just assume they are space-delimited
    std::string str;
    stream >> str;
    value = from_string(tag<EnumT>, str, sdata);
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
template <typename T, typename MetadataTagT = default_metadata_tag>
    constexpr auto serialization_data = make_serialization_data(metadata_of<T, MetadataTagT>);


template <typename T, typename SerializerT>
    struct streamable_rvalue
{
    streamable_rvalue(const streamable_rvalue&) = delete;
    streamable_rvalue(streamable_rvalue&&) = delete;
    streamable_rvalue& operator =(const streamable_rvalue&) = delete;
    streamable_rvalue& operator =(streamable_rvalue&&) = delete;

private:
    const T& value_;
    SerializerT serializer_;

public:
    constexpr streamable_rvalue(const T& _value, SerializerT&& _serializer) noexcept : value_(_value), serializer_(std::move(_serializer)) { }

    friend std::ostream& operator <<(std::ostream& stream, const streamable_rvalue& self)
    {
        to_stream(self.value_, stream, self.serializer_);
        return stream;
    }
};
template <typename T, typename SerializerT>
    streamable_rvalue(const T& value, SerializerT&& serializer) -> streamable_rvalue<T, remove_rvalue_reference_t<SerializerT>>;

template <typename T, typename SerializerT>
    struct streamable_lvalue
{
    streamable_lvalue(const streamable_lvalue&) = delete;
    streamable_lvalue(streamable_lvalue&&) = delete;
    streamable_lvalue& operator =(const streamable_lvalue&) = delete;
    streamable_lvalue& operator =(streamable_lvalue&&) = delete;

private:
    T& value_;
    SerializerT serializer_;

public:
    constexpr streamable_lvalue(T& _value, SerializerT&& _serializer) noexcept : value_(_value), serializer_(std::move(_serializer)) { }

    friend std::ostream& operator <<(std::ostream& stream, const streamable_lvalue& self)
    {
        to_stream(self.value_, stream, self.serializer_);
        return stream;
    }
    friend std::istream& operator >>(std::istream& stream, const streamable_lvalue& self)
    {
        from_stream(self.value_, stream, self.serializer_);
        return stream;
    }
};
template <typename T, typename SerializerT>
    streamable_lvalue(T& value, SerializerT&& serializer) -> streamable_lvalue<T, remove_rvalue_reference_t<SerializerT>>;


inline std::string scalar_to_string(std::string s) { return std::move(s); }
inline std::string scalar_to_string(int val) { return std::to_string(val); }
inline std::string scalar_to_string(unsigned val) { return std::to_string(val); }
inline std::string scalar_to_string(long val) { return std::to_string(val); }
inline std::string scalar_to_string(unsigned long val) { return std::to_string(val); }
inline std::string scalar_to_string(long long val) { return std::to_string(val); }
inline std::string scalar_to_string(unsigned long long val) { return std::to_string(val); }
inline std::string scalar_to_string(float val) { return std::to_string(val); }
inline std::string scalar_to_string(double val) { return std::to_string(val); }
inline std::string scalar_to_string(long double val) { return std::to_string(val); }

inline std::string scalar_from_string(tag_t<std::string>, const std::string& s) { return s; }
inline int scalar_from_string(tag_t<int>, const std::string& string) { return std::stoi(string); }
inline unsigned scalar_from_string(tag_t<unsigned>, const std::string& string) { return checked_cast<unsigned>(std::stoul(string)); }
inline long scalar_from_string(tag_t<long>, const std::string& string) { return std::stol(string); }
inline unsigned long scalar_from_string(tag_t<unsigned long>, const std::string& string) { return std::stoul(string); }
inline long long scalar_from_string(tag_t<long long>, const std::string& string) { return std::stoll(string); }
inline unsigned long long scalar_from_string(tag_t<unsigned long long>, const std::string& string) { return std::stoull(string); }
inline float scalar_from_string(tag_t<float>, const std::string& string) { return std::stof(string); }
inline double scalar_from_string(tag_t<double>, const std::string& string) { return std::stod(string); }
inline long double scalar_from_string(tag_t<long double>, const std::string& string) { return std::stold(string); }

} // namespace detail

inline namespace serialize
{

template <typename MetadataTagT = default_metadata_tag> struct default_serializer_t { };
template <typename MetadataTagT = default_metadata_tag> constexpr default_serializer_t<MetadataTagT> default_serializer { };
    
template <typename T, typename MetadataTagT = default_metadata_tag>
    std::string to_string(const T& value, default_serializer_t<MetadataTagT> = { })
{
    using D = std::decay_t<T>;
    if constexpr (have_metadata_v<D, MetadataTagT>)
        return to_string(value, makeshift::detail::serialization_data<T, MetadataTagT>);
    else
        return makeshift::detail::scalar_to_string(value);
}
template <typename T, typename MetadataTagT = default_metadata_tag>
    T from_string(tag_t<T>, const std::string& string, default_serializer_t<MetadataTagT> = { })
{
    using D = std::decay_t<T>;
    if constexpr (have_metadata_v<D, MetadataTagT>)
        return from_string(tag<T>, string, makeshift::detail::serialization_data<T, MetadataTagT>);
    else
        return makeshift::detail::scalar_from_string(tag<T>, string);
}
template <typename T, typename MetadataTagT = default_metadata_tag>
    void to_stream(const T& value, std::ostream& stream, default_serializer_t<MetadataTagT> = { })
{
    using D = std::decay_t<T>;
    if constexpr (have_metadata_v<D, MetadataTagT>)
        to_stream(value, stream, makeshift::detail::serialization_data<T, MetadataTagT>);
    else
        stream << value;
}
template <typename T, typename MetadataTagT = default_metadata_tag>
    void from_stream(T& value, std::istream& stream, default_serializer_t<MetadataTagT> = { })
{
    if constexpr (have_metadata_v<T, MetadataTagT>)
        from_stream(value, stream, makeshift::detail::serialization_data<T, MetadataTagT>);
    else
        stream >> value;
}

template <typename T>
    auto streamable(const T& value)
{
    return makeshift::detail::streamable_rvalue { value, default_serializer<> };
}
template <typename T, typename SerializerT>
    auto streamable(const T& value, SerializerT&& serializer)
{
    return makeshift::detail::streamable_rvalue { value, std::forward<SerializerT>(serializer) };
}
template <typename T>
    auto streamable(T& value)
{
    return makeshift::detail::streamable_lvalue { value, default_serializer<> };
}
template <typename T, typename SerializerT>
    auto streamable(T& value, SerializerT&& serializer)
{
    return makeshift::detail::streamable_lvalue { value, std::forward<SerializerT>(serializer) };
}

} // inline namespace serialize

} // namespace makeshift

#endif // MAKESHIFT_SERIALIZE_HPP_
