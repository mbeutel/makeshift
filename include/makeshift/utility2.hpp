
#ifndef INCLUDED_MAKESHIFT_UTILITY2_HPP_
#define INCLUDED_MAKESHIFT_UTILITY2_HPP_


#include <cstddef>     // for ptrdiff_t
#include <type_traits> // for underlying_type<>

#include <gsl/gsl_assert> // for Expects()

#include <makeshift/version.hpp> // for MAKESHIFT_NODISCARD

#include <makeshift/detail/utility2.hpp>


namespace makeshift
{

namespace detail
{

namespace adl
{


template <typename EnumT> constexpr MAKESHIFT_NODISCARD EnumT operator |(EnumT lhs, EnumT rhs) noexcept { return EnumT(std::underlying_type_t<EnumT>(lhs) | std::underlying_type_t<EnumT>(rhs)); }
template <typename EnumT> constexpr MAKESHIFT_NODISCARD EnumT operator &(EnumT lhs, EnumT rhs) noexcept { return EnumT(std::underlying_type_t<EnumT>(lhs) & std::underlying_type_t<EnumT>(rhs)); }
template <typename EnumT> constexpr MAKESHIFT_NODISCARD EnumT operator ^(EnumT lhs, EnumT rhs) noexcept { return EnumT(std::underlying_type_t<EnumT>(lhs) ^ std::underlying_type_t<EnumT>(rhs)); }
template <typename EnumT> constexpr MAKESHIFT_NODISCARD EnumT operator ~(EnumT arg) noexcept { return EnumT(~std::underlying_type_t<EnumT>(arg)); }
template <typename EnumT> constexpr EnumT operator |=(EnumT& lhs, EnumT rhs) noexcept { lhs = lhs | rhs; return lhs; }
template <typename EnumT> constexpr EnumT operator &=(EnumT& lhs, EnumT rhs) noexcept { lhs = lhs & rhs; return lhs; }
template <typename EnumT> constexpr EnumT operator ^=(EnumT& lhs, EnumT rhs) noexcept { lhs = lhs ^ rhs; return lhs; }


    //ᅟ
    // `has_flag(haystack, needle)` determines whether the flags enum `haystack` contains the flag `needle`. Equivalent to `(haystack & needle) != EnumT::none`.
    //
template <typename EnumT> MAKESHIFT_NODISCARD constexpr bool has_flag(EnumT haystack, EnumT needle) noexcept { return (haystack & needle) != EnumT(0); }
    
    //ᅟ
    // `has_any_of(haystack, needles)` determines whether the flags enum `haystack` contains any of the flags in `needles`. Equivalent to `(haystack & needles) != EnumT::none`.
    //
template <typename EnumT> MAKESHIFT_NODISCARD constexpr bool has_any_of(EnumT haystack, EnumT needles) noexcept { return (haystack & needles) != EnumT(0); }
    
    //ᅟ
    // `has_all_of(haystack, needles)` determines whether the flags enum `haystack` contains all of the flags in `needles`. Equivalent to `(haystack & needles) == needles`.
    //
template <typename EnumT> MAKESHIFT_NODISCARD constexpr bool has_all_of(EnumT haystack, EnumT needles) noexcept { return (haystack & needles) == needles; }


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
    // Inherit from `define_type_enum<>` to define a type enumeration:
    //ᅟ
    //ᅟ    struct FloatTypes : define_type_enum<FloatTypes, float, double> { using base::base; };
    //ᅟ
    //ᅟ    FloatTypes floatType = ...;
    //ᅟ    switch (floatType)
    //ᅟ    {
    //ᅟ    case type_v<float>:  ...; break;
    //ᅟ    case type_v<double>: ...; break;
    //ᅟ    }
    //
template <typename TypeEnumT, typename... Ts>
    struct define_type_enum : makeshift::detail::adl::define_type_enum_base<TypeEnumT, Ts...>
{
    using _base_base = makeshift::detail::adl::define_type_enum_base<TypeEnumT, Ts...>;
    using _base_base::_base_base;
    using base = define_type_enum;
};


    //ᅟ
    // Anonymous type enumeration.
    //ᅟ
    //ᅟ    using FloatTypes = type_enum<float, double>;
    //ᅟ
    //ᅟ    FloatTypes floatType = ...;
    //ᅟ    switch (floatType)
    //ᅟ    {
    //ᅟ    case type_v<float>:  ...; break;
    //ᅟ    case type_v<double>: ...; break;
    //ᅟ    }
    //
template <typename... Ts>
    class type_enum final : public define_type_enum<type_enum<Ts...>, Ts...>
{
    using _base_base = define_type_enum<type_enum<Ts...>, Ts...>;
    using _base_base::_base_base;
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


    //ᅟ
    // Returns the size of an array, container, or range.
    //
template <typename C> 
    MAKESHIFT_NODISCARD constexpr auto size(const C& c) -> decltype(c.size())
{
    return c.size();
}
template <typename T, std::size_t N>
    MAKESHIFT_NODISCARD constexpr std::size_t size(const T (&)[N]) noexcept
{
    return N;
}


    //ᅟ
    // Returns the size of an array, container, or range, as a *signed* value.
    //
template <typename C>
    MAKESHIFT_NODISCARD constexpr auto ssize(const C& c)
        -> std::common_type_t<std::ptrdiff_t, std::make_signed_t<decltype(c.size())>>
{
    using R = std::common_type_t<std::ptrdiff_t, std::make_signed_t<decltype(c.size())>>;
    return static_cast<R>(c.size());
}
template <typename T, std::ptrdiff_t N>
    MAKESHIFT_NODISCARD constexpr std::ptrdiff_t ssize(const T (&)[N]) noexcept
{
    return N;
}


template <dim2 Extent>
    class extent : std::integral_constant<dim2, Extent>
{
public:
    constexpr extent(void) = default;
    constexpr extent(dim2 _extent)
        : extent_(_extent)
    {
        Expects(_extent == Extent);
    }
};
template <>
    class extent<-1>
{
private:
    dim2 extent_;

public:
    using value_type = dim2;
    using type       = extent;

    constexpr extent(dim2 _extent)
        : extent_(_extent)
    {
        Expects(extent_ >= 0);
    }

    MAKESHIFT_NODISCARD constexpr operator dim2(void) const noexcept { return extent_; }
    MAKESHIFT_NODISCARD constexpr dim2 operator ()(void) const noexcept { return extent_; }
};


    //ᅟ
    // Returns the result of the function applied to the extent's value as an extent.
    //ᅟ
    //ᅟ    auto baseIndexR = []{ return 42; };
    //ᅟ    auto offsetR = []{ return 3; };
    //ᅟ    auto indexR = constexpr_transform(std::plus<>, baseIndexR, offsetR); // equivalent to `[]{ return 45; }`
    //
template <typename F, typename... Rs>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::constexpr_transform_functor<F, Rs...>
    constexpr_transform(const F&, const Rs&...) noexcept
{
    static_assert(std::is_empty<F>::value, "transformer must be stateless");
    static_assert(makeshift::detail::cand(is_constexpr_value_v<Rs>...), "arguments must be constexpr values");
    return { };
}



} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_UTILITY2_HPP_
