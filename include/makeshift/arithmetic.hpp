
#ifndef INCLUDED_MAKESHIFT_ARITHMETIC_HPP_
#define INCLUDED_MAKESHIFT_ARITHMETIC_HPP_


#include <array>
#include <iosfwd>      // for ostream
#include <stdexcept>   // for runtime_error
#include <type_traits> // for enable_if<>

#include <makeshift/utility.hpp>      // for dim_t
#include <makeshift/type_traits2.hpp> // for type<>
#include <makeshift/metadata2.hpp>    // for reflect_compound_members()
#include <makeshift/version.hpp>      // for MAKESHIFT_NODISCARD, MAKESHIFT_CONSTEXPR_CXX20

#include <makeshift/detail/arithmetic.hpp> // for checked_operations<>


namespace makeshift
{

inline namespace arithmetic
{


template <typename V>
    struct factor
{
    V base;
    V exponent;

        // equivalence
    MAKESHIFT_NODISCARD MAKESHIFT_CONSTEXPR_CXX20 friend bool operator ==(const factor& lhs, const factor& rhs) noexcept
    {
        return lhs.base == rhs.base
            && lhs.exponent == rhs.exponent;
    }
    MAKESHIFT_NODISCARD MAKESHIFT_CONSTEXPR_CXX20 friend bool operator !=(const factor& lhs, const factor& rhs) noexcept
    {
        return !(lhs == rhs);
    }

        // string conversion
    friend std::ostream& operator <<(std::ostream& stream, const factor& f)
    {
        return stream << f.base << '^' << f.exponent;
    }
};
template <typename V>
    constexpr auto reflect(type<factor<V>>)
{
    return reflect_compound_members(
        with_name(&factor<V>::base, "base"),
        with_name(&factor<V>::exponent, "exponent")
    );
}

template <typename V, dim_t NumFactors>
    struct factorization
{
    V remainder;
    std::array<factor<V>, NumFactors> factors;

        // equivalence
    MAKESHIFT_NODISCARD MAKESHIFT_CONSTEXPR_CXX20 friend bool operator ==(const factorization& lhs, const factorization& rhs) noexcept
    {
        return lhs.remainder == rhs.remainder
            && lhs.factors == rhs.factors;
    }
    MAKESHIFT_NODISCARD MAKESHIFT_CONSTEXPR_CXX20 friend bool operator !=(const factorization& lhs, const factorization& rhs) noexcept
    {
        return !(lhs == rhs);
    }

        // string conversion
    friend std::ostream& operator <<(std::ostream& stream, const factorization& f)
    {
        bool first = true;
        for (auto&& factor : f.factors)
        {
            if (first)
                first = false;
            else
                stream << " + ";
            stream << factor;
        }
        if (f.remainder != 0)
            stream << " + " << f.remainder;
        return stream;
    }
};
template <typename V, dim_t NumFactors>
    constexpr auto reflect(type<factorization<V, NumFactors>>)
{
    return reflect_compound_members(
        with_name(&factorization<V, NumFactors>::remainder, "remainder"),
        with_name(&factorization<V, NumFactors>::factors, "factors")
    );
}


class arithmetic_overflow : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};


} // inline namespace arithmetic


namespace detail
{


struct throw_error_handler
{
    static constexpr bool isNoexcept = false;
    static constexpr MAKESHIFT_FORCEINLINE void checkOverflow(bool cond) { if (!cond) throw arithmetic_overflow("integer overflow"); }
    static constexpr MAKESHIFT_FORCEINLINE void checkUnderflow(bool cond) { if (!cond) throw arithmetic_overflow("integer underflow"); }
};
struct assert_error_handler
{
    static constexpr MAKESHIFT_FORCEINLINE void checkOverflow(bool cond) { Expects(cond); }
    static constexpr MAKESHIFT_FORCEINLINE void checkUnderflow(bool cond) { Expects(cond); }
};


} // namespace detail


