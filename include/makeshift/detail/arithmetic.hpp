
#ifndef INCLUDED_MAKESHIFT_DETAIL_ARITHMETIC_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_ARITHMETIC_HPP_


#include <limits>
#include <type_traits> // for is_[un]signed<>, is_integral<>
#include <cstdint>     // for [u]int(8|16|32|64)_t

#include <gsl/gsl_assert> // for Expects()

#include <makeshift/version.hpp> // for MAKESHIFT_FORCEINLINE


namespace makeshift
{

inline namespace arithmetic
{


template <typename V>
    struct factor;
template <typename V, dim_t NumFactors>
    struct factorization;


} // inline namespace arithmetic


namespace detail
{


    // The implementations below have borrowed heavily from the suggestions made and examples used in the SEI CERT C Coding Standard:
    // https://wiki.sei.cmu.edu/confluence/display/c/


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
        Expects(rhs != 0);
        return lhs / rhs;
    }
    static constexpr V modulo(V lhs, V rhs)
    {
        Expects(rhs != 0);
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
        Expects(rhs < sizeof(V)*8);
        EH::checkOverflow(lhs <= (std::numeric_limits<V>::max() >> rhs));
        return lhs << rhs;
    }
    static constexpr V shr(V lhs, V rhs)
    {
        Expects(rhs < sizeof(V)*8);
        return lhs >> rhs;
    }

        // Computes bᵉ for e ∊ ℕ₀.
    template <typename N>
        static constexpr V powi(V b, N e)
    {
        static_assert(std::is_integral<N>::value, "exponent must be an integral type");

            // negative powers are not integral
        Expects(e >= 0);

            // conventionally, `powi(0,0)` is 1
        if (e == 0)
            return e != 0 ? 0 : 1;
        if (b == 0)
            return 0;

            // we assume `b > 0` henceforth
        V m = std::numeric_limits<V>::max() / b;

        V result = 1;
        while (e > 0)
        {
                // ensure the multiplication cannot overflow
            EH::checkOverflow(result <= m);
            result *= b;
            --e;
        }
        return result;
    }

        // Computes ⌊n ÷ d⌋ for n ∊ ℕ₀, d ∊ ℕ.
    static constexpr V ratio_floor(V n, V d)
    {
        Expects(d != 0);
        return n / d;
    }

        // Computes ⌈n ÷ d⌉ for n ∊ ℕ₀, d ∊ ℕ.
    static constexpr V ratio_ceil(V n, V d)
    {
        Expects(d != 0);
        return n != 0
             ? (n - 1) / d + 1 // overflow-safe
             : 0;
    }

        // Given x,b ∊ ℕ, x > 0, b > 1, returns (r,{ {b,e} }) such that x = bᵉ + r with r ≥ 0 minimal.
    static constexpr factorization<V, 1> factorize_floor(V x, V b)
    {
        Expects(x > 0 && b > 1);

        V e = 0;
        V x0 = 1;
        constexpr V M = std::numeric_limits<V>::max();
        V m = M / b;

            // We generally assume bᵉ ≤ x (⇔ e ≤ log x ÷ log b).
            //
            // Then, by definition,
            //
            //  (    bᵉ⁺¹ > x
            //    ⇔ e+1 > log x ÷ log b
            //    ⇒ e = ⌊log x ÷ log b⌋ )   (<-- #1)
            //
            // Additionally we know that
            //
            //  (    bᵉ > m = ⌊M ÷ b⌋
            //    ⇒ bᵉ > M ÷ b
            //    ⇒ bᵉ⁺¹ > M ≥ x
            //    ⇒ e = ⌊log x ÷ log b⌋ )   because of #1 (<-- #2)

            // x₀ ≤ x upon loop entry because x₀ = 1, x ≥ 1.
        for (;;) 
        {
                // Compare with m before computing bᵉ⁺¹ to avoid overflow.
            if (x0 > m)
                return { x - x0, { factor<V>{ b, e } } };

            V x1 = x0 * b; // = bᵉ⁺¹
            if (x1 > x)
                return { x - x0, { factor<V>{ b, e } } };

            x0 = x1;
            ++e;
        }
    }

