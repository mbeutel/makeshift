
#ifndef MAKESHIFT_METADATA_HPP_
#define MAKESHIFT_METADATA_HPP_


#include <string_view>
#include <type_traits> // for decay_t<>
#include <utility>     // for forward<>()
#include <cstddef>     // for size_t
#include <tuple>

#include <makeshift/types.hpp> // for tag<>


namespace makeshift
{

namespace detail
{

template <typename T>
    struct literal_decay_
{
    using type = std::decay_t<T>;
};
template <std::size_t N>
    struct literal_decay_<const char (&)[N]>
{
    using type = std::string_view;
};
template <typename T>
    using literal_decay_t = typename literal_decay_<T>::type;
    
template <typename T>
    constexpr literal_decay_t<T> literal_decay(T&& value)
{
    return std::forward<T>(value);
}

struct type_metadata_base { };
struct value_metadata_base { };
struct property_metadata_base { };

} // namespace detail


inline namespace metadata
{

template <typename T, typename AttributesT>
    struct type_metadata : makeshift::detail::type_metadata_base
{
    using type = T;

    AttributesT attributes;

    constexpr type_metadata(AttributesT&& _attributes)
    noexcept(noexcept(AttributesT(std::move(_attributes))))
        : attributes(_attributes)
    {
    }
};
template <typename T, typename... AttrT>
    constexpr type_metadata<T, std::tuple<makeshift::detail::literal_decay_t<AttrT>...>> type(AttrT&&... attributes)
{
    return { std::make_tuple(makeshift::detail::literal_decay(std::forward<AttrT>(attributes))...) };
}

template <typename ValC, typename AttributesT>
    struct value_metadata : ValC, makeshift::detail::value_metadata_base
{
    AttributesT attributes;

    constexpr value_metadata(AttributesT&& _attributes)
    noexcept(noexcept(AttributesT(std::move(_attributes))))
        : attributes(_attributes)
    {
    }
};
template <auto Val, typename... AttrT>
    constexpr value_metadata<std::integral_constant<decltype(Val), Val>, std::tuple<makeshift::detail::literal_decay_t<AttrT>...>> value(AttrT&&... attributes)
{
    return { std::make_tuple(makeshift::detail::literal_decay(std::forward<AttrT>(attributes))...) };
}

/*template <typename AttributesT, auto... Accessors>
    struct property_metadata : makeshift::detail::property_metadata_base
{
    static inline constexpr std::tuple<decltype(Accessors)...> accessors { Accessors... };

    AttributesT attributes;

    constexpr property_metadata(AttributesT&& _attributes)
    noexcept(noexcept(AttributesT(std::move(_attributes))))
        : attributes(_attributes)
    {
    }
};
template <auto... Accessors, typename... AttrT>
    constexpr property_metadata<std::tuple<makeshift::detail::literal_decay_t<AttrT>...>, Accessors...> property(AttrT&&... attributes)
{
    return { std::make_tuple(makeshift::detail::literal_decay(std::forward<AttrT>(attributes))...) };
}*/

    // Use `flags(type<>` to mark an enum type as a bitflag type in metadata.
//struct flags_t { };
//static inline constexpr flags_t flags { };

template <typename TypeMetadataT>
    struct flags_t
{
    using value_type = TypeMetadataT;
    TypeMetadataT value;
};
template <typename TypeMetadataT,
          typename = std::enable_if_t<is_same_template<TypeMetadataT, type_metadata>>>
    constexpr flags_t<std::decay_t<TypeMetadataT>> flags(TypeMetadataT&& typeMetadata)
{
    return { std::forward<TypeMetadataT>(typeMetadata) };
}


    // Use `description("the description")` to encode a human-readable description of an entity in metadata.
struct description_t { std::string_view value; };
static inline constexpr description_t description(std::string_view value) { return { value }; }

    // Use `metadata_of<T>` to look up metadata for a type.
template <typename T>
    static constexpr auto metadata_of { reflect((T*) nullptr, tag<>) };

} // inline namespace metadata

} // namespace makeshift

#endif // MAKESHIFT_METADATA_HPP

