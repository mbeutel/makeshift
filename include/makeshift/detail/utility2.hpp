
#ifndef INCLUDED_MAKESHIFT_DETAIL_UTILITY2_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_UTILITY2_HPP_


#include <cstddef>     // for size_t, ptrdiff_t
#include <utility>     // for tuple_size<>, integer_sequence<>
#include <iterator>    // for iterator_traits<>, bidirectional_iterator_tag
#include <exception>   // for terminate()
#include <type_traits> // for integral_constant<>, make_signed<>, make_unsigned<>, common_type<>

#include <gsl/gsl_assert> // for Expects()

#include <makeshift/type_traits.hpp>  // for tag<>
#include <makeshift/type_traits2.hpp> // for flags_base, unwrap_enum_tag, type<>, type_sequence<>
#include <makeshift/version.hpp>      // for MAKESHIFT_NODISCARD

#include <makeshift/detail/type_traits2.hpp> // for is_tuple_like_r<>
#include <makeshift/detail/workaround.hpp>   // for csum<>(), cand()


namespace makeshift
{

namespace detail
{


template <typename T>
    constexpr bool is_power_of_2(T value) noexcept
{
    return value > 0
        && (value & (value - 1)) == 0;
}


namespace adl
{


template <typename FlagsT, typename UnderlyingTypeT>
    struct define_flags_base : makeshift::detail::flags_base
{
public:
    using base_type = UnderlyingTypeT;
    enum class flags : UnderlyingTypeT { none = 0 };

    flags value; // used for Natvis debugger visualizer

    define_flags_base(void) = delete;

    static constexpr flags none = flags::none;

    friend constexpr type<FlagsT> flag_type_of_(flags, makeshift::detail::unwrap_enum_tag) { return { }; }

        // We just forward the metadata defined for the derived type.
        // TODO: ensure that have_metadata<flag> is false if no metadata is defined for FlagsT.
    template <typename MetadataTagT>
        friend constexpr auto reflect(tag<flags>, MetadataTagT) -> decltype(reflect(tag<FlagsT>{ }, MetadataTagT{ }))
    {
        return reflect(tag<FlagsT>{ }, MetadataTagT{ });
    }

        // We just forward the metadata defined for the derived type.
        // TODO: ensure that have_metadata<flag> is false if no metadata is defined for FlagsT.
    template <typename U = FlagsT>
        friend constexpr auto reflect(type<flags>) -> decltype(reflect(type<U>{ }))
    {
        return reflect(type<FlagsT>{ });
    }

protected:
    static constexpr flags flag(UnderlyingTypeT constant)
    {
        Expects(is_power_of_2(constant));
        return flags(constant);
    }
};


template <typename TypeEnumT, typename... Ts>
    struct define_type_enum_base : makeshift::detail::type_enum_base
{
private:
    enum class value_t : int { };

    value_t value_;

public:
    friend constexpr type<TypeEnumT> type_enum_type_of_(value_t, makeshift::detail::unwrap_enum_tag) { return { }; }

    using types = type_sequence<Ts...>;
    static constexpr std::size_t size = sizeof...(Ts);

    constexpr define_type_enum_base(const define_type_enum_base&) = default;
    constexpr define_type_enum_base& operator =(const define_type_enum_base&) = default;

    template <typename T,
              std::enable_if_t<try_index_of_type_v<T, Ts...> != -1, int> = 0>
        constexpr define_type_enum_base(type<T>) noexcept
            : value_(value_t(int(try_index_of_type_v<T, Ts...>)))
    {
    }

    explicit constexpr define_type_enum_base(int _value)
        : value_(value_t(_value))
    {
        Expects(_value >= 0 && _value < int(sizeof...(Ts)));
    }

    explicit operator bool(void) const = delete;

        // This conversion exists so type enums can be used in switch statements.
    constexpr operator value_t(void) const noexcept { return value_; }

    constexpr explicit operator int(void) const noexcept { return int(value_); }

