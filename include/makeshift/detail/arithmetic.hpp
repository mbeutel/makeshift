
#ifndef INCLUDED_MAKESHIFT_DETAIL_ARITHMETIC_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_ARITHMETIC_HPP_


#include <limits>
#include <type_traits> // for is_[un]signed<>, is_integral<>
#include <cstdint>     // for [u]int(8|16|32|64)_t


namespace makeshift
{

namespace detail
{


template <typename V> struct wider_type;
template <> struct wider_type<std::int8_t> { using type = std::int32_t; };
template <> struct wider_type<std::int16_t> { using type = std::int32_t; };
template <> struct wider_type<std::int32_t> { using type = std::int64_t; };
template <> struct wider_type<std::uint8_t> { using type = std::uint32_t; };
template <> struct wider_type<std::uint16_t> { using type = std::uint32_t; };
template <> struct wider_type<std::uint32_t> { using type = std::uint64_t; };

enum int_signedness { is_signed, is_unsigned };
enum int_width { has_wider_type, has_no_wider_type };

template <typename EH, typename V>
    struct checked_operations;

template <typename EH, typename V, int_signedness>
    struct checked_3_;
template <typename EH, typename V>
    struct checked_3_<EH, V, is_unsigned>
{
    static constexpr V divide(V lhs, V rhs)
    {
        EH::checkDivByZero(rhs != 0);
        return lhs / rhs;
    }
    static constexpr V modulo(V lhs, V rhs)
    {
        EH::checkDivByZero(rhs != 0);
        return lhs % rhs;
    }
    static constexpr V negate(V arg)
    {
        EH::checkUnderflow(arg == 0);
        return 0;
    }
    static constexpr V subtract(V lhs, V rhs)
    {
        EH::checkUnderflow(lhs >= rhs);
        return lhs - rhs;
    }
    static constexpr V shl(V lhs, V rhs)
    {
        EH::checkDomain(rhs < sizeof(V)*8);
        EH::checkOverflow(lhs <= (std::numeric_limits<V>::max() >> rhs));
        return lhs << rhs;
    }
    static constexpr V shr(V lhs, V rhs)
    {
        EH::checkDomain(rhs < sizeof(V)*8);
        return lhs >> rhs;
    }

    static constexpr V powi(V base, V exp)
    {
            // conventionally, `powi(0,0)` is 1
        if (exp == 0)
            return exp != 0 ? 0 : 1;

            // we assume `b > 0` henceforth
        V bound = std::numeric_limits<T>::max() / base;

        V result = T(1);
        while (exp > 0)
        {
                // ensure the multiplication cannot overflow
            EH::checkOverflow(result <= bound);
            result *= base;
            --exp;
        }
        return result;
    }
};
template <typename EH, typename V>
    struct checked_3_<EH, V, is_signed>
{
    using U = typename std::make_unsigned<V>::type;

    static constexpr V divide(V lhs, V rhs)
    {
        EH::checkOverflow(lhs != std::numeric_limits<V>::min() || rhs != -1);
        EH::checkDivByZero(rhs != 0);
        return lhs / rhs;
    }
    static constexpr V modulo(V lhs, V rhs)
    {
        EH::checkOverflow(lhs != std::numeric_limits<V>::min() || rhs != -1);
        EH::checkDivByZero(rhs != 0);
        return lhs % rhs;
    }
    static constexpr V negate(V arg)
    {
            // this assumes a two's complement representation (it will yield a false negative for one's complement integers)
        EH::checkUnderflow(arg != std::numeric_limits<V>::min());
        return -arg;
    }
    static constexpr V subtract(V lhs, V rhs)
    {
        EH::checkUnderflow(
            (rhs <= 0 || lhs >= std::numeric_limits<V>::min() + rhs)
         && (rhs >= 0 || lhs <= std::numeric_limits<V>::max() + rhs));
        return lhs - rhs;
    }
    static constexpr V shl(V lhs, V rhs)
    {
            // note that we throw when shifting negative integers
        EH::checkDomain(lhs >= 0 && rhs >= 0 && rhs < V(sizeof(V)*8));
        EH::checkOverflow(lhs <= (std::numeric_limits<V>::max() >> rhs));
        return lhs << rhs;
    }
    static constexpr V shr(V lhs, V rhs)
    {
            // note that we throw when shifting negative integers
        EH::checkDomain(lhs >= 0 && rhs >= 0 && rhs < V(sizeof(V)*8));
        return lhs >> rhs;
    }

