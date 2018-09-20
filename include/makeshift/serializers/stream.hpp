
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

#include <makeshift/detail/serialize_enum.hpp>


namespace makeshift
{

namespace detail
{


template <typename MetadataTagT, typename T> using have_ostream_operator_r = decltype(std::declval<std::ostream&>() << std::declval<const T&>());
template <typename MetadataTagT, typename T> using have_ostream_operator = std::disjunction<is_enum_with_metadata<MetadataTagT, T>, can_apply<have_ostream_operator_r, MetadataTagT, T>>;
template <typename MetadataTagT, typename T> constexpr bool have_ostream_operator_v = have_ostream_operator<MetadataTagT, T>::value;

template <typename MetadataTagT, typename T> using have_istream_operator_r = decltype(std::declval<std::istream&>() >> std::declval<T&>());
template <typename MetadataTagT, typename T> using have_istream_operator = std::disjunction<is_enum_with_metadata<MetadataTagT, T>, can_apply<have_istream_operator_r, MetadataTagT, T>>;
template <typename MetadataTagT, typename T> constexpr bool have_istream_operator_v = have_istream_operator<MetadataTagT, T>::value;


template <typename EnumT, std::size_t N, typename SerializerT>
    void enum_to_stream_impl(EnumT value, std::ostream& stream, const enum_serialization_data<N>& sdata, const enum_serialization_options& options, SerializerT&& serializer)
{
    stream << streamable(to_string_impl(value, sdata, options), serializer);
}
template <typename EnumT, std::size_t N, typename SerializerT>
    void enum_from_stream_impl(EnumT& value, std::istream& stream, const enum_serialization_data<N>& sdata, const enum_serialization_options& options, SerializerT&& serializer)
{
    std::string str;
    stream >> streamable(str, serializer);
    value = from_string_impl(tag_v<EnumT>, str, sdata, options);
}

template <typename EnumT, std::size_t N, typename SerializerT>
    void enum_to_stream_impl(EnumT value, std::ostream& stream, const flags_enum_serialization_data<N>& sdata, const enum_serialization_options& options, SerializerT&& serializer)
{
    stream << streamable(to_string_impl(value, sdata, options), serializer);
}
template <typename EnumT, std::size_t N, typename SerializerT>
    void enum_from_stream_impl(EnumT& value, std::istream& stream, const flags_enum_serialization_data<N>& sdata, const enum_serialization_options& options, SerializerT&& serializer)
{
    std::string str;
    stream >> streamable(str, serializer);
    value = from_string_impl(tag_v<EnumT>, str, sdata, options);
}


template <typename ConstrainedIntT, typename SerializerT>
    void constrained_integer_to_stream_impl(ConstrainedIntT value, std::ostream& stream, SerializerT&& serializer)
{
    stream << streamable(value.value(), serializer);
}

template <typename ConstrainedIntT, typename SerializerT>
    void constrained_integer_from_stream_impl(ConstrainedIntT& value, std::istream& stream, SerializerT&& serializer)
{
    using Int = typename ConstrainedIntT::value_type;
    Int result;
    stream >> streamable(result, serializer);
    value = ConstrainedIntT::check(result);
}


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

    constexpr stream_serializer_options(void) noexcept = default;
    constexpr stream_serializer_options(enum_serialization_options _enum_options) noexcept : enum_options(_enum_options) { }
};


    //ᅟ
    // Stream serializer for enums with metadata and for types with overloaded stream operators.
    //
template <typename BaseT = void>
    struct stream_serializer : define_serializer<stream_serializer, BaseT, stream_serializer_options>
{
    using base = define_serializer<makeshift::stream_serializer, BaseT, stream_serializer_options>;
    using base::base;
    
    template <typename T, typename SerializerT/*,
              typename = std::enable_if_t<makeshift::detail::have_ostream_operator_v<metadata_tag_of_serializer_t<std::decay_t<SerializerT>>, std::decay_t<T>>>*/>
        friend std::enable_if_t<makeshift::detail::have_ostream_operator_v<metadata_tag_of_serializer_t<std::decay_t<SerializerT>>, std::decay_t<T>>>
        to_stream_impl(const T& value, std::ostream& stream, const stream_serializer& streamSerializer, SerializerT&& serializer)
    {
        (void) streamSerializer;
        (void) serializer;
        using D = std::decay_t<T>;
        if constexpr (std::is_enum<D>::value && have_metadata_v<D, metadata_tag_of_serializer_t<std::decay_t<SerializerT>>>)
            enum_to_stream_impl(value, stream, makeshift::detail::serialization_data<D, metadata_tag_of_serializer_t<std::decay_t<SerializerT>>>, streamSerializer.data.enum_options, serializer);
        else if constexpr (is_constrained_integer_v<D>)
            makeshift::detail::constrained_integer_to_stream_impl(value, stream, serializer);
        else
            stream << value;
    }
    template <typename T, typename SerializerT/*,
              typename = std::enable_if_t<makeshift::detail::have_istream_operator_v<metadata_tag_of_serializer_t<std::decay_t<SerializerT>>, std::decay_t<T>>>*/>
        friend std::enable_if_t<makeshift::detail::have_ostream_operator_v<metadata_tag_of_serializer_t<std::decay_t<SerializerT>>, std::decay_t<T>>>
        from_stream_impl(T& value, std::istream& stream, const stream_serializer& streamSerializer, SerializerT&& serializer)
    {
        (void) serializer;
        (void) streamSerializer;
        using D = std::decay_t<T>;
        if constexpr (std::is_enum<D>::value && have_metadata_v<D, metadata_tag_of_serializer_t<std::decay_t<SerializerT>>>)
            enum_from_stream_impl(value, stream, makeshift::detail::serialization_data<D, metadata_tag_of_serializer_t<std::decay_t<SerializerT>>>, streamSerializer.data.enum_options, serializer);
        else if constexpr (is_constrained_integer_v<D>)
            makeshift::detail::constrained_integer_from_stream_impl(value, stream, serializer);
        else
            stream >> value;
    }
};

    //ᅟ
    // Default stream serializer for enums with metadata and for types with overloaded stream operators.
    //
constexpr stream_serializer<> stream_serializer_v{ };
stream_serializer(void) -> stream_serializer<>;
stream_serializer(const stream_serializer_options&) -> stream_serializer<>;
stream_serializer(stream_serializer_options&&) -> stream_serializer<>;


    //ᅟ
    // Wraps the given rvalue as a streamable object using `stream_serializer_v`.
    //ᅟ
    //ᅟ    std::cout << streamable(vec.size()) << '\n';
    //
template <typename T>
    auto streamable(const T& value)
{
    return streamable(value, stream_serializer_v);
}


    //ᅟ
    // Wraps the given lvalue as a streamable object using `stream_serializer_v`.
    //ᅟ
    //ᅟ    int i;
    //ᅟ    std::cin >> streamable(i);
    //ᅟ    std::cout << streamable(i) << '\n';
    //
template <typename T>
    auto streamable(T& value)
{
    return streamable(value, stream_serializer_v);
}


} // inline namespace serialize

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_SERIALIZERS_STREAM_HPP_
