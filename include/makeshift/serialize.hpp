
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

#include <makeshift/type_traits.hpp> // for tag<>
#include <makeshift/metadata.hpp>
#include <makeshift/tuple.hpp>
#include <makeshift/array.hpp>       // for to_array()

#include <makeshift/detail/cfg.hpp>  // for MAKESHIFT_SYS_DLLFUNC

#include <gsl/span>



namespace makeshift
{

namespace detail
{

    // Default implementations for to_stream() and from_stream().
    // These are used in streamable_[l|r]value<> and found only if ADL doesn't find an implementation for the given SerializationContextT,
    // or if SerializationContextT is a serialization context defined by makeshift.
template <typename T, typename SerializationContextT>
    void to_stream(std::ostream& stream, const T& value, const SerializationContextT& sctx)
{
    stream << to_string(value, sctx);
}
template <typename T, typename SerializationContextT>
    T from_stream(tag_t<T>, std::istream& stream, const SerializationContextT& sctx)
{
    std::string str;
    stream >> str;
    return from_string(tag<T>, str, sctx);
}

struct enum_value_serialization_context
{
    std::uint64_t value;
    std::string_view string;

    constexpr enum_value_serialization_context(void) noexcept : value(0), string{ } { }
    constexpr enum_value_serialization_context(std::uint64_t _value, std::string_view _string) noexcept : value(_value), string(_string) { }
};
struct enum_serialization_data_ref
{
    gsl::span<const enum_value_serialization_context> values;
    std::string_view typeName;
    std::string_view typeDesc;
};
struct flags_enum_serialization_data_ref
{
    gsl::span<const enum_value_serialization_context> values;
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
    constexpr enum_value_serialization_context make_enum_value_serialization_context(const value_metadata<ValC, std::tuple<AttributesT...>>& valueMetadata)
{
    static_assert(sizeof(ValC::value) <= sizeof(std::uint64_t), "enums with an underlying type of more than 64 bits are not supported");

    std::string_view name = std::get<std::string_view>(valueMetadata.attributes);
    return { std::uint64_t(ValC::value), name };
}

template <std::size_t N>
    struct enum_serialization_context
{
    std::array<enum_value_serialization_context, N> values;
    std::string_view typeName;
    std::string_view typeDesc;

    constexpr enum_serialization_data_ref data(void) const noexcept { return { values, typeName, typeDesc }; }
};
template <typename T> using is_value_metadata = is_same_template<T, value_metadata>;
template <typename EnumT, typename AttributesT>
    constexpr auto make_enum_serialization_context(const type_metadata<EnumT, AttributesT>& enumMetadata)
{
    std::string_view typeName = get_or_default<std::string_view>(enumMetadata.attributes);
    std::string_view typeDesc = get_or_default<description_t>(enumMetadata.attributes).value;
    auto values = enumMetadata.attributes
        | tuple_filter<is_value_metadata>()
        | tuple_map([](const auto& v) { return make_enum_value_serialization_context(v); })
        | to_array();
    return enum_serialization_context<array_size_v<decltype(values)>> { values, typeName, typeDesc };
}

template <std::size_t N>
    struct flags_enum_serialization_context
{
    std::array<enum_value_serialization_context, N> values;
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
    constexpr auto make_flags_enum_serialization_context(const type_metadata<DefT, AttributesT>& enumMetadata)
{
    std::string_view defTypeName = get_or_default<std::string_view>(enumMetadata.attributes);
    std::string_view typeDesc = get_or_default<description_t>(enumMetadata.attributes).value;
    std::string_view flagTypeName = get_flag_type_name(enumMetadata);
    auto values = enumMetadata.attributes
        | tuple_filter<is_value_metadata>()
        | tuple_map([](const auto& v) { return make_enum_value_serialization_context(v); })
        | to_array();
    return flags_enum_serialization_context<array_size_v<decltype(values)>>{ values, flagTypeName, defTypeName, typeDesc };
}

template <typename EnumT, std::size_t N>
    std::string to_string(EnumT value, const enum_serialization_context<N>& sctx)
{
    return enum_to_string(std::uint64_t(value), sctx.data());
}
template <typename EnumT, std::size_t N>
    EnumT from_string(tag_t<EnumT>, const std::string& string, const enum_serialization_context<N>& sctx)
{
    return EnumT(string_to_enum(string, sctx.data()));
}

template <typename EnumT, std::size_t N>
    std::string to_string(EnumT value, const flags_enum_serialization_context<N>& sctx)
{
    return flags_enum_to_string(std::uint64_t(value), sctx.data());
}
template <typename EnumT, std::size_t N>
    EnumT from_string(tag_t<EnumT>, const std::string& string, const flags_enum_serialization_context<N>& sctx)
{
    return EnumT(string_to_flags_enum(string, sctx.data()));
}

template <typename T, typename SerializationContextT>
    struct streamable_rvalue
{
    streamable_rvalue(const streamable_rvalue&) = delete;
    streamable_rvalue(streamable_rvalue&&) = delete;
    streamable_rvalue& operator =(const streamable_rvalue&) = delete;
    streamable_rvalue& operator =(streamable_rvalue&&) = delete;

private:
    const T& value_;
    SerializationContextT sctx_;

public:
    constexpr streamable_rvalue(const T& _value, SerializationContextT&& _sctx) noexcept : value_(_value), sctx_(std::move(_sctx)) { }

