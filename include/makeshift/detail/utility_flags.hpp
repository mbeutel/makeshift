
#ifndef MAKESHIFT_DETAIL_UTILITY_FLAGS_HPP_
#define MAKESHIFT_DETAIL_UTILITY_FLAGS_HPP_


#include <makeshift/type_traits.hpp> // for flags_base


namespace makeshift
{

inline namespace types
{


    // Inherit from `define_flags<>` to define a flag enum type:
    // 
    //     struct Vegetable : define_flags<Vegetable>
    //     {
    //         static constexpr flag tomato { 1 };
    //         static constexpr flag onion { 2 };
    //         static constexpr flag eggplant { 4 };
    //         static constexpr flag garlic { 8 };
    //     };
    //     using Vegetables = Vegetable::flags;

template <typename FlagsT, typename UnderlyingTypeT = unsigned>
    struct define_flags : makeshift::detail::flags_base
{
    using base_type = UnderlyingTypeT;
    enum class flags : UnderlyingTypeT { none = 0 };
    using flag = flags; // alias for declaring flag constants

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
    friend constexpr bool hasFlag(flags _flags, flag _flag) noexcept { return (UnderlyingTypeT(_flags) & UnderlyingTypeT(_flag)) != 0; }
    friend constexpr bool hasAnyOf(flags _flags, flags desiredFlags) noexcept { return (UnderlyingTypeT(_flags) & UnderlyingTypeT(desiredFlags)) != 0; }
    friend constexpr bool hasAllOf(flags _flags, flags desiredFlags) noexcept { return flags(UnderlyingTypeT(_flags) & UnderlyingTypeT(desiredFlags)) == desiredFlags; }
};


} // inline namespace types

} // namespace makeshift

#endif // MAKESHIFT_DETAIL_UTILITY_FLAGS_HPP_
