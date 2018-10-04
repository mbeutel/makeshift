
#ifndef INCLUDED_MAKESHIFT_SERIALIZERS_STREAM_HPP_
#define INCLUDED_MAKESHIFT_SERIALIZERS_STREAM_HPP_


#include <string>      // for string, to_string()
#include <string_view>
#include <type_traits> // for decay<>, is_enum<>
#include <utility>     // for forward<>()
#include <iosfwd>      // for istream, ostream
#include <cstddef>     // for size_t

#include <makeshift/type_traits.hpp> // for tag<>
#include <makeshift/metadata.hpp>
#include <makeshift/serialize.hpp>   // for define_serializer<>, metadata_tag_of_serializer<>
#include <makeshift/streamable.hpp>

#include <makeshift/detail/export.hpp>         // for MAKESHIFT_PUBLIC
#include <makeshift/detail/serialize_enum.hpp>


namespace makeshift
{

namespace detail
{


[[noreturn]] MAKESHIFT_PUBLIC void raise_ostream_error(std::ostream& stream);
[[noreturn]] MAKESHIFT_PUBLIC void raise_istream_error(std::istream& stream);


MAKESHIFT_PUBLIC void enum_to_stream(std::ostream& stream, std::string_view name, const enum_serialization_options& options);
MAKESHIFT_PUBLIC void enum_from_stream(std::istream& stream, std::string& name, const enum_serialization_options& options);


template <typename EnumT, std::size_t N, typename SerializerT>
    void enum_to_stream(EnumT value, std::ostream& stream, const enum_serialization_data<N>& sdata, const enum_serialization_options& options, SerializerT&)
{
    enum_to_stream(stream, to_string(value, sdata, options), options);
}
template <typename EnumT, std::size_t N, typename SerializerT>
    void enum_from_stream(EnumT& value, std::istream& stream, const enum_serialization_data<N>& sdata, const enum_serialization_options& options, SerializerT&)
{
    std::string str;
    enum_from_stream(stream, str, options);
    value = from_string(tag_v<EnumT>, str, sdata, options);
}

template <typename EnumT, std::size_t N, typename SerializerT>
    void enum_to_stream(EnumT value, std::ostream& stream, const flags_enum_serialization_data<N>& sdata, const enum_serialization_options& options, SerializerT&)
{
    enum_to_stream(stream, to_string(value, sdata, options), options);
}
template <typename EnumT, std::size_t N, typename SerializerT>
    void enum_from_stream(EnumT& value, std::istream& stream, const flags_enum_serialization_data<N>& sdata, const enum_serialization_options& options, SerializerT&)
{
    std::string str;
    enum_from_stream(stream, str, options);
    value = from_string(tag_v<EnumT>, str, sdata, options);
}


MAKESHIFT_PUBLIC void string_to_stream(std::ostream& stream, std::string_view string);
MAKESHIFT_PUBLIC void string_from_stream(std::istream& stream, std::string& string);


