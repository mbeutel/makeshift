
#ifndef INCLUDED_MAKESHIFT_UTILITY2_HPP_
#define INCLUDED_MAKESHIFT_UTILITY2_HPP_


#include <array>
#include <tuple>
#include <cstddef>     // for ptrdiff_t
#include <utility>     // for move()
#include <type_traits> // for underlying_type<>, integral_constant<>

#include <gsl/gsl_assert> // for Expects()

#include <makeshift/version.hpp> // for MAKESHIFT_NODISCARD

#include <makeshift/detail/workaround.hpp>   // for cand()
#include <makeshift/detail/type_traits2.hpp> // for constval_tag
#include <makeshift/detail/utility2.hpp>


namespace makeshift
{

namespace detail
{

namespace adl
{


template <typename EnumT,
          std::enable_if_t<is_flags_enum_v<EnumT>, int> = 0>
    MAKESHIFT_NODISCARD constexpr EnumT
        operator ~(EnumT val) noexcept
{
    return EnumT(~std::underlying_type_t<EnumT>(val));
}
template <typename EnumT,
          std::enable_if_t<is_flags_enum_v<EnumT>, int> = 0>
    MAKESHIFT_NODISCARD constexpr EnumT
    operator |(EnumT lhs, EnumT rhs) noexcept
{
    return EnumT(std::underlying_type_t<EnumT>(lhs) | std::underlying_type_t<EnumT>(rhs));
}
template <typename EnumT,
          std::enable_if_t<is_flags_enum_v<EnumT>, int> = 0>
    MAKESHIFT_NODISCARD constexpr EnumT
    operator &(EnumT lhs, EnumT rhs) noexcept
{
    return EnumT(std::underlying_type_t<EnumT>(lhs) & std::underlying_type_t<EnumT>(rhs));
}
template <typename EnumT,
          std::enable_if_t<is_flags_enum_v<EnumT>, int> = 0>
    MAKESHIFT_NODISCARD constexpr EnumT
    operator ^(EnumT lhs, EnumT rhs) noexcept
{
    return EnumT(std::underlying_type_t<EnumT>(lhs) ^ std::underlying_type_t<EnumT>(rhs));
}
template <typename EnumT,
          std::enable_if_t<is_flags_enum_v<EnumT>, int> = 0>
    MAKESHIFT_NODISCARD constexpr EnumT&
    operator |=(EnumT& lhs, EnumT rhs) noexcept
{
    lhs = lhs | rhs;
    return lhs;
}
template <typename EnumT,
          std::enable_if_t<is_flags_enum_v<EnumT>, int> = 0>
    MAKESHIFT_NODISCARD constexpr EnumT&
    operator &=(EnumT& lhs, EnumT rhs) noexcept
{
    lhs = lhs & rhs;
    return lhs;
}
template <typename EnumT,
          std::enable_if_t<is_flags_enum_v<EnumT>, int> = 0>
    MAKESHIFT_NODISCARD constexpr EnumT&
    operator ^=(EnumT& lhs, EnumT rhs) noexcept
{
    lhs = lhs ^ rhs;
    return lhs;
}


} // namespace adl

} // namespace detail


    //ᅟ
    // `has_flag(haystack, needle)` determines whether the flags enum `haystack` contains the flag `needle`. Equivalent to `(haystack & needle) == needle`.
    //
template <typename EnumT>
    MAKESHIFT_NODISCARD constexpr bool
    has_flag(EnumT haystack, EnumT needle)
{
    static_assert(is_flags_enum_v<EnumT>, "arguments must be of flags enum type");
    Expects(makeshift::detail::is_power_of_2(std::underlying_type_t<EnumT>(needle)));
    return (haystack & needle) == needle;
}
    
    //ᅟ
    // `has_any_flag_of(haystack, needles)` determines whether the flags enum `haystack` contains any of the flags in `needles`. Equivalent to `(haystack & needles) != EnumT::none`.
    //
template <typename EnumT>
    MAKESHIFT_NODISCARD constexpr bool
    has_any_flag_of(EnumT haystack, EnumT needles)
{
    static_assert(is_flags_enum_v<EnumT>, "arguments must be of flags enum type");
    return (haystack & needles) != EnumT::none;
}

