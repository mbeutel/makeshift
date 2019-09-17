
#ifndef INCLUDED_MAKESHIFT_ENUM_HPP_
#define INCLUDED_MAKESHIFT_ENUM_HPP_


#include <type_traits> // for enable_if<>

#include <gsl/gsl-lite.hpp> // for Expects()

#include <makeshift/type_traits.hpp> // for underlying_type<>, is_bitmask_type<>
#include <makeshift/macros.hpp>      // for MAKESHIFT_NODISCARD

#include <makeshift/detail/enum.hpp>


    //ᅟ
    // Defines bitmask operators `|`, `&`, `^`, `~`, `|=`, `&=`, and `^=` for the given enum type.
    //ᅟ
    //ᅟ    enum class Vegetables
    //ᅟ    {
    //ᅟ        tomato   = 0b001,
    //ᅟ        onion    = 0b010,
    //ᅟ        eggplant = 0b100
    //ᅟ    };
    //ᅟ    MAKESHIFT_DEFINE_BITMASK_OPERATORS(Vegetables)
    //
#define MAKESHIFT_DEFINE_BITMASK_OPERATORS(ENUM) MAKESHIFT_DEFINE_BITMASK_OPERATORS_(ENUM)


namespace makeshift
{


    //ᅟ
    // `has_flag(haystack, needle)` determines whether the bitmask `haystack` contains the flag `needle`.
    // Equivalent to `(haystack & needle) == needle`.
    //
template <typename EnumT>
    MAKESHIFT_NODISCARD constexpr bool
    has_flag(EnumT haystack, EnumT needle)
{
    static_assert(is_bitmask_type_v<EnumT>, "arguments must be of bitmask type");
    Expects(makeshift::detail::is_flag_power_of_2(std::underlying_type_t<EnumT>(needle)));
    return (haystack & needle) == needle;
}
    
    //ᅟ
    // `has_any_flag_of(haystack, needles)` determines whether the bitmask `haystack` contains any of the flags in `needles`.
    // Equivalent to `(haystack & needles) != { }`.
    //
template <typename EnumT>
    MAKESHIFT_NODISCARD constexpr bool
    has_any_flag_of(EnumT haystack, EnumT needles)
{
    static_assert(is_bitmask_type_v<EnumT>, "arguments must be of bitmask type");
    return (haystack & needles) != EnumT{ };
}

    //ᅟ
    // `has_all_flags_of(haystack, needles)` determines whether the bitmask `haystack` contains all of the flags in `needles`.
    // Equivalent to `(haystack & needles) == needles`.
    //
template <typename EnumT>
    MAKESHIFT_NODISCARD constexpr bool
    has_all_flags_of(EnumT haystack, EnumT needles)
{
    static_assert(is_bitmask_type_v<EnumT>, "arguments must be of bitmask type");
    return (haystack & needles) == needles;
}


    //ᅟ
    // Inherit from `define_type_enum<>` to define a named type enumeration:
    //ᅟ
    //ᅟ    struct FloatTypes : define_type_enum<FloatTypes, float, double> { using base::base; };
    //ᅟ
    //ᅟ    FloatTypes floatType = ...;
    //ᅟ    switch (floatType)
    //ᅟ    {
    //ᅟ    case type_c<float>:  ...; break;
    //ᅟ    case type_c<double>: ...; break;
    //ᅟ    }
    //
template <typename TypeEnumT, typename... Ts>
    struct define_type_enum : makeshift::detail::define_type_enum_base<TypeEnumT, Ts...>
{
    using _base_base = makeshift::detail::define_type_enum_base<TypeEnumT, Ts...>;
    using _base_base::_base_base;
    using base = define_type_enum;
};

template <typename TypeEnumT>
    struct underlying_type<TypeEnumT, std::enable_if_t<std::is_base_of<makeshift::detail::type_enum_base, TypeEnumT>::value>>
{
    using type = typename TypeEnumT::underlying_type;
};


    //ᅟ
    // Anonymous type enumeration.
    //ᅟ
    //ᅟ    using FloatTypes = type_enum<float, double>;
    //ᅟ
    //ᅟ    FloatTypes floatType = ...;
    //ᅟ    switch (floatType)
    //ᅟ    {
    //ᅟ    case type_c<float>:  ...; break;
    //ᅟ    case type_c<double>: ...; break;
    //ᅟ    }
    //
template <typename... Ts>
    class type_enum final : public define_type_enum<type_enum<Ts...>, Ts...>
{
    using _base_base = define_type_enum<type_enum<Ts...>, Ts...>;
    using _base_base::_base_base;
};


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_ENUM_HPP_