    static constexpr V _powi_non_negative_base(V base, V exp)
    {
            // conventionally, `powi(0,0)` is 1
        if (exp == 0)
            return exp != 0 ? 0 : 1;

            // we assume `b > 0` henceforth
        V bound = std::numeric_limits<T>::max() / base;

        V result = T(1);
        while (exp > 0)
        {
                // ensure the multiplication cannot overflow
            EH::checkOverflow(result <= bound);
            result *= base;
            --exp;
        }
        return result;
    }
    static constexpr V powi(V base, V exp)
    {
            // negative powers are not integral
        EH::checkDomain(exp >= 0);

        if (base >= 0)
            return _powi_non_negative_base(base, exp);

            // `b` is negative; factor out sign
        V sign = 1 - 2 * (exp % 2);

            // perform `powi()` for unsigned positive number
        using U = std::make_unsigned_t<V>;
        U absPow = checked_operations<EH, U>::powi(U(b), U(e));

            // handle special case where result is `numeric_limits<T>::min()`
        if (sign == -1 && absPow == U(std::numeric_limits<V>::max()) + 1)
            return std::numeric_limits<T>::min(); // assuming two's complement

            // convert back to signed, prefix with sign
        EH::checkOverflow(absPow <= std::numeric_limits<V>::max());
        return sign * V(absPow);
    }


};
template <typename EH, typename V, int_width, int_signedness>
    struct checked_2_;
template <typename EH, typename V>
    struct checked_2_<EH, V, has_wider_type, is_unsigned> : checked_3_<EH, V, is_unsigned>
{
    using W = typename wider_type<V>::type;

    static constexpr V add(V lhs, V rhs)
    {
        W result = W(lhs) + W(rhs);
        EH::checkOverflow(result <= std::numeric_limits<V>::max());
        return static_cast<V>(result);
    }
    static constexpr V multiply(V lhs, V rhs)
    {
        W result = W(lhs) * W(rhs);
        EH::checkOverflow(result <= std::numeric_limits<V>::max());
        return static_cast<V>(result);
    }
};
template <typename EH, typename V>
    struct checked_2_<EH, V, has_wider_type, is_signed> : checked_3_<EH, V, is_signed>
{
    using W = typename wider_type<V>::type;

    static constexpr V add(V lhs, V rhs)
    {
        W result = W(lhs) + W(rhs);
        EH::checkOverflow(result >= std::numeric_limits<V>::min() && result <= std::numeric_limits<V>::max());
        return static_cast<V>(result);
    }
    static constexpr V multiply(V lhs, V rhs)
    {
        W result = W(lhs) * W(rhs);
        EH::checkOverflow(result >= std::numeric_limits<V>::min() && result <= std::numeric_limits<V>::max());
        return static_cast<V>(result);
    }
};
template <typename EH, typename V>
    struct checked_2_<EH, V, has_no_wider_type, is_unsigned> : checked_3_<EH, V, is_unsigned>
{
    static constexpr V add(V lhs, V rhs)
    {
        V result = lhs + rhs;
        EH::checkOverflow(result >= lhs && result >= rhs);
        return result;
    }
    static constexpr V multiply(V lhs, V rhs)
    {
        V result = lhs * rhs;
        EH::checkOverflow(
            (result >= lhs || rhs == 0)
         && (result >= rhs || lhs == 0));
        return result;
    }
};
template <typename EH, typename V>
    struct checked_2_<EH, V, has_no_wider_type, is_signed> : checked_3_<EH, V, is_signed>
{
    using U = typename std::make_unsigned<V>::type;

    static constexpr V add(V lhs, V rhs)
    {
        V result = V(U(lhs) + U(rhs));
        EH::checkOverflow(
            (lhs >= 0 || rhs >= 0 || result < 0) // at least one operand is non-negative, or the result is negative
         && (lhs <  0 || rhs <  0 || result >= 0)); // at least one operand is negative, or the result is non-negative
        return result;
    }
    static constexpr V multiply(V lhs, V rhs)
    {
        EH::checkOverflow(
            (lhs <= 0 || ((rhs <= 0             || lhs <= std::numeric_limits<V>::max() / rhs)
                       && (rhs >  0             || rhs >= std::numeric_limits<V>::min() / lhs)))
         && (lhs >  0 || ((rhs <= 0             || lhs >= std::numeric_limits<V>::min() / rhs)
                       && (rhs >  0 || lhs == 0 || rhs >= std::numeric_limits<V>::max() / lhs))));
        return lhs * rhs;
    }
};
template <typename EH, typename V>
    struct checked_operations : checked_2_<EH, V,
        (sizeof(V) < sizeof(std::uint64_t)) ? has_wider_type : has_no_wider_type,
        std::is_signed<V>::value ? is_signed : is_unsigned>
{
    static_assert(!std::is_same<V, bool>::value, "checked arithmetic does not support bool");
    static_assert(std::is_integral<V>::value, "checked arithmetic only works with integral types");
};


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_ARITHMETIC_HPP_
