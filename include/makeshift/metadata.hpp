
#ifndef MAKESHIFT_METADATA_HPP_
#define MAKESHIFT_METADATA_HPP_


#include <string_view>
#include <type_traits> // for decay<>
#include <utility>     // for forward<>()
#include <cstddef>     // for size_t
#include <tuple>

#include <makeshift/type_traits.hpp> // for metadata_tag


namespace makeshift
{

namespace detail
{

template <typename T> struct literal_decay_ { using type = std::decay_t<T>; };
template <std::size_t N> struct literal_decay_<const char (&)[N]> { using type = std::string_view; };
template <typename T> using literal_decay_t = typename literal_decay_<T>::type;
    
template <typename T>
    constexpr literal_decay_t<T> literal_decay(T&& value)
{
    return std::forward<T>(value);
}

struct type_metadata_base { };
struct value_metadata_base { };
struct property_metadata_base { };

//static constexpr auto metadata_of { reflect((T*) nullptr, metadata_tag{ }) };

} // namespace detail


inline namespace metadata
{

    // Expose the ""sv literal.
using namespace std::literals::string_view_literals;


    // Tag type for reflect() methods which define type metadata.
struct default_metadata_tag { };


    // Use `type<T>(...)` to declare metadata for a type.
template <typename T, typename AttributesT>
    struct type_metadata : makeshift::detail::type_metadata_base
{
    using type = T;

    AttributesT attributes;

    constexpr type_metadata(AttributesT&& _attributes) : attributes(_attributes) { }
};
template <typename T, typename... AttrT>
    constexpr type_metadata<T, std::tuple<makeshift::detail::literal_decay_t<AttrT>...>> type(AttrT&&... attributes)
{
    return { std::make_tuple(makeshift::detail::literal_decay(std::forward<AttrT>(attributes))...) };
}


    // Use `value<V>(...)` to declare metadata for a known value of a type.
template <typename ValC, typename AttributesT>
    struct value_metadata : ValC, makeshift::detail::value_metadata_base
{
    using value = ValC;

    AttributesT attributes;

    constexpr value_metadata(AttributesT&& _attributes) : attributes(_attributes) { }
};
template <auto Val, typename... AttrT>
    constexpr value_metadata<std::integral_constant<decltype(Val), Val>, std::tuple<makeshift::detail::literal_decay_t<AttrT>...>> value(AttrT&&... attributes)
{
    return { std::make_tuple(makeshift::detail::literal_decay(std::forward<AttrT>(attributes))...) };
}


    // Use `property<Accessors...>(...)` to declare metadata for properties of a type.
template <typename AccessorsC, typename AttributesT>
    struct property_metadata : makeshift::detail::property_metadata_base
{
    using accessors = AccessorsC;

    AttributesT attributes;

    constexpr property_metadata(AttributesT&& _attributes) : attributes(_attributes) { }
};
template <auto... Accessors, typename... AttrT>
    constexpr property_metadata<type_sequence<std::integral_constant<decltype(Accessors), Accessors>...>, std::tuple<makeshift::detail::literal_decay_t<AttrT>...>> property(AttrT&&... attributes)
{
    return { std::make_tuple(makeshift::detail::literal_decay(std::forward<AttrT>(attributes))...) };
}


    // When defining metadata for flag enums, use `flags(type<TheFlagsType>(...))` to define metadata for the bitflag type itself.
template <typename TypeMetadataT>
    struct flags_t
{
    using value_type = TypeMetadataT;
    TypeMetadataT value;
};
template <typename TypeMetadataT,
          typename = std::enable_if_t<is_same_template_v<TypeMetadataT, type_metadata>>>
    constexpr flags_t<std::decay_t<TypeMetadataT>> flags(TypeMetadataT&& typeMetadata)
{
    return { std::forward<TypeMetadataT>(typeMetadata) };
}


    // Use `caption("the caption")` to encode a human-readable caption of an entity in metadata.
struct caption_t { std::string_view value; };
static inline constexpr caption_t caption(std::string_view value) { return { value }; }


    // Use `metadata_of<T[, MetadataTagT]>` to look up metadata for a type.
template <typename T, typename MetadataTagT = default_metadata_tag> static constexpr auto metadata_of { reflect((T*) nullptr, MetadataTagT{ }) };
template <typename T, typename MetadataTagT = default_metadata_tag> using metadata_of_t = decltype(metadata_of<T, MetadataTagT>);


    // Determines whether there is metadata for the given type.
template <typename T, typename MetadataTagT = default_metadata_tag> using have_metadata = can_apply<metadata_of_t, T, MetadataTagT>;
template <typename T, typename MetadataTagT = default_metadata_tag> constexpr bool have_metadata_v = have_metadata<T, MetadataTagT>::value;

} // inline namespace metadata

} // namespace makeshift

#endif // MAKESHIFT_METADATA_HPP_
