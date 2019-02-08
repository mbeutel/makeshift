
#ifndef INCLUDED_MAKESHIFT_DETAIL_ARITHMETIC_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_ARITHMETIC_HPP_


#include <limits>
#include <type_traits> // for is_[un]signed<>, is_integral<>
#include <cstdint>     // for [u]int(8|16|32|64)_t
#include <stdexcept>   // for runtime_error

#include <gsl/gsl_assert> // for Expects()

#include <makeshift/version.hpp>    // for MAKESHIFT_FORCEINLINE


namespace makeshift
{

inline namespace arithmetic
{


template <typename V, std::size_t NumFactors>
    struct factorization;


} // inline namespace arithmetic


namespace detail
{


struct throw_error_handler
{
    static constexpr bool isNoexcept = false;
    static constexpr MAKESHIFT_FORCEINLINE void checkDivByZero(bool cond) { if (!cond) throw arithmetic_div_by_zero_error("division by zero"); }
    static constexpr MAKESHIFT_FORCEINLINE void checkDomain(bool cond) { if (!cond) throw arithmetic_domain_error("invalid shift operand"); }
    static constexpr MAKESHIFT_FORCEINLINE void checkOverflow(bool cond) { if (!cond) throw arithmetic_overflow_error("integer overflow"); }
    static constexpr MAKESHIFT_FORCEINLINE void checkUnderflow(bool cond) { if (!cond) throw arithmetic_overflow_error("integer underflow"); }
    static constexpr MAKESHIFT_FORCEINLINE void checkRange(bool cond) { if (!cond) throw arithmetic_overflow_error("integer out of range"); }
};
struct assert_error_handler
{
    static constexpr MAKESHIFT_FORCEINLINE void checkDivByZero(bool cond) { Expects(cond); }
    static constexpr MAKESHIFT_FORCEINLINE void checkDomain(bool cond) { Expects(cond); }
    static constexpr MAKESHIFT_FORCEINLINE void checkOverflow(bool cond) { Expects(cond); }
    static constexpr MAKESHIFT_FORCEINLINE void checkUnderflow(bool cond) { Expects(cond); }
    static constexpr MAKESHIFT_FORCEINLINE void checkRange(bool cond) { Expects(cond); }
};


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

        // Computes bᵉ for e ∊ ℕ₀.
    template <typename N>
        static constexpr V powi(V b, N e)
    {
        static_assert(std::is_integral<N>::value, "exponent must be an integral type");

            // negative powers are not integral
        EH::checkDomain(e >= 0);

            // conventionally, `powi(0,0)` is 1
        if (e == 0)
            return e != 0 ? 0 : 1;

            // we assume `b > 0` henceforth
        V m = std::numeric_limits<T>::max() / b;

        V result = T(1);
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
        EH::checkDivByZero(d != 0);
        return n / d;
    }

        // Computes ⌈n ÷ d⌉ for n ∊ ℕ₀, d ∊ ℕ.
    static constexpr V ratio_ceil(V n, V d)
    {
        EH::checkDivByZero(d != 0);
        return n != 0
             ? (n - 1) / d + 1 // overflow-safe
             : 0;
    }

        // Given x,b ∊ ℕ, x > 0, b > 1, computes (r,{ e }) such that x = bᵉ + r with r ≥ 0 minimal.
    static constexpr factorization<V, 1> factorize_floor(V x, V b)
    {
        EH::checkDomain(x > 0 && b > 1);

        V e = 0;
        V xp;
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
                return { x - x0, { e } };

            V x1 = x0 * b; // = bᵉ⁺¹
            if (x1 > x)
                return { x - x0, { e } };

            x0 = x1;
            ++e;
        }
    }

        // Given x,b ∊ ℕ, x > 0, b > 1, computes (r,{ e }) such that x = bᵉ - r with r ≥ 0 minimal.
    static constexpr factorization<V, 1> factorize_ceil(V x, V b)
    {
        auto [rFloor, eFloor] = factorize_floor(x, b);
        if (rFloor == 0)
            return { 0, eFloor };
        auto xFloor = x - rFloor;
        auto xCeil = multiply(x, b);
        return { xCeil - x, { eFloor[0] + 1 } }; // eFloor cannot overflow
    }

        // Computes ⌊log x ÷ log b⌋ for x,b ∊ ℕ, x > 0, b > 1.
    static constexpr V log_floor(V x, V b)
    {
        auto result = factorize_floor(x, b);
        return result.exponents[0];
    }