    friend std::ostream& operator <<(std::ostream& stream, const streamable_rvalue& self)
    {
        to_stream(stream, self.value_, self.sctx_);
        return stream;
    }
};
template <typename T, typename SerializationContextT>
    streamable_rvalue(const T& value, SerializationContextT&& sctx) -> streamable_rvalue<T, remove_rvalue_reference_t<SerializationContextT>>;

template <typename T, typename SerializationContextT>
    struct streamable_lvalue
{
    streamable_lvalue(const streamable_lvalue&) = delete;
    streamable_lvalue(streamable_lvalue&&) = delete;
    streamable_lvalue& operator =(const streamable_lvalue&) = delete;
    streamable_lvalue& operator =(streamable_lvalue&&) = delete;

private:
    T& value_;
    SerializationContextT sctx_;

public:
    constexpr streamable_lvalue(T& _value, SerializationContextT&& _sctx) noexcept : value_(_value), sctx_(std::move(_sctx)) { }

    friend std::ostream& operator <<(std::ostream& stream, const streamable_lvalue& self)
    {
        to_stream(self.value_, self.sctx_);
        return stream;
    }
    friend std::istream& operator >>(std::istream& stream, const streamable_lvalue& self)
    {
        self.value_ = from_stream(tag<T>, stream, self.sctx_);
        return stream;
    }
};
template <typename T, typename SerializationContextT>
    streamable_lvalue(T& value, SerializationContextT&& sctx) -> streamable_lvalue<T, remove_rvalue_reference_t<SerializationContextT>>;

} // namespace detail

inline namespace serialize
{

template <typename T, typename... AttributesT>
    constexpr auto make_serialization_context(const type_metadata<T, std::tuple<AttributesT...>>& typeMetadata)
{
    if constexpr (std::is_enum<T>::value)
        return makeshift::detail::make_enum_serialization_context(typeMetadata);
    else if constexpr (std::is_base_of<makeshift::detail::flags_base, T>::value)
        return makeshift::detail::make_flags_enum_serialization_context(typeMetadata);
    else
        static_assert(sizeof(T) == ~0, "unsupported type");
}

template <typename T>
    static /*constexpr*/ auto default_serialization_context { make_serialization_context(metadata_of<T>) };
    
template <typename T>
    auto streamable(const T& value)
{
    return makeshift::detail::streamable_rvalue { value, default_serialization_context<T> };
}
template <typename T, typename SerializationContextT>
    auto streamable(const T& value, SerializationContextT&& sctx)
{
    return makeshift::detail::streamable_rvalue { value, std::forward<SerializationContextT>(sctx) };
}
template <typename T>
    auto streamable(T& value)
{
    return makeshift::detail::streamable_lvalue { value, default_serialization_context<T> };
}
template <typename T, typename SerializationContextT>
    auto streamable(T& value, SerializationContextT&& sctx)
{
    return makeshift::detail::streamable_lvalue { value, std::forward<SerializationContextT>(sctx) };
}

template <typename T>
    std::string to_string(const T& value)
{
    return to_string(value, default_serialization_context<T>);
}
template <typename T>
    T from_string(const std::string& string)
{
    return from_string(tag<T>, string, default_serialization_context<T>);
}
template <typename T>
    T from_string(tag_t<T>, const std::string& string)
{
    return from_string(tag<T>, string, default_serialization_context<T>);
}

} // inline namespace serialize

} // namespace makeshift

#endif // MAKESHIFT_SERIALIZE_HPP_
