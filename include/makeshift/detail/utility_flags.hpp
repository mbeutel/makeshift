
#ifndef INCLUDED_MAKESHIFT_DETAIL_UTILITY_FLAGS_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_UTILITY_FLAGS_HPP_


#include <makeshift/type_traits.hpp> // for flags_base, none, tag<>


namespace makeshift
{

namespace detail
{

namespace flag_enums
{


template <typename FlagsT, typename UnderlyingTypeT>
    struct define_flags_base : makeshift::detail::flags_base
{
    using base_type = UnderlyingTypeT;
    enum class flags : UnderlyingTypeT { none = 0 };
    using flag = flags; // alias for declaring flag constants

    friend constexpr tag<FlagsT> flag_type_of_(flags, makeshift::detail::flags_tag) { return { }; }

        // We just forward the metadata defined for the derived type.
        // TODO: ensure that have_metadata<flag> is false if no metadata is defined for FlagsT.
    template <typename MetadataTagT>
        friend constexpr auto reflect(tag<flag>, MetadataTagT) -> decltype(reflect(tag<FlagsT>{ }, MetadataTagT{ }))
    {
        return reflect(tag<FlagsT>{ }, MetadataTagT{ });
    }
};


template <typename EnumT> constexpr EnumT operator |(EnumT lhs, EnumT rhs) noexcept { return EnumT(std::underlying_type_t<EnumT>(lhs) | std::underlying_type_t<EnumT>(rhs)); }
template <typename EnumT> constexpr EnumT operator &(EnumT lhs, EnumT rhs) noexcept { return EnumT(std::underlying_type_t<EnumT>(lhs) & std::underlying_type_t<EnumT>(rhs)); }
template <typename EnumT> constexpr EnumT operator ^(EnumT lhs, EnumT rhs) noexcept { return EnumT(std::underlying_type_t<EnumT>(lhs) ^ std::underlying_type_t<EnumT>(rhs)); }
template <typename EnumT> constexpr EnumT operator ~(EnumT arg) noexcept { return EnumT(~std::underlying_type_t<EnumT>(arg)); }
template <typename EnumT> constexpr EnumT operator |=(EnumT& lhs, EnumT rhs) noexcept { lhs = lhs | rhs; return lhs; }
template <typename EnumT> constexpr EnumT operator &=(EnumT& lhs, EnumT rhs) noexcept { lhs = lhs & rhs; return lhs; }
template <typename EnumT> constexpr EnumT operator ^=(EnumT& lhs, EnumT rhs) noexcept { lhs = lhs ^ rhs; return lhs; }
template <typename EnumT> constexpr bool operator ==(EnumT lhs, none) noexcept { return lhs == EnumT(0); }
template <typename EnumT> constexpr bool operator ==(none, EnumT rhs) noexcept { return rhs == EnumT(0); }
template <typename EnumT> constexpr bool operator !=(EnumT lhs, none) noexcept { return lhs != EnumT(0); }
template <typename EnumT> constexpr bool operator !=(none, EnumT rhs) noexcept { return rhs != EnumT(0); }


    //ᅟ
    // `has_flag(haystack, needle)` determines whether the flags enum `haystack` contains the flag `needle`. Equivalent to `(haystack & needle) != none_v`.
    //
template <typename EnumT> constexpr bool has_flag(EnumT haystack, EnumT needle) noexcept { return (haystack & needle) != EnumT(0); }
    
    //ᅟ
    // `has_any_of(haystack, needles)` determines whether the flags enum `haystack` contains any of the flags in `needles`. Equivalent to `(haystack & needles) != none_v`.
    //
template <typename EnumT> constexpr bool has_any_of(EnumT haystack, EnumT needles) noexcept { return (haystack & needles) != EnumT(0); }
    
    //ᅟ
    // `has_all_of(haystack, needles)` determines whether the flags enum `haystack` contains all of the flags in `needles`. Equivalent to `(haystack & needles) == needles`.
    //
template <typename EnumT> constexpr bool has_all_of(EnumT haystack, EnumT needles) noexcept { return (haystack & needles) == needles; }


} // namespace flag_enums

} // namespace detail


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
    struct define_flags : makeshift::detail::flag_enums::define_flags_base<FlagsT, UnderlyingTypeT>
{
};


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_UTILITY_FLAGS_HPP_