inline namespace arithmetic
{


    //ᅟ
    // Computes -x. Uses `Expects()` to raise error upon underflow.
    //
template <typename V>
    MAKESHIFT_NODISCARD constexpr V negate(V x)
{
    return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::negate(x);
}

    //ᅟ
    // Computes a + b. Uses `Expects()` to raise error upon overflow.
    //
template <typename V>
    MAKESHIFT_NODISCARD constexpr V add(V a, V b)
{
    return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::add(a, b);
}

    //ᅟ
    // Computes a - b. Uses `Expects()` to raise error upon overflow.
    //
template <typename V>
    MAKESHIFT_NODISCARD constexpr V subtract(V a, V b)
{
    return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::subtract(a, b);
}

    //ᅟ
    // Computes a ∙ b. Uses `Expects()` to raise error upon overflow.
    //
template <typename V>
    MAKESHIFT_NODISCARD constexpr V multiply(V a, V b)
{
    return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::multiply(a, b);
}

    //ᅟ
    // Computes n ÷ d for d ≠ 0. Enforces preconditions with `Expects()`. Uses `Expects()` to raise error upon overflow.
    //
template <typename V>
    MAKESHIFT_NODISCARD constexpr V divide(V n, V d)
{
    return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::divide(n, d);
}

    //ᅟ
    // Computes n mod d for d ≠ 0. Enforces preconditions with `Expects()`. Uses `Expects()` to raise error upon overflow.
    //
template <typename V>
    MAKESHIFT_NODISCARD constexpr V modulo(V n, V d)
{
    return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::modulo(n, d);
}

    //ᅟ
    // Computes x ∙ 2ⁿ for x,n ∊ ℕ₀ (i.e. left-shifts x by n bits). Enforces preconditions with `Expects()`. Uses `Expects()` to raise error upon overflow.
    //
template <typename V>
    MAKESHIFT_NODISCARD constexpr V shift_left(V x, V n)
{
    return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::shl(x, n);
}

    //ᅟ
    // Computes ⌊x ÷ 2ⁿ⌋ for x,n ∊ ℕ₀ (i.e. right-shifts x by n bits). Enforces preconditions with `Expects()`.
    //
template <typename V>
    MAKESHIFT_NODISCARD constexpr V shift_right(V x, V n)
{
    return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::shr(x, n);
}

    //ᅟ
    // Computes bᵉ for e ∊ ℕ₀. Enforces preconditions with `Expects()`. Uses `Expects()` to raise error upon overflow.
    //
template <typename V, typename N>
    MAKESHIFT_NODISCARD constexpr V powi(V b, N e)
{
    return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::powi(b, e);
}

    //ᅟ
    // Computes ⌊n ÷ d⌋ for n ∊ ℕ₀, d ∊ ℕ, d ≠ 0. Enforces preconditions with `Expects()`.
    //
template <typename V>
    MAKESHIFT_NODISCARD constexpr V ratio_floor(V n, V d)
{
    return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::ratio_floor(n, d);
}

    //ᅟ
    // Computes ⌈n ÷ d⌉ for n ∊ ℕ₀, d ∊ ℕ, d ≠ 0. Enforces preconditions with `Expects()`.
    //
template <typename V>
    MAKESHIFT_NODISCARD constexpr V ratio_ceil(V n, V d)
{
    return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::ratio_ceil(n, d);
}

    //ᅟ
    // Computes ⌊log x ÷ log b⌋ for x,b ∊ ℕ, x > 0, b > 1. Enforces preconditions with `Expects()`.
    //
template <typename V>
    MAKESHIFT_NODISCARD constexpr V log_floor(V x, V b)
{
    return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::log_floor(x, b);
}

    //ᅟ
    // Computes ⌈log x ÷ log b⌉ for x,b ∊ ℕ, x > 0, b > 1. Enforces preconditions with `Expects()`.
    //
template <typename V>
    MAKESHIFT_NODISCARD constexpr V log_ceil(V x, V b)
{
    return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::log_ceil(x, b);
}

    //ᅟ
    // Given x,b ∊ ℕ, x > 0, b > 1, returns (r,{ {b,e} }) such that x = bᵉ + r with r ≥ 0 minimal. Enforces preconditions with `Expects()`.
    //
template <typename V>
    constexpr factorization<V, 1> factorize_floor(V x, V b)
{
    return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::factorize_floor(x, b);
}

    //ᅟ
    // Given x,b ∊ ℕ, x > 0, b > 1, returns (r,{ {b,e} }) such that x = bᵉ - r with r ≥ 0 minimal. Enforces preconditions with `Expects()`. Uses `Expects()` to raise error upon overflow.
    //
template <typename V>
    constexpr factorization<V, 1> factorize_ceil(V x, V b)
{
    return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::factorize_ceil(x, b);
}

    //ᅟ
    // Given x,a,b ∊ ℕ, x > 0, a,b > 1, a ≠ b, returns (r,{ {a,i}, {b,j} }) such that x = aⁱ ∙ bʲ - r with r ≥ 0 minimal. Enforces preconditions with `Expects()`.
    //
template <typename V>
    constexpr factorization<V, 2> factorize_floor(V x, V a, V b)
{
    return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::factorize_floor(x, a, b);
}

    //ᅟ
    // Given x,a,b ∊ ℕ, x > 0, a,b > 1, a ≠ b, returns (r,{ {a,i}, {b,j} }) such that x = aⁱ ∙ bʲ - r with r ≥ 0 minimal. Enforces preconditions with `Expects()`. Uses `Expects()` to raise error upon overflow.
    //
template <typename V>
    constexpr factorization<V, 2> factorize_ceil(V x, V a, V b)
{
    return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::factorize_ceil(x, a, b);
}


    //ᅟ
    // Computes -x. Throws exception upon underflow.
    //
template <typename V>
    MAKESHIFT_NODISCARD constexpr V negate_or_throw(V x)
{
    return makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::negate(x);
}

    //ᅟ
    // Computes a + b. Throws exception upon overflow.
    //
template <typename V>
    MAKESHIFT_NODISCARD constexpr V add_or_throw(V a, V b)
{
    return makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::add(a, b);
}

    //ᅟ
    // Computes a - b. Throws exception upon overflow.
    //
template <typename V>
    MAKESHIFT_NODISCARD constexpr V subtract_or_throw(V a, V b)
{
    return makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::subtract(a, b);
}

    //ᅟ
    // Computes a ∙ b. Throws exception upon overflow.
    //
template <typename V>
    V multiply_or_throw(V a, V b)
{
    return makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::multiply(a, b);
}

    //ᅟ
    // Computes n ÷ d for d ≠ 0. Enforces preconditions with `Expects()`. Throws exception upon overflow.
    //
template <typename V>
    MAKESHIFT_NODISCARD constexpr V divide_or_throw(V n, V d)
{
    return makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::divide(n, d);
}

    //ᅟ
    // Computes n mod d for d ≠ 0. Enforces preconditions with `Expects()`. Throws exception upon overflow.
    //
template <typename V>
    MAKESHIFT_NODISCARD constexpr V modulo_or_throw(V n, V d)
{
    return makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::modulo(n, d);
}

    //ᅟ
    // Computes x ∙ 2ⁿ for x,n ∊ ℕ₀ (i.e. left-shifts x by n bits). Enforces preconditions with `Expects()`. Throws exception upon overflow.
    //
template <typename V>
    MAKESHIFT_NODISCARD constexpr V shift_left_or_throw(V x, V n)
{
    return makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::shl(x, n);
}

    //ᅟ
    // Computes bᵉ for e ∊ ℕ₀. Enforces preconditions with `Expects()`. Throws exception upon overflow.
    //
template <typename V, typename N>
    MAKESHIFT_NODISCARD constexpr V powi_or_throw(V b, N e)
{
    return makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::powi(b, e);
}

    //ᅟ
    // Given x,b ∊ ℕ, x > 0, b > 1, returns (r,{ {b,e} }) such that x = bᵉ - r with r ≥ 0 minimal. Enforces preconditions with `Expects()`. Throws exception upon overflow.
    //
template <typename V>
    constexpr factorization<V, 1> factorize_ceil_or_throw(V x, V b)
{
    return makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::factorize_ceil(x, b);
}

    //ᅟ
    // Given x,a,b ∊ ℕ, x > 0, a,b > 1, a ≠ b, computes returns (r,{ {a,i}, {b,j} }) such that x = aⁱ ∙ bʲ - r with r ≥ 0 minimal. Enforces preconditions with `Expects()`. Throws exception upon overflow.
    //
template <typename V>
    constexpr factorization<V, 2> factorize_ceil_or_throw(V x, V a, V b)
{
    return makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::factorize_ceil(x, a, b);
}


    //ᅟ
    // Computes a + b. Uses `Expects()` to raise error upon overflow.
    //
struct plus_checked
{
    template <typename V>
        MAKESHIFT_NODISCARD constexpr V operator ()(V a, V b) const
    {
        return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::add(a, b);
    }
};


    //ᅟ
    // Computes a - b. Uses `Expects()` to raise error upon overflow.
    //
struct minus_checked
{
    template <typename V>
        MAKESHIFT_NODISCARD constexpr V operator ()(V a, V b) const
    {
        return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::subtract(a, b);
    }
};


    //ᅟ
    // Computes a ∙ b. Uses `Expects()` to raise error upon overflow.
    //
struct multiplies_checked
{
    template <typename V>
        MAKESHIFT_NODISCARD constexpr V operator ()(V a, V b) const
    {
        return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::multiply(a, b);
    }
};


    //ᅟ
    // Computes n ÷ d for d ≠ 0. Enforces preconditions with `Expects()`. Uses `Expects()` to raise error upon overflow.
    //
struct divides_checked
{
    template <typename V>
        MAKESHIFT_NODISCARD constexpr V operator ()(V n, V d) const
    {
        return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::divide(n, d);
    }
};


    //ᅟ
    // Computes n mod d for d ≠ 0. Enforces preconditions with `Expects()`. Uses `Expects()` to raise error upon overflow.
    //
struct modulus_checked
{
    template <typename V>
        MAKESHIFT_NODISCARD constexpr V operator ()(V n, V d) const
    {
        return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::modulo(n, d);
    }
};


    //ᅟ
    // Computes a + b. Throws exception upon overflow.
    //
struct plus_or_throw
{
    template <typename V>
        MAKESHIFT_NODISCARD constexpr V operator ()(V a, V b) const
    {
        return makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::add(a, b);
    }
};


    //ᅟ
    // Computes a - b. Throws exception upon overflow.
    //
struct minus_or_throw
{
    template <typename V>
        MAKESHIFT_NODISCARD constexpr V operator ()(V a, V b) const
    {
        return makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::subtract(a, b);
    }
};


    //ᅟ
    // Computes a ∙ b. Throws exception upon overflow.
    //
struct multiplies_or_throw
{
    template <typename V>
        MAKESHIFT_NODISCARD constexpr V operator ()(V a, V b) const
    {
        return makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::multiply(a, b);
    }
};


    //ᅟ
    // Computes n ÷ d for d ≠ 0. Enforces preconditions with `Expects()`. Throws exception upon overflow.
    //
struct divides_or_throw
{
    template <typename V>
        MAKESHIFT_NODISCARD constexpr V operator ()(V n, V d) const
    {
        return makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::divide(n, d);
    }
};


    //ᅟ
    // Computes n mod d for d ≠ 0. Enforces preconditions with `Expects()`. Throws exception upon overflow.
    //
struct modulus_or_throw
{
    template <typename V>
        MAKESHIFT_NODISCARD constexpr V operator ()(V n, V d) const
    {
        return makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::modulo(n, d);
    }
};


} // inline namespace arithmetic

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_ARITHMETIC_HPP_
