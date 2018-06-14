
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

#include <makeshift/type_traits.hpp> // for tag<>, flags_base
#include <makeshift/metadata.hpp>
#include <makeshift/tuple.hpp>
#include <makeshift/array.hpp>       // for to_array()

#include <makeshift/detail/cfg.hpp>  // for MAKESHIFT_SYS_DLLFUNC

#include <gsl/span>



namespace makeshift
{


inline namespace metadata
{


    // Base class for metadata-based serializers.
template <typename MetadataTagT = default_metadata_tag>
    struct metadata_serializer_t
{
    using metadata_tag = MetadataTagT;
};
template <typename MetadataTagT = default_metadata_tag> constexpr metadata_serializer_t<MetadataTagT> metadata_serializer { };


} // inline namespace metadat


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
    constexpr streamable_rvalue(const T& _value, SerializerT&& _serializer) noexcept : value_(_value), serializer_(std::forward<SerializerT>(_serializer)) { }

    friend std::ostream& operator <<(std::ostream& stream, const streamable_rvalue& self)
    {
        to_stream_impl(self.value_, stream, self.serializer_, self.serializer_);
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
    constexpr streamable_lvalue(T& _value, SerializerT&& _serializer) noexcept : value_(_value), serializer_(std::forward<SerializerT>(_serializer)) { }

    friend std::ostream& operator <<(std::ostream& stream, const streamable_lvalue& self)
    {
        to_stream_impl(self.value_, stream, self.serializer_, self.serializer_);
        return stream;
    }
    friend std::istream& operator >>(std::istream& stream, const streamable_lvalue& self)
    {
        from_stream_impl(self.value_, stream, self.serializer_, self.serializer_);
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
MAKESHIFT_SYS_DLLFUNC unsigned scalar_from_string(tag_t<unsigned>, const std::string& string);
inline long scalar_from_string(tag_t<long>, const std::string& string) { return std::stol(string); }
inline unsigned long scalar_from_string(tag_t<unsigned long>, const std::string& string) { return std::stoul(string); }
inline long long scalar_from_string(tag_t<long long>, const std::string& string) { return std::stoll(string); }
inline unsigned long long scalar_from_string(tag_t<unsigned long long>, const std::string& string) { return std::stoull(string); }
inline float scalar_from_string(tag_t<float>, const std::string& string) { return std::stof(string); }
inline double scalar_from_string(tag_t<double>, const std::string& string) { return std::stod(string); }
inline long double scalar_from_string(tag_t<long double>, const std::string& string) { return std::stold(string); }


template <typename MetadataTagT, typename T> struct is_enum_with_metadata : std::conjunction<have_metadata<T, MetadataTagT>, std::is_enum<T>> { };

template <typename MetadataTagT, typename T> struct have_string_conversion : is_enum_with_metadata<MetadataTagT, T> { };
template <typename MetadataTagT> struct have_string_conversion<MetadataTagT, std::string> : std::true_type { };
template <typename MetadataTagT> struct have_string_conversion<MetadataTagT, int> : std::true_type { };
template <typename MetadataTagT> struct have_string_conversion<MetadataTagT, unsigned> : std::true_type { };
template <typename MetadataTagT> struct have_string_conversion<MetadataTagT, long> : std::true_type { };
template <typename MetadataTagT> struct have_string_conversion<MetadataTagT, unsigned long> : std::true_type { };
template <typename MetadataTagT> struct have_string_conversion<MetadataTagT, long long> : std::true_type { };
template <typename MetadataTagT> struct have_string_conversion<MetadataTagT, unsigned long long> : std::true_type { };
template <typename MetadataTagT> struct have_string_conversion<MetadataTagT, float> : std::true_type { };
template <typename MetadataTagT> struct have_string_conversion<MetadataTagT, double> : std::true_type { };
template <typename MetadataTagT> struct have_string_conversion<MetadataTagT, long double> : std::true_type { };
template <typename MetadataTagT, typename T> constexpr bool have_string_conversion_v = have_string_conversion<MetadataTagT, T>::value;

template <typename MetadataTagT, typename T> using have_ostream_operator_r = decltype(std::declval<std::ostream&>() << std::declval<const T&>());
template <typename MetadataTagT, typename T> using have_ostream_operator = std::disjunction<is_enum_with_metadata<MetadataTagT, T>, can_apply<have_ostream_operator_r, MetadataTagT, T>>;
template <typename MetadataTagT, typename T> constexpr bool have_ostream_operator_v = have_ostream_operator<MetadataTagT, T>::value;

template <typename MetadataTagT, typename T> using have_istream_operator_r = decltype(std::declval<std::istream&>() >> std::declval<T&>());
template <typename MetadataTagT, typename T> using have_istream_operator = std::disjunction<is_enum_with_metadata<MetadataTagT, T>, can_apply<have_istream_operator_r, MetadataTagT, T>>;
template <typename MetadataTagT, typename T> constexpr bool have_istream_operator_v = have_istream_operator<MetadataTagT, T>::value;


template <typename MetadataTagT = default_metadata_tag> struct builtin_string_serializer_base_t : metadata_serializer_t<MetadataTagT> { };
template <typename T, typename MetadataTagT, typename SerializerT,
          typename = std::enable_if_t<have_string_conversion_v<MetadataTagT, std::decay_t<T>>>>
    std::string to_string_impl(const T& value, builtin_string_serializer_base_t<MetadataTagT>, SerializerT&)
{
    using D = std::decay_t<T>;
    if constexpr (std::is_enum<D>::value)
        return to_string(value, serialization_data<D, MetadataTagT>);
    else
        return scalar_to_string(value);
}
template <typename T, typename MetadataTagT, typename SerializerT,
          typename = std::enable_if_t<have_string_conversion_v<MetadataTagT, std::decay_t<T>>>>
    T from_string_impl(tag_t<T>, const std::string& string, builtin_string_serializer_base_t<MetadataTagT>, SerializerT&)
{
    if constexpr (std::is_enum<T>::value)
        return from_string(tag<T>, string, serialization_data<T, MetadataTagT>);
    else
        return scalar_from_string(tag<T>, string);
}

template <typename MetadataTagT = default_metadata_tag> struct stream_serializer_base_t : metadata_serializer_t<MetadataTagT> { };
template <typename T, typename MetadataTagT, typename SerializerT,
          typename = std::enable_if_t<have_ostream_operator_v<MetadataTagT, std::decay_t<T>>>>
    void to_stream_impl(const T& value, std::ostream& stream, stream_serializer_base_t<MetadataTagT>, SerializerT&)
{
    using D = std::decay_t<T>;
    if constexpr (std::is_enum<D>::value)
        to_stream(value, stream, serialization_data<D, MetadataTagT>);
    else
        stream << value;
}
template <typename T, typename MetadataTagT, typename SerializerT,
          typename = std::enable_if_t<have_istream_operator_v<MetadataTagT, std::decay_t<T>>>>
    void from_stream_impl(T& value, std::istream& stream, stream_serializer_base_t<MetadataTagT>, SerializerT&)
{
    using D = std::decay_t<T>;
    if constexpr (std::is_enum<D>::value)
        from_stream(value, stream, serialization_data<D, MetadataTagT>);
    else
        stream >> value;
}


} // namespace detail


inline namespace serialize
{


    // To customize string and/or stream serialization for arbitrary types, define your own serializer type and define `to_string_impl()`, `from_string_impl()`,
    // `to_stream_impl()`, `from_stream_impl()` overloads in the same namespace. Use `combine()` to combine multiple serializers.
    //
    // To override parts of the behavior of an existing serializer, have your serializer inherit from the existing serializer. Do not inherit from
    // serializers with orthogonal concerns and try to keep the scope of a serializer as small as possible to permit unambiguous combination.
    //
    // Orthogonal serializers can be combined with `combine()`, cf. the definition of `default_serializer<>`.


    // String serializer for common scalar types (built-in types and std::string).
template <typename MetadataTagT = default_metadata_tag> struct builtin_string_serializer_t : makeshift::detail::builtin_string_serializer_base_t<MetadataTagT> { };

    // String serializer for common scalar types (built-in types and std::string).
template <typename MetadataTagT = default_metadata_tag> constexpr builtin_string_serializer_t<MetadataTagT> builtin_string_serializer { };


    // Stream serializer for enums with metadata and for types with overloaded stream operators.
template <typename MetadataTagT = default_metadata_tag> struct stream_serializer_t : makeshift::detail::stream_serializer_base_t<MetadataTagT> { };

    // Stream serializer for enums with metadata and for types with overloaded stream operators.
template <typename MetadataTagT = default_metadata_tag> constexpr stream_serializer_t<MetadataTagT> stream_serializer { };


    // Default serializer.
template <typename MetadataTagT = default_metadata_tag>
    constexpr auto default_serializer = combine(
        builtin_string_serializer<MetadataTagT>,
        stream_serializer<MetadataTagT>
    );


    // Serializes the given value as string using the provided serializer.
    //
    //     std::string s = to_string(42, builtin_string_serializer<>); // returns "42"s
    //
template <typename T, typename SerializerT>
    std::string to_string(const T& value, SerializerT& serializer)
{
    return to_string_impl(value, serializer, serializer);
}


    // Serializes the given value as string using the default serializer.
    //
    //     std::string s = to_string(42); // returns "42"s
    //
template <typename T>
    std::string to_string(const T& value)
{
    return to_string(value, default_serializer<>);
}


    // Deserializes the given value from a string using the provided serializer.
    //
    //     int i = from_string(tag<int>, "42", builtin_string_serializer<>); // returns 42
    //
template <typename T, typename SerializerT>
    T from_string(tag_t<T>, const std::string& string, SerializerT& serializer)
{
    return from_string_impl(tag<T>, string, serializer, serializer);
}


    // Deserializes the given value from a string using the default serializer.
    //
    //     int i = from_string(tag<int>, "42"); // returns 42
    //
template <typename T>
    T from_string(tag_t<T>, const std::string& string)
{
    return from_string(tag<T>, string, default_serializer<>);
}


    // Wraps the given rvalue as a streamable object using the serializer provided.
    //
    //     std::cout << streamable(vec.size(), stream_serializer<>) << '\n';
    //
template <typename T, typename SerializerT>
    auto streamable(const T& value, SerializerT&& serializer)
{
    return makeshift::detail::streamable_rvalue { value, std::forward<SerializerT>(serializer) };
}


    // Wraps the given rvalue as a streamable object using the default serializer.
    //
    //     std::cout << streamable(vec.size()) << '\n';
    //
template <typename T>
    auto streamable(const T& value)
{
    return makeshift::detail::streamable_rvalue { value, default_serializer<> };
}


    // Wraps the given lvalue as a streamable object using the serializer provided.
    //
    //     int i;
    //     std::cin >> streamable(i, stream_serializer<>);
    //     std::cout << streamable(i, stream_serializer<>) << '\n';
    //
template <typename T, typename SerializerT>
    auto streamable(T& value, SerializerT&& serializer)
{
    return makeshift::detail::streamable_lvalue { value, std::forward<SerializerT>(serializer) };
}


    // Wraps the given lvalue as a streamable object using the default serializer.
    //
    //     int i;
    //     std::cin >> streamable(i);
    //     std::cout << streamable(i) << '\n';
    //
template <typename T>
    auto streamable(T& value)
{
    return makeshift::detail::streamable_lvalue { value, default_serializer<> };
}


} // inline namespace serialize

} // namespace makeshift


#endif // MAKESHIFT_SERIALIZE_HPP_