        // Computes ⌈log x ÷ log b⌉ for x,b ∊ ℕ, x > 0, b > 1.
    static constexpr V log_ceil(V x, V b)
    {
        EH::checkDomain(x > 0 && b > 1);

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

        // Given x,a,b ∊ ℕ, x > 0, a,b > 1, a ≠ b, computes (r,{ i,j }) such that x = aⁱ ∙ bʲ - r with r ≥ 0 minimal.
    constexpr factorization<V, 2> factorize_ceil(V x, V a, V b)
    {
        EH::checkDomain(x > 0 && a > 1 && b > 1 && a != b);

            // algorithm discussed in http://stackoverflow.com/a/39050139

        // TODO: check for possible overflow and avoid

        V i = log_ceil(x, a),
          j = 0;
        V ci = i,
          cj = j;
        V y = powi(a, i),
          c = y;

        for (;;)
        {
            while (y < x)
                y *= b, ++j;
            if (y < c)
                c = y, ci = i, cj = j;
            if (i == 0)
                return { c - x, { ci, cj } };
            y /= a, --i;
        }
    }

        // Given x,a,b ∊ ℕ, x > 0, a,b > 1, a ≠ b, computes (r,{ i,j }) such that x = aⁱ ∙ bʲ + r with r ≥ 0 minimal.
    constexpr factorization<V, 2> factorize_floor(V a, V b, V x)
    {
        EH::checkDomain(x > 0 && a > 1 && b > 1 && a != b);

        // TODO: check for possible overflow and avoid

            // adaption of algorithm in factorize_ceil() for different optimisation criterion

        V i = log_floor(x, a),
          j = 0;
        V ci = i,
          cj = j;
        V y = powi(a, i),
          c = y;

        for (;;)
        {
            while (y > x && i > 0)
                y /= 2, --i;
            if (y > c && y <= x)
                c = y, ci = i, cj = j;
            if (i == 0)
                return { x - c, { ci, cj } };
            y *= b, ++j;
        }
    }
};
template <typename EH, typename V>
    struct checked_3_<EH, V, is_signed>
{
    using U = std::make_unsigned_t<V>;

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

        // Computes bᵉ for e ∊ ℕ₀.
    template <typename N>
        static constexpr V powi(V b, N e)
    {
        static_assert(std::is_integral<N>::value, "exponent must be an integral type");

            // negative powers are not integral
        EH::checkDomain(e >= 0);

            // factor out sign if `b` is negative
        V sign = b >= 0
            ? 1
            : 1 - 2 * (e % 2);

            // perform `powi()` for unsigned positive number
        using U = std::make_unsigned_t<V>;
        U absPow = checked_operations<EH, U>::powi(U(b), e);

            // handle special case where result is `numeric_limits<T>::min()`
        if (sign == -1 && absPow == U(std::numeric_limits<V>::max()) + 1)
            return std::numeric_limits<T>::min(); // assuming two's complement

            // convert back to signed, prefix with sign
        EH::checkOverflow(absPow <= std::numeric_limits<V>::max());
        return sign * V(absPow);
    }

        // Computes ⌊n ÷ d⌋ for n ∊ ℕ₀, d ∊ ℕ.
    static constexpr V ratio_floor(V n, V d)
    {
        EH::checkDivByZero(d != 0);
        EH::checkDomain(d > 0 && n >= 0);
        return n / d;
    }

        // Computes ⌈n ÷ d⌉ for n ∊ ℕ₀, d ∊ ℕ.
    static constexpr V ratio_ceil(V n, V d)
    {
        EH::checkDivByZero(d != 0);
        EH::checkDomain(d > 0 && n >= 0);
        return n != 0
             ? (n - 1) / d + 1 // overflow-safe
             : 0;
    }

        // Given x,b ∊ ℕ, x > 0, b > 1, computes (r,{ e }) such that x = bᵉ + r with r ≥ 0 minimal.
    static constexpr factorization<V, 1> factorize_floor(V x, V b)
    {
        EH::checkDomain(x > 0 && b > 1);
        auto [r, e] = checked_operations<EH, U>::factorize_floor(U(x), U(b));
        return { V(r), { V(e[0]) };
    }

        // Given x,b ∊ ℕ, x > 0, b > 1, computes (r,{ e }) such that x = bᵉ - r with r ≥ 0 minimal.
    static constexpr factorization<V, 1> factorize_ceil(V x, V b)
    {
        EH::checkDomain(x > 0 && b > 1);
        auto [r, e] = checked_operations<EH, U>::factorize_ceil(U(x), U(b));
        return { V(r), { V(e[0]) };
    }

        // Computes ⌊log x ÷ log b⌋ for x,b ∊ ℕ, x > 0, b > 1.
    static constexpr V log_floor(V x, V b)
    {
        EH::checkDomain(x > 0 && b > 1);
        return V(checked_operations<EH, U>::log_floor(U(x), U(b)));
    }

        // Computes ⌈log x ÷ log b⌉ for x,b ∊ ℕ, x > 0, b > 1.
    static constexpr V log_ceil(V x, V b)
    {
        EH::checkDomain(x > 0 && b > 1);
        return V(checked_operations<EH, U>::log_ceil(U(x), U(b)));
    }

        // Given x,a,b ∊ ℕ, x > 0, a,b > 1, a ≠ b, computes (r,{ i,j }) such that x = aⁱ ∙ bʲ - r with r ≥ 0 minimal.
    constexpr factorization<V, 2> factorize_ceil(V x, V a, V b)
    {
        EH::checkDomain(x > 0 && a > 1 && b > 1 && a != b);
        auto [r, e] = checked_operations<EH, U>::factorize_ceil(U(x), U(a), U(b));
        return { V(r), { V(e[0]), V(e[1]) } };
    }

        // Given x,a,b ∊ ℕ, x > 0, a,b > 1, a ≠ b, computes (r,{ i,j }) such that x = aⁱ ∙ bʲ + r with r ≥ 0 minimal.
    constexpr factorization<V, 2> factorize_floor(V a, V b, V x)
    {
        EH::checkDomain(x > 0 && a > 1 && b > 1 && a != b);
        auto [r, e] = checked_operations<EH, U>::factorize_floor(U(x), U(a), U(b));
        return { V(r), { V(e[0]), V(e[1]) } };
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
