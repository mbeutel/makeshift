
#ifndef INCLUDED_MAKESHIFT_DETAIL_ENUM_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_ENUM_HPP_


#include <cstddef>     // for size_t
#include <cstdint>     // for int32_t
#include <type_traits> // for enable_if<>

#include <gsl/gsl-lite.hpp> // for Expects()

#include <makeshift/utility.hpp> // for type<>, type_sequence<>
#include <makeshift/macros.hpp>  // for MAKESHIFT_NODISCARD

#include <makeshift/detail/type_traits.hpp> // for is_tuple_like_r<>, type_enum_base, unwrap_enum_tag


#define MAKESHIFT_DEFINE_BITMASK_OPERATORS_(ENUM)                                                       \
    MAKESHIFT_NODISCARD constexpr inline ENUM                                                           \
    operator ~(ENUM val) noexcept                                                                       \
    {                                                                                                   \
        return ENUM(~makeshift::underlying_type_t<ENUM>(val));                                          \
    }                                                                                                   \
    MAKESHIFT_NODISCARD constexpr inline ENUM                                                           \
    operator |(ENUM lhs, ENUM rhs) noexcept                                                             \
    {                                                                                                   \
        return ENUM(makeshift::underlying_type_t<ENUM>(lhs) | makeshift::underlying_type_t<ENUM>(rhs)); \
    }                                                                                                   \
    MAKESHIFT_NODISCARD constexpr inline ENUM                                                           \
    operator &(ENUM lhs, ENUM rhs) noexcept                                                             \
    {                                                                                                   \
        return ENUM(makeshift::underlying_type_t<ENUM>(lhs) & makeshift::underlying_type_t<ENUM>(rhs)); \
    }                                                                                                   \
    MAKESHIFT_NODISCARD constexpr inline ENUM                                                           \
    operator ^(ENUM lhs, ENUM rhs) noexcept                                                             \
    {                                                                                                   \
        return ENUM(makeshift::underlying_type_t<ENUM>(lhs) ^ makeshift::underlying_type_t<ENUM>(rhs)); \
    }                                                                                                   \
    constexpr inline ENUM&                                                                              \
    operator |=(ENUM& lhs, ENUM rhs) noexcept                                                           \
    {                                                                                                   \
        lhs = lhs | rhs;                                                                                \
        return lhs;                                                                                     \
    }                                                                                                   \
    constexpr inline ENUM&                                                                              \
    operator &=(ENUM& lhs, ENUM rhs) noexcept                                                           \
    {                                                                                                   \
        lhs = lhs & rhs;                                                                                \
        return lhs;                                                                                     \
    }                                                                                                   \
    constexpr inline ENUM&                                                                              \
    operator ^=(ENUM& lhs, ENUM rhs) noexcept                                                           \
    {                                                                                                   \
        lhs = lhs ^ rhs;                                                                                \
        return lhs;                                                                                     \
    }

namespace makeshift
{

namespace detail
{


template <typename T>
    constexpr bool is_flag_power_of_2(T value) noexcept
{
    return value > 0
        && (value & (value - 1)) == 0;
}


template <typename TypeEnumT, typename... Ts>
    struct define_type_enum_base : makeshift::detail::type_enum_base
{
    static_assert(sizeof...(Ts) < 24, "type enumeration may not contain more than 24 types");

private:
    static constexpr std::int32_t n = sizeof...(Ts);

        // This type exists to permit usage of type enums in switch statements. We define enumerators with suitable values to make use of compiler warnings
        // for non-exhaustive switch statements. (We also avoid warning C4063 "case '<integer>' is not a valid value for switch of enum" for VC++ this way.)
    enum class value_t : std::int32_t
    {
        type0  = 0,               type1  = n >  1 ?  1 : 0, type2  = n >  2 ?  2 : 0, type3  = n >  3 ?  3 : 0, type4  = n >  4 ?  4 : 0, type5  = n >  5 ?  5 : 0,
        type6  = n >  6 ?  6 : 0, type7  = n >  7 ?  7 : 0, type8  = n >  8 ?  8 : 0, type9  = n >  9 ?  9 : 0, type10 = n > 10 ? 10 : 0, type11 = n > 11 ? 11 : 0,
        type12 = n > 12 ? 12 : 0, type13 = n > 13 ? 13 : 0, type14 = n > 14 ? 14 : 0, type15 = n > 15 ? 15 : 0, type16 = n > 16 ? 16 : 0, type17 = n > 17 ? 17 : 0,
        type18 = n > 18 ? 18 : 0, type19 = n > 19 ? 19 : 0, type20 = n > 20 ? 20 : 0, type21 = n > 21 ? 21 : 0, type22 = n > 22 ? 22 : 0, type23 = n > 23 ? 23 : 0
    };

