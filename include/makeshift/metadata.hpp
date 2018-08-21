
#ifndef INCLUDED_MAKESHIFT_METADATA_HPP_
#define INCLUDED_MAKESHIFT_METADATA_HPP_


#include <string_view>
#include <type_traits> // for decay<>, conditional<>
#include <utility>     // for forward<>()
#include <cstddef>     // for size_t
#include <tuple>

#include <makeshift/type_traits.hpp> // for can_apply<>


namespace makeshift
{

inline namespace types
{


    // Like `std::decay<>` but with additional support for converting plain old literals to modern types.
template <typename T> struct literal_decay { using type = std::decay_t<T>; };
template <std::size_t N> struct literal_decay<const char (&)[N]> { using type = std::string_view; };
template <typename T> using literal_decay_t = typename literal_decay<T>::type;
    

} // inline namespace types


namespace detail
{


struct type_metadata_base { };
struct value_metadata_base { };
struct property_metadata_base { };

template <typename SerializerT> struct serializer_metadata_tag_r { using type = typename SerializerT::metadata_tag; };
template <typename SerializerT> using serializer_metadata_tag_rt = typename SerializerT::metadata_tag;


} // namespace detail


inline namespace metadata
{


    // Expose the `""sv` literal.
using namespace std::literals::string_view_literals;


    //ᅟ
    // Stores metadata for a type.
    //
template <typename T, typename AttributesT>
    struct type_metadata : makeshift::detail::type_metadata_base
{
    using type = T;

    AttributesT attributes;

    constexpr type_metadata(AttributesT&& _attributes) : attributes(std::move(_attributes)) { }
};

    //ᅟ
    // Use `type<T>(...)` to declare metadata for a type.
    //
template <typename T, typename... AttrT>
    constexpr type_metadata<T, std::tuple<literal_decay_t<AttrT>...>> type(AttrT&&... attributes)
{
    return { std::tuple<literal_decay_t<AttrT>...>(std::forward<AttrT>(attributes)...) };
}


    //ᅟ
    // Stores metadata for a known value of a type.
    //
template <typename ValC, typename AttributesT>
    struct value_metadata : ValC, makeshift::detail::value_metadata_base
{
    using value_type = ValC;
    static constexpr ValC value { };

    AttributesT attributes;

    constexpr value_metadata(AttributesT&& _attributes) : attributes(std::move(_attributes)) { }
};

    //ᅟ
    // Use `value<V>(...)` to declare metadata for a known value of a type.
    //
template <auto Val, typename... AttrT>
    constexpr value_metadata<std::integral_constant<decltype(Val), Val>, std::tuple<literal_decay_t<AttrT>...>> value(AttrT&&... attributes)
{
    return { std::tuple<literal_decay_t<AttrT>...>(std::forward<AttrT>(attributes)...) };
}


    //ᅟ
    // Stores metadata for properties of a type.
    //
template <typename AccessorsC, typename AttributesT>
    struct property_metadata : makeshift::detail::property_metadata_base
{
    using accessors = AccessorsC;

    AttributesT attributes;

    constexpr property_metadata(AttributesT&& _attributes) : attributes(std::move(_attributes)) { }
};

    //ᅟ
    // Use `property<Accessors...>(...)` to declare metadata for properties of a type.
    //
template <auto... Accessors, typename... AttrT>
    constexpr property_metadata<type_sequence<std::integral_constant<decltype(Accessors), Accessors>...>, std::tuple<literal_decay_t<AttrT>...>> property(AttrT&&... attributes)
{
    return { std::tuple<literal_decay_t<AttrT>...>(std::forward<AttrT>(attributes)...) };
}


    //ᅟ
    // Stores metadata for the bitflag type of a flags enum.
    //
template <typename TypeMetadataT>
    struct flags_t
{
    using value_type = TypeMetadataT;
    TypeMetadataT value;
};

    //ᅟ
    // When defining metadata for flag enums, use `flags(type<TheFlagsType>(...))` to define metadata for the bitflag type itself.
    //
template <typename TypeMetadataT,
          typename = std::enable_if_t<is_same_template_v<TypeMetadataT, type_metadata>>>
    constexpr flags_t<std::decay_t<TypeMetadataT>> flags(TypeMetadataT&& typeMetadata)
{
    return { std::forward<TypeMetadataT>(typeMetadata) };
}


    //ᅟ
    // Encodes a human-readable caption of an entity in metadata.
    //
struct caption_t { std::string_view value; };

    //ᅟ
    // Use `caption("the caption")` to encode a human-readable caption of an entity in metadata.
    //
static inline constexpr caption_t caption(std::string_view value) { return { value }; }


    //ᅟ
    // Use `metadata_of<T, MetadataTagT>` to look up metadata for a type.
    //
template <typename T, typename MetadataTagT> static constexpr auto metadata_of = reflect((T*) nullptr, MetadataTagT{ });

    //ᅟ
    // Use `metadata_of<T, MetadataTagT>` to look up metadata for a type.
    //
template <typename T, typename MetadataTagT> using metadata_of_t = decltype(reflect((T*) nullptr, MetadataTagT{ }));


    //ᅟ
    // Determines whether there is metadata for the given type and the given tag.
    //
template <typename T, typename MetadataTagT> struct have_metadata : can_apply<metadata_of_t, T, MetadataTagT> { };

    //ᅟ
    // Determines whether there is metadata for the given type and the given tag.
    //
template <typename T, typename MetadataTagT> constexpr bool have_metadata_v = have_metadata<T, MetadataTagT>::value;


    //ᅟ
    // Default tag type for `reflect()` methods which define type metadata for serialization.
    //
struct serialization_metadata_tag { };


    //ᅟ
    // Base class for metadata-based serializers.
    //ᅟ
    // Use this class with `chain_serializers()` or `combine()` to inject a metadata tag into a metadata-based serializer.
    //
template <typename MetadataTagT = serialization_metadata_tag>
    struct metadata_serializer_t
{
    using metadata_tag = MetadataTagT;
};
template <typename MetadataTagT = serialization_metadata_tag> constexpr metadata_serializer_t<MetadataTagT> metadata_serializer { };


    //ᅟ
    // Retrieves the metadata tag to be used for metadata-based serializers.
    // Defaults to `serialization_metadata_tag` if the user did not override the tag by combining or chaining with a `metadata_serializer_t`.
    //
template <typename SerializerT> struct serializer_metadata_tag : std::conditional<can_apply_v<makeshift::detail::serializer_metadata_tag_rt, SerializerT>, makeshift::detail::serializer_metadata_tag_r<SerializerT>, serialization_metadata_tag> { };

    //ᅟ
    // Retrieves the metadata tag to be used for metadata-based serializers. Defaults to `serialization_metadata_tag` if the
    // user did not override the tag by combining or chaining with a `metadata_serializer_t`.
    //
template <typename SerializerT> using serializer_metadata_tag_t = typename serializer_metadata_tag<SerializerT>::type;


} // inline namespace metadata

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_METADATA_HPP_
