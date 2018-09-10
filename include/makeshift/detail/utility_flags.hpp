
#ifndef INCLUDED_MAKESHIFT_DETAIL_UTILITY_FLAGS_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_UTILITY_FLAGS_HPP_


#include <makeshift/type_traits.hpp> // for flags_base, none, tag<>


namespace makeshift
{

inline namespace types
{


    //ᅟ
    // Inherit from `define_flags<>` to define a flag enum type:
    //ᅟ
    //ᅟ    struct Vegetable : define_flags<Vegetable>
    //ᅟ    {
    //ᅟ        static constexpr flag tomato { 1 };
    //ᅟ        static constexpr flag onion { 2 };
    //ᅟ        static constexpr flag eggplant { 4 };
    //ᅟ        static constexpr flag garlic { 8 };
    //ᅟ    };
    //ᅟ    using Vegetables = Vegetable::flags;
    //
template <typename FlagsT, typename UnderlyingTypeT = unsigned>
    struct define_flags : makeshift::detail::flags_base
{
    using base_type = UnderlyingTypeT;
    enum class flags : UnderlyingTypeT { none = 0 };
    using flag = flags; // alias for declaring flag constants

    friend constexpr tag<FlagsT> flag_type_of_(flags, makeshift::detail::flags_tag) { return { }; }

        // We just forward the metadata defined for the derived type.
        // TODO: ensure that have_metadata<flag> is false if no metadata is defined for FlagsT.
    template <typename MetadataTagT>
        friend constexpr auto reflect(flag*, MetadataTagT) -> decltype(reflect((FlagsT*) nullptr, MetadataTagT{ }))
    {
        return reflect((FlagsT*) nullptr, MetadataTagT{ });
    }

    friend constexpr flags operator |(flags lhs, flags rhs) noexcept { return flags(UnderlyingTypeT(lhs) | UnderlyingTypeT(rhs)); }
    friend constexpr flags operator &(flags lhs, flags rhs) noexcept { return flags(UnderlyingTypeT(lhs) & UnderlyingTypeT(rhs)); }
    friend constexpr flags operator ^(flags lhs, flags rhs) noexcept { return flags(UnderlyingTypeT(lhs) ^ UnderlyingTypeT(rhs)); }
    friend constexpr flags operator ~(flags arg) noexcept { return flags(~UnderlyingTypeT(arg)); }
    friend constexpr flags& operator |=(flags& lhs, flags rhs) noexcept { lhs = lhs | rhs; return lhs; }
    friend constexpr flags& operator &=(flags& lhs, flags rhs) noexcept { lhs = lhs & rhs; return lhs; }
    friend constexpr flags& operator ^=(flags& lhs, flags rhs) noexcept { lhs = lhs ^ rhs; return lhs; }
    friend constexpr bool operator ==(flags lhs, none_t) noexcept { return lhs == flags::none; }
    friend constexpr bool operator ==(none_t, flags rhs) noexcept { return rhs == flags::none; }
    friend constexpr bool operator !=(flags lhs, none_t) noexcept { return lhs != flags::none; }
    friend constexpr bool operator !=(none_t, flags rhs) noexcept { return rhs != flags::none; }

        //ᅟ
        // `has_flag(haystack, needle)` determines whether the flags enum `haystack` contains the flag `needle`. Equivalent to `(haystack & needle) != none`.
        //
    friend constexpr bool has_flag(flags haystack, flag needle) noexcept { return (UnderlyingTypeT(haystack) & UnderlyingTypeT(needle)) != 0; }
    
        //ᅟ
        // `has_any_of(haystack, needles)` determines whether the flags enum `haystack` contains any of the flags in `needles`. Equivalent to `(haystack & needles) != none`.
        //
    friend constexpr bool has_any_of(flags haystack, flags needles) noexcept { return (UnderlyingTypeT(haystack) & UnderlyingTypeT(needles)) != 0; }
    
        //ᅟ
        // `has_all_of(haystack, needles)` determines whether the flags enum `haystack` contains all of the flags in `needles`. Equivalent to `(haystack & needles) == needles`.
        //
    friend constexpr bool has_all_of(flags haystack, flags needles) noexcept { return flags(UnderlyingTypeT(haystack) & UnderlyingTypeT(needles)) == needles; }
};


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_UTILITY_FLAGS_HPP_