    value_t value_;

public:
    using underlying_type = std::int32_t;

    friend constexpr type<TypeEnumT> type_enum_type_of_(value_t, makeshift::detail::unwrap_enum_tag) { return { }; }

    using types = type_sequence<Ts...>;
    static constexpr std::size_t size = sizeof...(Ts); // TODO: ??

    constexpr define_type_enum_base(const define_type_enum_base&) = default;
    constexpr define_type_enum_base& operator =(const define_type_enum_base&) = default;

    explicit constexpr define_type_enum_base(value_t _value)
        : value_(_value)
    {
        Expects(std::int32_t(_value) >= 0 && std::int32_t(_value) < std::int32_t(sizeof...(Ts)));
    }
    explicit constexpr define_type_enum_base(std::int32_t _value)
        : define_type_enum_base(value_t(_value))
    {
    }

    explicit operator bool(void) const = delete;

        // This conversion exists so type enums can be used in switch statements.
    constexpr operator value_t(void) const noexcept { return value_; }

    constexpr explicit operator std::int32_t(void) const noexcept { return std::int32_t(value_); }

    friend constexpr bool operator ==(define_type_enum_base lhs, define_type_enum_base rhs) noexcept
    {
        return lhs.value_ == rhs.value_;
    }
    friend constexpr bool operator !=(define_type_enum_base lhs, define_type_enum_base rhs) noexcept
    {
        return !(lhs == rhs);
    }
};
template <typename TypeEnumT>
    struct define_type_enum_base<TypeEnumT> : makeshift::detail::type_enum_base
{
public:
    using underlying_type = void;

    using types = type_sequence<>;
    static constexpr std::size_t size = 0;

    constexpr define_type_enum_base(const define_type_enum_base&) = default;
    constexpr define_type_enum_base& operator =(const define_type_enum_base&) = default;

    explicit operator bool(void) const = delete;

    friend constexpr bool operator ==(define_type_enum_base, define_type_enum_base) noexcept
    {
        return true;
    }
    friend constexpr bool operator !=(define_type_enum_base lhs, define_type_enum_base rhs) noexcept
    {
        return !(lhs == rhs);
    }
};

template <typename TypeEnumT, typename... Ts, typename T>
    constexpr std::enable_if_t<try_index_of_type_v<T, Ts...> != -1, bool>
    operator ==(define_type_enum_base<TypeEnumT, Ts...> lhs, type<T>) noexcept
{
    return std::int32_t(try_index_of_type_v<T, Ts...>) == std::int32_t(lhs);
}
template <typename TypeEnumT, typename... Ts, typename T>
    constexpr std::enable_if_t<try_index_of_type_v<T, Ts...> != -1, bool>
    operator ==(type<T> lhs, define_type_enum_base<TypeEnumT, Ts...> rhs) noexcept
{
    return rhs == lhs;
}
template <typename TypeEnumT, typename... Ts, typename T>
    constexpr std::enable_if_t<try_index_of_type_v<T, Ts...> != -1, bool>
    operator !=(define_type_enum_base<TypeEnumT, Ts...> lhs, type<T> rhs) noexcept
{
    return !(lhs == rhs);
}
template <typename TypeEnumT, typename... Ts, typename T>
    constexpr std::enable_if_t<try_index_of_type_v<T, Ts...> != -1, bool>
    operator !=(type<T> lhs, define_type_enum_base<TypeEnumT, Ts...> rhs) noexcept
{
    return !(rhs == lhs);
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_ENUM_HPP_
