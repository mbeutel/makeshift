
#ifndef INCLUDED_MAKESHIFT_DETAIL_VARIANT_REFLECT_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_VARIANT_REFLECT_HPP_


#include <utility>     // for move(), forward<>()
#include <type_traits> // for decay<>

#include <makeshift/reflect.hpp> // for values_of<>()
#include <makeshift/variant.hpp> // for expand()


namespace makeshift
{

inline namespace types
{


    //ᅟ
    // Given a runtime value, returns a variant of the type-encoded possible values as defined by metadata.
    //ᅟ
    //ᅟ    enum Color { red, green, blue };
    //ᅟ    constexpr inline auto reflect(tag<Color>, reflection_tag) noexcept { ... }
    //ᅟ
    //ᅟ    Color runtimeColor = ...;
    //ᅟ    auto color = expand(runtimeColor); // returns std::variant<constant<red>, constant<green>, constant<blue>>
    //
template <typename T, typename MetadataTagT = reflection_tag>
    constexpr auto
    expand(T&& value, tag<MetadataTagT> = { })
{
    /*constexpr*/ auto tuple = values_of(tag_v<std::decay_t<T>>, MetadataTagT{ }); // TODO: currently not constexpr due to VC++ ICE
    return expand(std::forward<T>(value), std::move(tuple));
}


    //ᅟ
    // Given a runtime value, returns a variant of `unknown_value<>` and the type-encoded possible values as defined by metadata.
    // The variant holds `unknown_value<>` if the runtime value does not appear in the tuple of possible values.
    //ᅟ
    //ᅟ    enum Color { red, green, blue };
    //ᅟ    constexpr inline auto reflect(tag<Color>, reflection_tag) noexcept { ... }
    //ᅟ
    //ᅟ    Color runtimeColor = ...;
    //ᅟ    auto color = try_expand(runtimeColor); // returns std::variant<unknown_value<Color>, constant<red>, constant<green>, constant<blue>>
    //
template <typename T, typename MetadataTagT = reflection_tag>
    constexpr auto
    try_expand(T&& value, tag<MetadataTagT> = { })
{
    /*constexpr*/ auto tuple = values_of(tag_v<std::decay_t<T>>, MetadataTagT{ }); // TODO: currently not constexpr due to VC++ ICE
    return try_expand(std::forward<T>(value), std::move(tuple));
}


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_VARIANT_REFLECT_HPP_
