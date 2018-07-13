
#ifndef MAKESHIFT_STREAMABLE_HPP_
#define MAKESHIFT_STREAMABLE_HPP_


#include <string>      // for string, to_string()
#include <string_view>
#include <type_traits> // for decay<>, is_enum<>
#include <utility>     // for forward<>()
#include <iosfwd>      // for istream, ostream
#include <cstddef>     // for size_t

#include <makeshift/type_traits.hpp> // for tag<>
#include <makeshift/metadata.hpp>
#include <makeshift/serialize.hpp>   // for define_serializer<>

#include <makeshift/detail/serialize-enum.hpp>


namespace makeshift
{

namespace detail
{


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


template <typename MetadataTagT, typename T> using have_ostream_operator_r = decltype(std::declval<std::ostream&>() << std::declval<const T&>());
template <typename MetadataTagT, typename T> using have_ostream_operator = std::disjunction<is_enum_with_metadata<MetadataTagT, T>, can_apply<have_ostream_operator_r, MetadataTagT, T>>;
template <typename MetadataTagT, typename T> constexpr bool have_ostream_operator_v = have_ostream_operator<MetadataTagT, T>::value;

template <typename MetadataTagT, typename T> using have_istream_operator_r = decltype(std::declval<std::istream&>() >> std::declval<T&>());
template <typename MetadataTagT, typename T> using have_istream_operator = std::disjunction<is_enum_with_metadata<MetadataTagT, T>, can_apply<have_istream_operator_r, MetadataTagT, T>>;
template <typename MetadataTagT, typename T> constexpr bool have_istream_operator_v = have_istream_operator<MetadataTagT, T>::value;


template <typename EnumT, std::size_t N, typename SerializerT>
    void enum_to_stream_impl(EnumT value, std::ostream& stream, const enum_serialization_data<N>& sdata, const enum_serialization_options_t& options, SerializerT& serializer)
{
    stream << streamable_rvalue { to_string_impl(value, sdata, options), serializer };
}
template <typename EnumT, std::size_t N, typename SerializerT>
    void enum_from_stream_impl(EnumT& value, std::istream& stream, const enum_serialization_data<N>& sdata, const enum_serialization_options_t& options, SerializerT& serializer)
{
    std::string str;
    stream >> streamable_lvalue { str, serializer };
    value = from_string_impl(tag<EnumT>, str, sdata, options);
}

template <typename EnumT, std::size_t N, typename SerializerT>
    void enum_to_stream_impl(EnumT value, std::ostream& stream, const flags_enum_serialization_data<N>& sdata, const enum_serialization_options_t& options, SerializerT& serializer)
{
    stream << streamable_rvalue { to_string_impl(value, sdata, options), serializer };
}
template <typename EnumT, std::size_t N, typename SerializerT>
    void enum_from_stream_impl(EnumT& value, std::istream& stream, const flags_enum_serialization_data<N>& sdata, const enum_serialization_options_t& options, SerializerT& serializer)
{
    std::string str;
    stream >> streamable_lvalue { str, serializer };
    value = from_string_impl(tag<EnumT>, str, sdata, options);
}


} // namespace detail


inline namespace serialize
{


    // To customize stream serialization for arbitrary types, define your own serializer type along with `to_stream_impl()`, `from_stream_impl()`
    // overloads in the same namespace.
    //
    // To override parts of the behavior of an existing serializer, have your serializer inherit from the existing serializer. Do not inherit from
    // serializers with orthogonal concerns and try to keep the scope of a serializer as small as possible to permit unambiguous combination.
    //
    // Orthogonal serializers can be combined with `combine()`:
    //ᅟ
    //ᅟ    auto serializer = combine(
    //ᅟ        string_serializer,
    //ᅟ        stream_serializer
    //ᅟ    );


    //ᅟ
    // Runtime arguments for stream serializers.
    //
struct stream_serializer_args
{
    enum_serialization_options_t enum_options;

    constexpr stream_serializer_args(void) noexcept = default;
    constexpr stream_serializer_args(enum_serialization_options_t _enum_options) noexcept : enum_options(_enum_options) { }
};


    //ᅟ
    // Stream serializer for enums with metadata and for types with overloaded stream operators.
    //
template <typename BaseT = void>
    struct stream_serializer_t : define_serializer<stream_serializer_t, BaseT, stream_serializer_args>
{
    using base = define_serializer<makeshift::stream_serializer_t, BaseT, stream_serializer_args>;
    using base::base;
    
    template <typename T, typename SerializerT/*,
              typename = std::enable_if_t<makeshift::detail::have_ostream_operator_v<serializer_metadata_tag_t<std::decay_t<SerializerT>>, std::decay_t<T>>>*/>
        friend std::enable_if_t<makeshift::detail::have_ostream_operator_v<serializer_metadata_tag_t<std::decay_t<SerializerT>>, std::decay_t<T>>>
        to_stream_impl(const T& value, std::ostream& stream, const stream_serializer_t& streamSerializer, SerializerT& serializer)
    {
        (void) streamSerializer;
        (void) serializer;
        using D = std::decay_t<T>;
        if constexpr (std::is_enum<D>::value)
            enum_to_stream_impl(value, stream, makeshift::detail::serialization_data<D, serializer_metadata_tag_t<std::decay_t<SerializerT>>>, streamSerializer.enum_options, serializer);
        else
            stream << value;
    }
    template <typename T, typename SerializerT/*,
              typename = std::enable_if_t<makeshift::detail::have_istream_operator_v<serializer_metadata_tag_t<std::decay_t<SerializerT>>, std::decay_t<T>>>*/>
        friend std::enable_if_t<makeshift::detail::have_ostream_operator_v<serializer_metadata_tag_t<std::decay_t<SerializerT>>, std::decay_t<T>>>
        from_stream_impl(T& value, std::istream& stream, const stream_serializer_t& streamSerializer, SerializerT& serializer)
    {
        (void) serializer;
        (void) streamSerializer;
        using D = std::decay_t<T>;
        if constexpr (std::is_enum<D>::value)
            enum_from_stream_impl(value, stream, makeshift::detail::serialization_data<D, serializer_metadata_tag_t<std::decay_t<SerializerT>>>, streamSerializer.enum_options, serializer);
        else
            stream >> value;
    }
};

    //ᅟ
    // Default stream serializer for enums with metadata and for types with overloaded stream operators.
    //
constexpr stream_serializer_t<> stream_serializer{ };


    //ᅟ
    // Wraps the given rvalue as a streamable object using the serializer provided.
    //ᅟ
    //ᅟ    std::cout << streamable(vec.size(), stream_serializer) << '\n';
    //
template <typename T, typename SerializerT>
    auto streamable(const T& value, SerializerT&& serializer)
{
    return makeshift::detail::streamable_rvalue { value, std::forward<SerializerT>(serializer) };
}


    //ᅟ
    // Wraps the given rvalue as a streamable object using `stream_serializer`.
    //ᅟ
    //ᅟ    std::cout << streamable(vec.size()) << '\n';
    //
template <typename T>
    auto streamable(const T& value)
{
    return makeshift::detail::streamable_rvalue { value, stream_serializer };
}


    //ᅟ
    // Wraps the given lvalue as a streamable object using the serializer provided.
    //ᅟ
    //ᅟ    int i;
    //ᅟ    std::cin >> streamable(i, stream_serializer);
    //ᅟ    std::cout << streamable(i, stream_serializer) << '\n';
    //
template <typename T, typename SerializerT>
    auto streamable(T& value, SerializerT&& serializer)
{
    return makeshift::detail::streamable_lvalue { value, std::forward<SerializerT>(serializer) };
}


    //ᅟ
    // Wraps the given lvalue as a streamable object using `stream_serializer`.
    //ᅟ
    //ᅟ    int i;
    //ᅟ    std::cin >> streamable(i);
    //ᅟ    std::cout << streamable(i) << '\n';
    //
template <typename T>
    auto streamable(T& value)
{
    return makeshift::detail::streamable_lvalue { value, stream_serializer };
}


} // inline namespace serialize

} // namespace makeshift


#endif // MAKESHIFT_STREAMABLE_HPP_