    //ᅟ
    // `has_all_flags_of(haystack, needles)` determines whether the flags enum `haystack` contains all of the flags in `needles`. Equivalent to `(haystack & needles) == needles`.
    //
template <typename EnumT>
    MAKESHIFT_NODISCARD constexpr bool
    has_all_flags_of(EnumT haystack, EnumT needles)
{
    static_assert(is_flags_enum_v<EnumT>, "arguments must be of flags enum type");
    return (haystack & needles) == needles;
}


    //ᅟ
    // Inherit from `define_flags<>` to define a flag enum type:
    //ᅟ
    //ᅟ    struct Vegetable : define_flags<Vegetable>
    //ᅟ    {
    //ᅟ        static constexpr auto tomato = flag(1);
    //ᅟ        static constexpr auto onion = flag(2);
    //ᅟ        static constexpr auto eggplant = flag(4);
    //ᅟ    };
    //ᅟ    using Vegetables = Vegetable::flags;
    //
template <typename FlagsT, typename UnderlyingTypeT = unsigned>
    struct define_flags : makeshift::detail::adl::define_flags_base<FlagsT, UnderlyingTypeT>
{
        // make sure no objects of the flag type itself can be created
    define_flags(void) = delete;
    define_flags(const define_flags&) = delete;
    define_flags& operator =(const define_flags&) = delete;
};


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


    //ᅟ
    // Represents an index value.
    //
using index = std::ptrdiff_t;


    //ᅟ
    // Represents a difference value.
    //
using diff = std::ptrdiff_t;


    //ᅟ
    // Represents a dimension value.
    //
using dim = std::ptrdiff_t;


    //ᅟ
    // Represents an array stride.
    //
using stride = std::ptrdiff_t;


    //ᅟ
    // Represents an integer constexpr value.
    //
template <int Value> using int_constant = std::integral_constant<int, Value>;

    //ᅟ
    // Represents an integer constexpr value.
    //
template <int Value> constexpr int_constant<Value> int_c{ };


    //ᅟ
    // Represents a boolean constexpr value.
    //
template <bool Value> using bool_constant = std::integral_constant<bool, Value>; // superseded by `std::bool_constant<>` in C++17

    //ᅟ
    // Represents a boolean constexpr value.
    //
template <bool Value> constexpr bool_constant<Value> bool_c{ };

    //ᅟ
    // Represents the constexpr value `false`.
    //
constexpr inline bool_constant<false> false_c{ };

    //ᅟ
    // Represents the constexpr value `true`.
    //
constexpr inline bool_constant<true> true_c{ };


    //ᅟ
    // Represents an index constexpr value.
    //
template <index Value> using index_constant = std::integral_constant<index, Value>;

    //ᅟ
    // Represents an index constexpr value.
    //
template <index Value> constexpr index_constant<Value> index_c{ };


    //ᅟ
    // Represents a difference constexpr value.
    //
template <diff Value> using diff_constant = std::integral_constant<diff, Value>;

    //ᅟ
    // Represents a difference constexpr value.
    //
template <diff Value> constexpr diff_constant<Value> diff_c{ };


    //ᅟ
    // Represents a dimension constexpr value.
    //
template <dim Value> using dim_constant = std::integral_constant<dim, Value>;

    //ᅟ
    // Represents an dimension constexpr value.
    //
template <dim Value> constexpr dim_constant<Value> dim_c{ };


    //ᅟ
    // Represents an array stride constexpr value.
    //
template <stride Value> using stride_constant = std::integral_constant<stride, Value>;

    //ᅟ
    // Represents an array stride constexpr value.
    //
template <stride Value> constexpr stride_constant<Value> stride_c{ };