    friend constexpr bool operator ==(define_type_enum_base lhs, define_type_enum_base rhs) noexcept
    {
        return lhs.value_ == rhs.value_;
    }
    friend constexpr bool operator !=(define_type_enum_base lhs, define_type_enum_base rhs) noexcept
    {
        return !(lhs == rhs);
    }
};

template <typename TypeEnumT, typename... Ts, typename T,
          std::enable_if_t<try_index_of_type_v<T, Ts...> != -1, int> = 0>
    constexpr bool operator ==(define_type_enum_base<TypeEnumT, Ts...> lhs, type<T>) noexcept
{
    return int(try_index_of_type_v<T, Ts...>) == int(lhs);
}
template <typename TypeEnumT, typename... Ts, typename T,
          std::enable_if_t<try_index_of_type_v<T, Ts...> != -1, int> = 0>
    constexpr bool operator ==(type<T> lhs, define_type_enum_base<TypeEnumT, Ts...> rhs) noexcept
{
    return rhs == lhs;
}
template <typename TypeEnumT, typename... Ts, typename T,
          std::enable_if_t<try_index_of_type_v<T, Ts...> != -1, int> = 0>
    constexpr bool operator !=(define_type_enum_base<TypeEnumT, Ts...> lhs, type<T> rhs) noexcept
{
    return !(lhs == rhs);
}
template <typename TypeEnumT, typename... Ts, typename T,
          std::enable_if_t<try_index_of_type_v<T, Ts...> != -1, int> = 0>
    constexpr bool operator !=(type<T> lhs, define_type_enum_base<TypeEnumT, Ts...> rhs) noexcept
{
    return !(rhs == lhs);
}


} // namespace adl


template <typename ContainerT>
    constexpr std::integral_constant<std::size_t, std::tuple_size<ContainerT>::value> csize_impl(std::true_type /*isConstval*/, const ContainerT&)
{
    return { };
}
template <typename ContainerT>
    constexpr auto csize_impl(std::false_type /*isConstval*/, const ContainerT& c)
        -> decltype(c.size())
{
    return c.size();
}

template <typename ContainerT>
    constexpr std::integral_constant<std::ptrdiff_t, std::tuple_size<ContainerT>::value> cssize_impl(std::true_type /*isConstval*/, const ContainerT&)
{
    return { };
}
template <typename ContainerT>
    constexpr auto cssize_impl(std::false_type /*isConstval*/, const ContainerT& c)
        -> std::common_type_t<std::ptrdiff_t, std::make_signed_t<decltype(c.size())>>
{
    using R = std::common_type_t<std::ptrdiff_t, std::make_signed_t<decltype(c.size())>>;
    return static_cast<R>(c.size());
}


constexpr inline std::ptrdiff_t cpow(int B, int I, int N) noexcept
{
    std::ptrdiff_t result = 1;
    for (int n = 0; n != N - 1 - I; ++n)
        result *= B;
    return result;
}
constexpr inline int hex2num(char hex) noexcept
{
    if (hex >= '0' && hex <= '9')
        return hex - '0';
    else if (hex >= 'A' && hex <= 'F')
        return hex - 'A';
    else if (hex >= 'a' && hex <= 'f')
        return hex - 'a';
    else
        std::terminate(); // should not happen
}
template <typename T, typename Is, char... Cs> struct make_constant_1_;
template <typename T, std::size_t... Is, char... Cs>
    struct make_constant_1_<T, std::index_sequence<Is...>, Cs...>
{
    static_assert(cand(Cs >= '0' && Cs <= '9'...), "invalid character: value must be a decimal integer literal");
    static constexpr T value = csum<T>((Cs - '0') * cpow(10, Is, sizeof...(Cs))...);
};
template <typename T, typename Is, char... Cs> struct make_constant_2_ : make_constant_1_<T, Is, Cs...> { };
template <typename T, std::size_t... Is, char... Cs>
    struct make_constant_2_<T, std::index_sequence<0, Is...>, '0', Cs...>
{
    static_assert(cand(Cs >= '0' && Cs <= '7'...), "invalid character: value must be an octal integer literal");
    static constexpr T value = csum<T>((Cs - '0') * cpow(8, Is - 1, sizeof...(Cs))...);
};
template <typename T, std::size_t... Is, char... Cs>
    struct make_constant_2_<T, std::index_sequence<0, 1, Is...>, '0', 'x', Cs...>
{
    static_assert(cand((Cs >= '0' && Cs <= '9') || (Cs >= 'A' && Cs <= 'F') || (Cs >= 'a' && Cs <= 'f')...), "invalid character: value must be a hexadecimal integer literal");
    static constexpr T value = csum<T>(hex2num(Cs) * cpow(16, Is - 2, sizeof...(Cs))...);
};
template <typename T, typename Is, char... Cs> struct make_constant_3_ : make_constant_2_<T, Is, Cs...> { };
template <typename T, std::size_t... Is, char... Cs>
    struct make_constant_3_<T, std::index_sequence<0, 1, Is...>, '0', 'X', Cs...>
        : make_constant_3_<T, std::index_sequence<0, 1, Is...>, '0', 'x', Cs...>
{
};
template <typename T, std::size_t... Is, char... Cs>
    struct make_constant_3_<T, std::index_sequence<0, 1, Is...>, '0', 'b', Cs...>
{
    static_assert(cand((Cs >= '0' && Cs <= '1')...), "invalid character: value must be a binary integer literal");
    static constexpr T value = csum<T>((Cs - '0') * cpow(2, Is - 2, sizeof...(Cs))...);
};
template <typename T, std::size_t... Is, char... Cs>
    struct make_constant_3_<T, std::index_sequence<0, 1, Is...>, '0', 'B', Cs...>
        : make_constant_3_<T, std::index_sequence<0, 1, Is...>, '0', 'b', Cs...>
{
};
template <typename T, std::size_t N, typename Is, char... Cs> struct make_constant_ : make_constant_3_<T, Is, Cs...> { };
template <typename T, typename Is, char... Cs> struct make_constant_<T, 1, Is, Cs...> : make_constant_1_<T, Is, Cs...> { };
template <typename T, typename Is, char... Cs> struct make_constant_<T, 2, Is, Cs...> : make_constant_2_<T, Is, Cs...> { };
template <typename T, char... Cs> struct make_constant : make_constant_<T, sizeof...(Cs), std::make_index_sequence<sizeof...(Cs)>, Cs...> { };
template <typename T, char... Cs> constexpr std::ptrdiff_t make_constant_v = make_constant<T, Cs...>::value;


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_UTILITY2_HPP_
