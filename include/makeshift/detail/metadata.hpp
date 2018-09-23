﻿
#ifndef INCLUDED_MAKESHIFT_DETAIL_METADATA_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_METADATA_HPP_


#include <makeshift/detail/utility_flags.hpp>


namespace makeshift
{

inline namespace metadata
{


    //ᅟ
    // Use `metadata_of<T, MetadataTagT>` to look up metadata for a type.
    //
template <typename T, typename MetadataTagT> static constexpr auto metadata_of = reflect(tag<T>{ }, MetadataTagT{ });

    //ᅟ
    // Use `metadata_of<T, MetadataTagT>` to look up metadata for a type.
    //
template <typename T, typename MetadataTagT> using metadata_of_t = decltype(reflect(tag<T>{ }, MetadataTagT{ }));


    //ᅟ
    // Determines whether there is metadata for the given type and the given tag.
    //
template <typename T, typename MetadataTagT> struct have_metadata : can_apply<metadata_of_t, T, MetadataTagT> { };

    //ᅟ
    // Determines whether there is metadata for the given type and the given tag.
    //
template <typename T, typename MetadataTagT> constexpr bool have_metadata_v = have_metadata<T, MetadataTagT>::value;


    //ᅟ
    // Determines qualities of a type.
    //
struct type_flag : define_flags<type_flag>
{
        //ᅟ
        // Indicates that the associated type is a value type (either a scalar type or a user-defined wrapper).
        //
    static constexpr flag value { 1 };

        //ᅟ
        // Indicates that the associated type is a compound type, i.e. it has the semantics of a named tuple with regard to identity and comparability.
        // This type flag does not necessarily require aggregate-ness as defined in the C++ standard (`std::is_aggregate<>`), which imposes
        // unnecessary limitations (e.g. it makes sense for an aggregate to have a user-defined constructor).
        //
    static constexpr flag compound { 2 };

        //ᅟ
        // Indicates that the associated type is a composite type which itself forms a value, e.g. a geometrical point defined as `struct Point { int x, y; };`.
        //
    static constexpr flag compound_value { 1 | 2 };
};
using type_flags = type_flag::flags;


} // inline namespace metadata


namespace detail
{


template <typename T>
    constexpr type_flags default_type_flags = std::conditional_t<std::disjunction<std::is_scalar<T>, is_constrained_integer<T>>::value, constant<type_flag::value>, constant<type_flags::none>>::value;

template <typename T, typename MetadataTagT>
    constexpr type_flags lookup_type_flags(void) noexcept
{
    if constexpr (have_metadata_v<T, MetadataTagT>)
        return metadata_of_t<T, MetadataTagT>::flags;
    else
        return default_type_flags<T>;
}


template <typename ChainableT> struct metadata_tag_r { using type = typename ChainableT::metadata_tag; };
template <typename ChainableT> using metadata_tag_rt = typename ChainableT::metadata_tag;


} // namespace detail


inline namespace metadata
{


    //ᅟ
    // Determines qualities of a type.
    //
template <typename T, typename MetadataTagT, typename = void> constexpr type_flags type_flags_of = makeshift::detail::lookup_type_flags<T, MetadataTagT>();


} // inline namespace metadata

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_METADATA_HPP_