        // Given x,b ∊ ℕ, x > 0, b > 1, returns (r,{ {b,e} }) such that x = bᵉ - r with r ≥ 0 minimal.
    static constexpr factorization<V, 1> factorize_ceil(V x, V b)
    {
        auto [rFloor, fFloor] = factorize_floor(x, b);
        if (rFloor == 0)
            return { 0, fFloor };
        auto xFloor = x - rFloor;
        auto rCeil = checked_operations<EH, V>::multiply(xFloor, b - 1) - rFloor; // x = bᵉ + r =: bᵉ⁺¹ - r' ⇒ r' = bᵉ(b - 1) - r
        return { rCeil, { factor<V>{ b, fFloor[0].exponent + 1 } } }; // e cannot overflow
    }

        // Computes ⌊log x ÷ log b⌋ for x,b ∊ ℕ, x > 0, b > 1.
    static constexpr V log_floor(V x, V b)
    {
        auto result = factorize_floor(x, b);
        return result.factors[0].exponent;
    }

        // Computes ⌈log x ÷ log b⌉ for x,b ∊ ℕ, x > 0, b > 1.
    static constexpr V log_ceil(V x, V b)
    {
        Expects(x > 0 && b > 1);

        V e = 0;
        V x0 = 1;
        constexpr V M = std::numeric_limits<V>::max();
        V m = M / b;

        while (x0 < x)
        {
            if (x0 > m)
            {
                    // x₀ = bᵉ < x, otherwise we wouldn't be in the loop.
                    // bᵉ > m implies bᵉ⁺¹ > M (cf. reasoning in factorize_floor()).
                    // Because x ≤ M, ⌈log x ÷ log b⌉ = e + 1.
                return e + 1;
            }

            x0 *= b;
            ++e;
        }

        return e;
    }

        // Given x,a,b ∊ ℕ, x > 0, a,b > 1, a ≠ b, returns (r,{ {a,i}, {b,j} }) such that x = aⁱ ∙ bʲ - r with r ≥ 0 minimal.
    static constexpr factorization<V, 2> factorize_ceil(V x, V a, V b)
    {
        Expects(x > 0 && a > 1 && b > 1 && a != b);

            // algorithm discussed in http://stackoverflow.com/a/39050139 and slightly altered to avoid unnecessary overflows

        auto [r, f] = factorize_ceil(x, a);
        V i = f[0].exponent,
          j = 0;
        V ci = i,
          cj = j;
        V y = checked_operations<EH, V>::add(x, r); // = aⁱ
        V cy = y; // cy ≥ x at all times

        for (;;)
        {
            if (i == 0)
                return { cy - x, { factor<V>{ a, ci }, factor<V>{ b, cj } } };

                // take factor a
            y /= a;
            --i;

                // give factors b until y ≥ x
            while (y < x)
            {
                y = checked_operations<EH, V>::multiply(y, b);
                ++j;
            }

            if (y < cy)
            {
                cy = y;
                ci = i;
                cj = j;
            }
        }
    }

        // Given x,a,b ∊ ℕ, x > 0, a,b > 1, a ≠ b, returns (r,{ {a,i}, {b,j} }) such that x = aⁱ ∙ bʲ + r with r ≥ 0 minimal.
    static constexpr factorization<V, 2> factorize_floor(V x, V a, V b)
    {
        Expects(x > 0 && a > 1 && b > 1 && a != b);

            // adaption of algorithm in factorize_ceil() for different optimisation criterion

        auto [r, f] = factorize_floor(x, a);
        V i = f[0].exponent,
          j = 0;
        V ci = i,
          cj = j;
        V y = x - r; // = aⁱ
        V cy = y; // cy ≤ x at all times

        for (;;)
        {
            if (i == 0)
                return { x - cy, { factor<V>{ a, ci }, factor<V>{ b, cj } } };

                // take factor a
            y /= a;
            --i;

                // give factors b as long as y ≤ x
                // (note that y ∙ b overflowing implies y ∙ b > x)
            while (checked_operations<EH, V>::can_multiply(y, b) && y * b <= x)
            {
                y *= b;
                ++j;
            }

            if (y > cy)
            {
                cy = y;
                ci = i;
                cj = j;
            }
        }
    }
};
template <typename EH, typename V>
    struct checked_3_<EH, V, is_signed>
{
    using U = std::make_unsigned_t<V>;

