
#ifndef INCLUDED_MAKESHIFT_EXPERIMENTAL_ENUM_HPP_
#define INCLUDED_MAKESHIFT_EXPERIMENTAL_ENUM_HPP_


#include <type_traits>  // for underlying_type<>

#include <gsl-lite/gsl-lite.hpp>  // for gsl_ExpectsAudit(), gsl_CPP17_OR_GREATER

#if !gsl_CPP17_OR_GREATER
# error makeshift requires C++17 mode or higher
#endif // !gsl_CPP17_OR_GREATER

#include <makeshift/type_traits.hpp> // for is_bitmask<>

#include <makeshift/experimental/detail/enum.hpp>


namespace makeshift {


    //
    // `has_flag<needle>(haystack)` determines whether the bitmask `haystack` contains the flag `needle`.
    //ᅟ
    // Equivalent to `(haystack & needle) == needle`.
    //
template <auto Needle>
[[nodiscard]] constexpr bool
has_flag(decltype(Needle) haystack)
{
    using Enum = decltype(Needle);
    static_assert(is_bitmask_v<Enum>, "arguments must be of bitmask type");
    static_assert(detail::is_flag_power_of_2(std::underlying_type_t<Enum>(Needle)), "argument must be a single flag value");

    return (haystack & Needle) == Needle;
}

    //
    // `has_any_flag_of<needles>(haystack)` determines whether the bitmask `haystack` contains any of the flags in `needles`.
    //ᅟ
    // Equivalent to `(haystack & needles) != decltype(needles){ }`.
    //
template <auto Needles>
[[nodiscard]] constexpr bool
has_any_flag_of(decltype(Needles) haystack)
{
    using Enum = decltype(Needles);
    static_assert(is_bitmask_v<Enum>, "arguments must be of bitmask type");

    return (haystack & Needles) != Enum{ };
}

    //
    // `has_no_flag_of<needles>(haystack)` determines whether the bitmask `haystack` contains none of the flags in `needles`.
    //ᅟ
    // Equivalent to `(haystack & needles) == decltype(needles){ }`.
    //
template <auto Needles>
[[nodiscard]] constexpr bool
has_no_flag_of(decltype(Needles) haystack)
{
    using Enum = decltype(Needles);
    static_assert(is_bitmask_v<Enum>, "arguments must be of bitmask type");

    return (haystack & Needles) == Enum{ };
}

    //
    // `has_all_flags_of<needles>(haystack)` determines whether the bitmask `haystack` contains all of the flags in `needles`.
    //ᅟ
    // Equivalent to `(haystack & needles) == needles`.
    //
template <auto Needles>
[[nodiscard]] constexpr bool
has_all_flags_of(decltype(Needles) haystack)
{
    using Enum = decltype(Needles);
    static_assert(is_bitmask_v<Enum>, "arguments must be of bitmask type");

    return (haystack & Needles) == Needles;
}

    //
    // `has_flag(haystack, needle)` determines whether the bitmask `haystack` contains the flag `needle`.
    //ᅟ
    // Equivalent to `(haystack & needle) == needle`.
    //
template <typename EnumT>
[[nodiscard]] constexpr bool
has_flag(EnumT haystack, EnumT needle)
{
    static_assert(is_bitmask_v<EnumT>, "arguments must be of bitmask type");
    gsl_ExpectsAudit(detail::is_flag_power_of_2(std::underlying_type_t<EnumT>(needle)));
    return (haystack & needle) == needle;
}

    //
    // `has_any_flag_of(haystack, needles)` determines whether the bitmask `haystack` contains any of the flags in `needles`.
    //ᅟ
    // Equivalent to `(haystack & needles) != EnumT{ }`.
    //
template <typename EnumT>
[[nodiscard]] constexpr bool
has_any_flag_of(EnumT haystack, EnumT needles)
{
    static_assert(is_bitmask_v<EnumT>, "arguments must be of bitmask type");
    return (haystack & needles) != EnumT{ };
}

    //
    // `has_no_flag_of(haystack, needles)` determines whether the bitmask `haystack` contains none of the flags in `needles`.
    //ᅟ
    // Equivalent to `(haystack & needles) == EnumT{ }`.
    //
template <typename EnumT>
[[nodiscard]] constexpr bool
has_no_flag_of(EnumT haystack, EnumT needles)
{
    static_assert(is_bitmask_v<EnumT>, "arguments must be of bitmask type");
    return (haystack & needles) == EnumT{ };
}

    //
    // `has_all_flags_of(haystack, needles)` determines whether the bitmask `haystack` contains all of the flags in `needles`.
    //ᅟ
    // Equivalent to `(haystack & needles) == needles`.
    //
template <typename EnumT>
[[nodiscard]] constexpr bool
has_all_flags_of(EnumT haystack, EnumT needles)
{
    static_assert(is_bitmask_v<EnumT>, "arguments must be of bitmask type");
    return (haystack & needles) == needles;
}


    //
    // Inherit from `define_type_enum<>` to define a named type enumeration.
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
struct define_type_enum : detail::define_type_enum_base<TypeEnumT, Ts...>
{
    using _base_base = detail::define_type_enum_base<TypeEnumT, Ts...>;
    using _base_base::_base_base;
    using base = define_type_enum;
};


    //
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


#endif // INCLUDED_MAKESHIFT_EXPERIMENTAL_ENUM_HPP_