    // defined in serializers_stream-reflect.hpp
template <typename T, typename SerializerT>
    void compound_to_stream(std::ostream& stream, const T& value, SerializerT& serializer, const any_compound_serialization_options& compoundOptions);
template <typename T, typename SerializerT>
    void compound_from_stream(std::istream& stream, T& value, SerializerT& serializer, const any_compound_serialization_options& compoundOptions);


template <typename ConstrainedIntT, typename SerializerT>
    void constrained_integer_to_stream(ConstrainedIntT value, std::ostream& stream, SerializerT& serializer)
{
    stream << streamable(value.value(), serializer);
}

template <typename ConstrainedIntT, typename SerializerT>
    void constrained_integer_from_stream(ConstrainedIntT& value, std::istream& stream, SerializerT& serializer)
{
    using Int = typename ConstrainedIntT::value_type;
    Int result;
    stream >> streamable(result, serializer);
    value = ConstrainedIntT::check(result);
}


struct serializer_default_base
{
    template <typename T, typename SerializerT>
        friend std::enable_if_t<is_ostreamable_v<T>>
        to_stream_impl(const T& value, std::ostream& stream, const serializer_default_base&, SerializerT&)
    {
        if (!(stream << value))
            raise_ostream_error(stream);
    }
    template <typename T, typename SerializerT>
        friend std::enable_if_t<is_istreamable_v<T>>
        from_stream_impl(T& value, std::istream& stream, const serializer_default_base&, SerializerT&)
    {
        if (!(stream >> value))
            raise_istream_error(stream);
    }
};


} // namespace detail


inline namespace serialize
{


    // To customize stream serialization for arbitrary types, define your own serializer type along with `to_stream_impl()`, `from_stream_impl()`
    // overloads discoverable by ADL.
    //
    // To override parts of the behavior of an existing serializer, define a new serializer that handles only the types you want to customize, and
    // chain it with the existing serializer using `chain()`.


    //ᅟ
    // Runtime arguments for stream serializers.
    //
struct stream_serializer_options
{
    enum_serialization_options enum_options;
    any_compound_serialization_options any_compound_options;

    constexpr stream_serializer_options(void) noexcept = default;
    constexpr stream_serializer_options(enum_serialization_options _enum_options) noexcept : enum_options(_enum_options) { }
};


    //ᅟ
    // Stream serializer for enums with metadata and for types with overloaded stream operators.
    //
template <typename BaseT = void>
    struct stream_serializer : define_serializer<stream_serializer, BaseT, stream_serializer_options>, makeshift::detail::serializer_default_base
{
    using base = define_serializer<makeshift::stream_serializer, BaseT, stream_serializer_options>;
    using base::base;
    
    template <typename T, typename SerializerT>
        friend std::enable_if_t<is_constrained_integer_v<T>>
        to_stream_impl(const T& value, std::ostream& stream, const stream_serializer&, SerializerT& serializer)
    {
        makeshift::detail::constrained_integer_to_stream(value, stream, serializer);
    }
    template <typename T, typename SerializerT>
        friend std::enable_if_t<is_compound_v<T, metadata_tag_of_serializer_t<std::decay_t<SerializerT>>>
                             && have_metadata_v<T, metadata_tag_of_serializer_t<std::decay_t<SerializerT>>>>
        to_stream_impl(const T& value, std::ostream& stream, const stream_serializer& streamSerializer, SerializerT& serializer)
    {
        makeshift::detail::compound_to_stream(stream, value, serializer, data(streamSerializer).any_compound_options);
    }
    template <typename T, typename SerializerT>
        friend std::enable_if_t<std::is_enum<T>::value && have_metadata_v<T, metadata_tag_of_serializer_t<std::decay_t<SerializerT>>>>
        to_stream_impl(const T& value, std::ostream& stream, const stream_serializer& streamSerializer, SerializerT& serializer)
    {
        using MetadataTag = metadata_tag_of_serializer_t<std::decay_t<SerializerT>>;
        enum_to_stream(value, stream, makeshift::detail::serialization_data<T, MetadataTag>, data(streamSerializer).enum_options, serializer);
    }
    template <typename T, typename SerializerT>
        friend std::enable_if_t<std::is_convertible<T, std::string_view>::value>
        to_stream_impl(const T& value, std::ostream& stream, const stream_serializer&, SerializerT&)
    {
        makeshift::detail::string_to_stream(stream, value);
    }

    template <typename T, typename SerializerT>
        friend std::enable_if_t<is_constrained_integer_v<T>>
        from_stream_impl(T& value, std::istream& stream, const stream_serializer&, SerializerT& serializer)
    {
        makeshift::detail::constrained_integer_from_stream(value, stream, serializer);
    }
    template <typename T, typename SerializerT>
        friend std::enable_if_t<is_compound_v<T, metadata_tag_of_serializer_t<std::decay_t<SerializerT>>>
                             && have_metadata_v<T, metadata_tag_of_serializer_t<std::decay_t<SerializerT>>>>
        from_stream_impl(T& value, std::istream& stream, const stream_serializer& streamSerializer, SerializerT& serializer)
    {
        makeshift::detail::compound_from_stream(stream, value, serializer, data(streamSerializer).any_compound_options);
    }
    template <typename T, typename SerializerT>
        friend std::enable_if_t<std::is_enum<T>::value && have_metadata_v<T, metadata_tag_of_serializer_t<std::decay_t<SerializerT>>>>
        from_stream_impl(T& value, std::istream& stream, const stream_serializer& streamSerializer, SerializerT& serializer)
    {
        using MetadataTag = metadata_tag_of_serializer_t<std::decay_t<SerializerT>>;
        enum_from_stream(value, stream, makeshift::detail::serialization_data<T, MetadataTag>, data(streamSerializer).enum_options, serializer);
    }
    template <typename T, typename SerializerT>
        friend std::enable_if_t<std::is_convertible<T, std::string_view>::value>
        from_stream_impl(T& value, std::istream& stream, const stream_serializer&, SerializerT&)
    {
        makeshift::detail::string_from_stream(stream, value);
    }
};
stream_serializer(void) -> stream_serializer<>;
stream_serializer(const stream_serializer_options&) -> stream_serializer<>;
stream_serializer(stream_serializer_options&&) -> stream_serializer<>;


} // inline namespace serialize


namespace detail
{

static constexpr stream_serializer<> default_stream_serializer{ };

template <typename T>
    struct streamable_ref_base<T, void>
{
    streamable_ref_base(const streamable_ref_base&) = delete;
    streamable_ref_base& operator =(const streamable_ref_base&) = delete;

private:
    T& value_;
    
public:
    constexpr streamable_ref_base(T& _value) noexcept : value_(_value) { }

    constexpr T& value(void) const noexcept { return value_; }
    static constexpr stream_serializer<> serializer(void) noexcept { return { }; }

    friend std::ostream& operator <<(std::ostream& stream, const streamable_ref_base& self)
    {
        to_stream_impl(self.value(), stream, default_stream_serializer, default_stream_serializer);
        return stream;
    }
};

} // namespace detail


inline namespace serialize
{


template <typename T>
    struct streamable_ref<T, void> : makeshift::detail::streamable_ref_base<T, void>
{
    using base = makeshift::detail::streamable_ref_base<T, void>;
    using base::base;

    friend std::istream& operator >>(std::istream& stream, const streamable_ref& self)
    {
        from_stream_impl(self.value(), stream, makeshift::detail::default_stream_serializer, makeshift::detail::default_stream_serializer);
        return stream;
    }
};
template <typename T>
    struct streamable_ref<const T, void> : makeshift::detail::streamable_ref_base<const T, void>
{
    using base = makeshift::detail::streamable_ref_base<const T, void>;
    using base::base;
};


    //ᅟ
    // Wraps the given reference as a streamable object using a default-initialized `stream_serializer<>`.
    //ᅟ
    //ᅟ    std::cout << streamable(vec.size()) << '\n';
    //ᅟ    int i;
    //ᅟ    std::cin >> streamable(i);
    //
template <typename T>
    constexpr streamable_ref<makeshift::detail::as_lvalue_t<T>, void>
    streamable(T&& value) noexcept
{
    return { value };
}


} // inline namespace serialize

} // namespace makeshift


#ifdef INCLUDED_MAKESHIFT_REFLECT_HPP_
 #include <makeshift/detail/serializers_stream-reflect.hpp>
#endif // INCLUDED_MAKESHIFT_REFLECT_HPP_


#endif // INCLUDED_MAKESHIFT_SERIALIZERS_STREAM_HPP_