    static constexpr V divide(V lhs, V rhs)
    {
        Expects(rhs != 0);
        EH::checkOverflow(lhs != std::numeric_limits<V>::min() || rhs != -1);
        return lhs / rhs;
    }
    static constexpr V modulo(V lhs, V rhs)
    {
        Expects(rhs != 0);
        EH::checkOverflow(lhs != std::numeric_limits<V>::min() || rhs != -1);
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
            // note that we fail when shifting negative integers
        Expects(lhs >= 0 && rhs >= 0 && rhs < V(sizeof(V)*8));
        EH::checkOverflow(lhs <= (std::numeric_limits<V>::max() >> rhs));
        return lhs << rhs;
    }
    static constexpr V shr(V lhs, V rhs)
    {
            // note that we fail when shifting negative integers
        Expects(lhs >= 0 && rhs >= 0 && rhs < V(sizeof(V)*8));
        return lhs >> rhs;
    }

        // Computes bᵉ for e ∊ ℕ₀.
    template <typename N>
        static constexpr V powi(V b, N e)
    {
        static_assert(std::is_integral<N>::value, "exponent must be an integral type");

            // negative powers are not integral
        Expects(e >= 0);

            // factor out sign if `b` is negative
        V sign = b >= 0
            ? 1
            : 1 - 2 * (e % 2);

            // perform `powi()` for unsigned positive number
        using U = std::make_unsigned_t<V>;
        U absPow = checked_operations<EH, U>::powi(U(sign * b), e);

            // handle special case where result is `numeric_limits<T>::min()`
        if (sign == -1 && absPow == U(std::numeric_limits<V>::max()) + 1)
            return std::numeric_limits<V>::min(); // assuming two's complement

            // convert back to signed, prefix with sign
        EH::checkOverflow(absPow <= U(std::numeric_limits<V>::max()));
        return sign * V(absPow);
    }

        // Computes ⌊n ÷ d⌋ for n ∊ ℕ₀, d ∊ ℕ.
    static constexpr V ratio_floor(V n, V d)
    {
        Expects(d > 0 && n >= 0);
        return n / d;
    }

        // Computes ⌈n ÷ d⌉ for n ∊ ℕ₀, d ∊ ℕ.
    static constexpr V ratio_ceil(V n, V d)
    {
        Expects(d > 0 && n >= 0);
        return n != 0
             ? (n - 1) / d + 1 // overflow-safe
             : 0;
    }

        // Given x,b ∊ ℕ, x > 0, b > 1, returns (r,{ {b,e} }) such that x = bᵉ + r with r ≥ 0 minimal.
    static constexpr factorization<V, 1> factorize_floor(V x, V b)
    {
        Expects(x > 0 && b > 1);
        auto [r, f] = checked_operations<EH, U>::factorize_floor(U(x), U(b));
        return { V(r), { factor<V>{ b, V(f[0].exponent) } } };
    }

        // Given x,b ∊ ℕ, x > 0, b > 1, returns (r,{ {b,e} }) such that x = bᵉ - r with r ≥ 0 minimal.
    static constexpr factorization<V, 1> factorize_ceil(V x, V b)
    {
        Expects(x > 0 && b > 1);
        auto [r, f] = checked_operations<EH, U>::factorize_ceil(U(x), U(b));
        EH::checkOverflow(r <= U(std::numeric_limits<V>::max()));
        return { V(r), { factor<V>{ b, V(f[0].exponent) } } };
    }