    //ᅟ
    // Returns the size of an array or container.
    //
template <typename ContainerT> 
    MAKESHIFT_NODISCARD constexpr auto size(const ContainerT& c) -> decltype(c.size())
{
    // superseded by `std::size()` in C++17
    return c.size();
}
template <typename T, std::size_t N>
    MAKESHIFT_NODISCARD constexpr std::size_t size(const T (&)[N]) noexcept
{
    // superseded by `std::size()` in C++17
    return N;
}


    //ᅟ
    // Returns the signed size of an array or container.
    //
template <typename ContainerT>
    MAKESHIFT_NODISCARD constexpr auto ssize(const ContainerT& c)
        -> std::common_type_t<std::ptrdiff_t, std::make_signed_t<decltype(c.size())>>
{
    // superseded by `std::ssize()` in C++2a
    using R = std::common_type_t<std::ptrdiff_t, std::make_signed_t<decltype(c.size())>>;
    return static_cast<R>(c.size());
}
template <typename T, std::ptrdiff_t N>
    MAKESHIFT_NODISCARD constexpr std::ptrdiff_t ssize(const T (&)[N]) noexcept
{
    // superseded by `std::ssize()` in C++2a
    return N;
}


    //ᅟ
    // Returns the size of an array or container as a constexpr value if known at compile time, or as a value if not.
    //
template <typename ContainerT>
    MAKESHIFT_NODISCARD constexpr auto csize(const ContainerT& c)
{
    return csize_impl(can_instantiate<makeshift::detail::is_tuple_like_r, ContainerT>{ }, c);
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
    return cssize_impl(can_instantiate<makeshift::detail::is_tuple_like_r, ContainerT>{ }, c);
}
template <typename T, std::ptrdiff_t N>
    MAKESHIFT_NODISCARD constexpr std::integral_constant<std::ptrdiff_t, N> cssize(const T (&)[N]) noexcept
{
    return { };
}


inline namespace literals
{


// TODO: remove


    //ᅟ
    // Encodes an integer value given as numeric literal in the type of the expression using `int_constant<>`.
    //ᅟ
    //ᅟ    auto i = 42_int; // decltype(i) is integral_constant<int, 42>
    //
template <char... Cs>
    constexpr inline int_constant<makeshift::detail::make_constant<int, Cs...>::value>
    operator "" _int(void) noexcept
{
    return { };
}


    //ᅟ
    // Encodes an index value given as numeric literal in the type of the expression using `index_constant<>`.
    //ᅟ
    //ᅟ    auto i = 42_idx; // decltype(i) is integral_constant<index, 42>
    //
template <char... Cs>
    constexpr inline index_constant<makeshift::detail::make_constant<index, Cs...>::value>
    operator "" _idx(void) noexcept
{
    return { };
}


    //ᅟ
    // Encodes an index value given as numeric literal in the type of the expression using `diff_constant<>`.
    //ᅟ
    //ᅟ    auto i = 42_diff; // decltype(i) is integral_constant<diff, 42>
    //
template <char... Cs>
    constexpr inline diff_constant<makeshift::detail::make_constant<diff, Cs...>::value>
    operator "" _diff(void) noexcept
{
    return { };
}


    //ᅟ
    // Encodes an index value given as numeric literal in the type of the expression using `stride_constant<>`.
    //ᅟ
    //ᅟ    auto i = 42_stride; // decltype(i) is integral_constant<stride, 42>
    //
template <char... Cs>
    constexpr inline stride_constant<makeshift::detail::make_constant<stride, Cs...>::value>
    operator "" _stride(void) noexcept
{
    return { };
}


    //ᅟ
    // Encodes a dimension value given as numeric literal in the type of the expression using `dim_constant<>`.
    //ᅟ
    //ᅟ    auto d = 3_dim; // decltype(i) is integral_constant<dim, 3>
    //
template <char... Cs>
    constexpr inline dim_constant<makeshift::detail::make_constant<dim, Cs...>::value>
    operator "" _dim(void) noexcept
{
    return { };
}


} // inline namespace literals

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_UTILITY2_HPP_
