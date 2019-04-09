
#ifndef INCLUDED_MAKESHIFT_UTILITY2_HPP_
#define INCLUDED_MAKESHIFT_UTILITY2_HPP_


#include <cstddef>     // for ptrdiff_t
#include <utility>     // for move()
#include <type_traits> // for underlying_type<>, integral_constant<>

#include <gsl/gsl_assert> // for Expects()

#include <makeshift/version.hpp> // for MAKESHIFT_NODISCARD

#include <makeshift/detail/utility2.hpp>


namespace makeshift
{

namespace detail
{

namespace adl
{


template <typename EnumT> MAKESHIFT_NODISCARD constexpr EnumT operator |(EnumT lhs, EnumT rhs) noexcept { return EnumT(std::underlying_type_t<EnumT>(lhs) | std::underlying_type_t<EnumT>(rhs)); }
template <typename EnumT> MAKESHIFT_NODISCARD constexpr EnumT operator &(EnumT lhs, EnumT rhs) noexcept { return EnumT(std::underlying_type_t<EnumT>(lhs) & std::underlying_type_t<EnumT>(rhs)); }
template <typename EnumT> MAKESHIFT_NODISCARD constexpr EnumT operator ^(EnumT lhs, EnumT rhs) noexcept { return EnumT(std::underlying_type_t<EnumT>(lhs) ^ std::underlying_type_t<EnumT>(rhs)); }
template <typename EnumT> MAKESHIFT_NODISCARD constexpr EnumT operator ~(EnumT arg) noexcept { return EnumT(~std::underlying_type_t<EnumT>(arg)); }
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
using diff2 = std::ptrdiff_t; // TODO: remove?


    //ᅟ
    // Represents an array stride.
    //
using stride2 = std::ptrdiff_t;


    //ᅟ
    // Represents a dimension value.
    //
using dim2 = std::ptrdiff_t;


    //ᅟ
    // Represents an index value.
    //
template <index2 Value> using index2_c = std::integral_constant<index2, Value>;


    //ᅟ
    // Represents a dimension value.
    //
template <dim2 Value> using dim2_c = std::integral_constant<dim2, Value>;


    //ᅟ
    // Returns the size of an array or container.
    //
template <typename ContainerT> 
    MAKESHIFT_NODISCARD constexpr auto size(const ContainerT& c) -> decltype(c.size())
{
    return c.size();
}
template <typename T, std::size_t N>
    MAKESHIFT_NODISCARD constexpr std::size_t size(const T (&)[N]) noexcept
{
    return N;
}


    //ᅟ
    // Returns the signed size of an array or container.
    //
template <typename ContainerT>
    MAKESHIFT_NODISCARD constexpr auto ssize(const ContainerT& c)
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


    //ᅟ
    // Returns the size of an array or container as a constexpr value if known at compile time, or as a value if not.
    //
template <typename ContainerT>
    MAKESHIFT_NODISCARD constexpr auto csize(const ContainerT& c)
{
    return csize_impl(can_apply<makeshift::detail::has_constval_size_r, ContainerT>{ }, c);
}
template <typename T, std::size_t N>
    MAKESHIFT_NODISCARD constexpr std::integral_constant<std::size_t, N> csize(const T (&)[N]) noexcept
{
    return { };
}


    //ᅟ
    // Returns the signed size of an array or container as a constexpr value if known at compile time, or as a value if not.
    //
template <typename ContainerT>
    MAKESHIFT_NODISCARD constexpr auto cssize(const ContainerT& c)
{
    return cssize_impl(can_apply<makeshift::detail::has_constval_size_r, ContainerT>{ }, c);
}
template <typename T, std::ptrdiff_t N>
    MAKESHIFT_NODISCARD constexpr std::integral_constant<std::ptrdiff_t, N> cssize(const T (&)[N]) noexcept
{
    return { };
}


    //ᅟ
    // Represents a pair of iterators.
    //
template <typename It, typename EndIt = It>
    struct range : makeshift::detail::range_base_<range<It, EndIt>>::type
{
    using iterator = It;
    using end_iterator = EndIt;

    It first;
    EndIt last;

    constexpr range(It _first, EndIt _last)
        : first(std::move(_first)), last(std::move(_last))
    {
    }

    MAKESHIFT_NODISCARD constexpr const It& begin(void) const noexcept { return first; }
    MAKESHIFT_NODISCARD constexpr const EndIt& end(void) const noexcept { return last; }
};

    //ᅟ
    // Construct a range from a pair of iterators.
    //
template <typename It, typename EndIt,
          typename = std::enable_if_t<!std::is_integral<EndIt>::value>>
    range<It, EndIt> make_range(It first, EndIt last)
{
    return { std::move(first), std::move(last) };
}

    //ᅟ
    // Construct a range from an iterator and an extent.
    //
template <typename It>
    range<It, It> make_range(It start, dim2 extent)
{
    return { start, start + extent };
}

    //ᅟ
    // Construct a range from another range (e.g. a container).
    // TODO: why would we want to do that? to take range args by value?
    //
template <typename ContainerT>
    auto make_range(ContainerT&& container)
{
    return makeshift::make_range(std::forward<ContainerT>(container).begin(), std::forward<ContainerT>(container).end());
}


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_UTILITY2_HPP_