        // Computes ⌊log x ÷ log b⌋ for x,b ∊ ℕ, x > 0, b > 1.
    static constexpr V log_floor(V x, V b)
    {
        Expects(x > 0 && b > 1);
        return V(checked_operations<EH, U>::log_floor(U(x), U(b)));
    }

        // Computes ⌈log x ÷ log b⌉ for x,b ∊ ℕ, x > 0, b > 1.
    static constexpr V log_ceil(V x, V b)
    {
        Expects(x > 0 && b > 1);
        return V(checked_operations<EH, U>::log_ceil(U(x), U(b)));
    }

        // Given x,a,b ∊ ℕ, x > 0, a,b > 1, a ≠ b, returns (r,{ {a,i}, {b,j} }) such that x = aⁱ ∙ bʲ - r with r ≥ 0 minimal.
    static constexpr factorization<V, 2> factorize_ceil(V x, V a, V b)
    {
        Expects(x > 0 && a > 1 && b > 1 && a != b);
        auto [r, f] = checked_operations<EH, U>::factorize_ceil(U(x), U(a), U(b));
        EH::checkOverflow(r <= U(std::numeric_limits<V>::max()));
        return { V(r), { factor<V>{ a, V(f[0].exponent) }, factor<V>{ b, V(f[1].exponent) } } };
    }

        // Given x,a,b ∊ ℕ, x > 0, a,b > 1, a ≠ b, returns (r,{ {a,i}, {b,j} }) such that x = aⁱ ∙ bʲ + r with r ≥ 0 minimal.
    static constexpr factorization<V, 2> factorize_floor(V x, V a, V b)
    {
        Expects(x > 0 && a > 1 && b > 1 && a != b);
        auto [r, f] = checked_operations<EH, U>::factorize_floor(U(x), U(a), U(b));
        return { V(r), { factor<V>{ a, V(f[0].exponent) }, factor<V>{ b, V(f[1].exponent) } } };
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
        return V(result);
    }
    static constexpr V multiply(V lhs, V rhs)
    {
        W result = W(lhs) * W(rhs);
        EH::checkOverflow(result <= std::numeric_limits<V>::max());
        return V(result);
    }
    static constexpr bool can_multiply(V lhs, V rhs)
    {
        W result = W(lhs) * W(rhs);
        return result <= std::numeric_limits<V>::max();
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
        return V(result);
    }
    static constexpr V multiply(V lhs, V rhs)
    {
        W result = W(lhs) * W(rhs);
        EH::checkOverflow(result >= std::numeric_limits<V>::min() && result <= std::numeric_limits<V>::max());
        return V(result);
    }
    static constexpr bool can_multiply(V lhs, V rhs)
    {
        W result = W(lhs) * W(rhs);
        return result >= std::numeric_limits<V>::min() && result <= std::numeric_limits<V>::max();
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
        EH::checkOverflow(can_multiply(lhs, rhs));
        return lhs * rhs;
    }
    static constexpr bool can_multiply(V lhs, V rhs)
    {
        return rhs == 0 || lhs <= std::numeric_limits<V>::max() / rhs;
    }
};
template <typename EH, typename V>
    struct checked_2_<EH, V, has_no_wider_type, is_signed> : checked_3_<EH, V, is_signed>
{
    using U = std::make_unsigned_t<V>;

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
        EH::checkOverflow(can_multiply(lhs, rhs));
        return lhs * rhs;
    }
    static constexpr bool can_multiply(V lhs, V rhs)
    {
        return (lhs <= 0 || ((rhs <= 0             || lhs <= std::numeric_limits<V>::max() / rhs)
                          && (rhs >  0             || rhs >= std::numeric_limits<V>::min() / lhs)))
            && (lhs >  0 || ((rhs <= 0             || lhs >= std::numeric_limits<V>::min() / rhs)
                          && (rhs >  0 || lhs == 0 || rhs >= std::numeric_limits<V>::max() / lhs)));
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
