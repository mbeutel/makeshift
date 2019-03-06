
#ifndef INCLUDED_MAKESHIFT_UTILITY2_HPP_
#define INCLUDED_MAKESHIFT_UTILITY2_HPP_


#include <cstddef>     // for ptrdiff_t
#include <type_traits> // for underlying_type<>

#include <makeshift/type_traits.hpp> // for none

#include <makeshift/detail/utility2.hpp>


namespace makeshift
{

namespace detail
{

namespace adl
{


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
    // `has_flag(haystack, needle)` determines whether the flags enum `haystack` contains the flag `needle`. Equivalent to `(haystack & needle) != EnumT::none`.
    //
template <typename EnumT> constexpr bool has_flag(EnumT haystack, EnumT needle) noexcept { return (haystack & needle) != EnumT(0); }
    
    //ᅟ
    // `has_any_of(haystack, needles)` determines whether the flags enum `haystack` contains any of the flags in `needles`. Equivalent to `(haystack & needles) != EnumT::none`.
    //
template <typename EnumT> constexpr bool has_any_of(EnumT haystack, EnumT needles) noexcept { return (haystack & needles) != EnumT(0); }
    
    //ᅟ
    // `has_all_of(haystack, needles)` determines whether the flags enum `haystack` contains all of the flags in `needles`. Equivalent to `(haystack & needles) == needles`.
    //
template <typename EnumT> constexpr bool has_all_of(EnumT haystack, EnumT needles) noexcept { return (haystack & needles) == needles; }


} // namespace adl

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
    //ᅟ    };
    //ᅟ    using Vegetables = Vegetable::flags;
    //
template <typename FlagsT, typename UnderlyingTypeT = unsigned>
    struct define_flags : makeshift::detail::adl::define_flags_base<FlagsT, UnderlyingTypeT>
{
};


    //ᅟ
    // Represents an index value.
    //
using index2 = std::ptrdiff_t;


    //ᅟ
    // Represents a difference value.
    //
using diff2 = std::ptrdiff_t;


    //ᅟ
    // Represents an array stride.
    //
using stride2 = std::ptrdiff_t;


    //ᅟ
    // Represents a dimension value.
    //
using dim2 = std::ptrdiff_t;


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_UTILITY2_HPP_
