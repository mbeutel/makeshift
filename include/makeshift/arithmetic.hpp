
#ifndef INCLUDED_MAKESHIFT_ARITHMETIC_HPP_
#define INCLUDED_MAKESHIFT_ARITHMETIC_HPP_


#include <array>
#include <stdexcept>   // for runtime_error
#include <type_traits> // for enable_if<>

#include <makeshift/utility.hpp> // for dim_t
#include <makeshift/version.hpp> // for MAKESHIFT_NODISCARD, MAKESHIFT_CONSTEXPR_CXX20

#include <gsl/span> // for span<>, dynamic_extent


namespace makeshift
{

inline namespace arithmetic
{


    // The implementations below have borrowed heavily from the suggestions made and examples used in the SEI CERT C Coding Standard:
    // https://wiki.sei.cmu.edu/confluence/display/c/

class arithmetic_overflow : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};


template <typename V>
    struct factor
{
    V base;
    V exponent;

    MAKESHIFT_NODISCARD MAKESHIFT_CONSTEXPR_CXX20 friend bool operator ==(const factor& lhs, const factor& rhs) noexcept
    {
        return lhs.base == rhs.base
            && lhs.exponent == rhs.exponent;
    }
    MAKESHIFT_NODISCARD MAKESHIFT_CONSTEXPR_CXX20 friend bool operator !=(const factor& lhs, const factor& rhs) noexcept
    {
        return !(lhs == rhs);
    }
};
template <typename V>
    factor(V, V) -> factor<V>;

template <typename V, dim_t NumFactors = gsl::dynamic_extent>
    struct factorization
{
    V remainder;
    std::array<factor<V>, NumFactors> factors;

    MAKESHIFT_NODISCARD MAKESHIFT_CONSTEXPR_CXX20 friend bool operator ==(const factorization& lhs, const factorization& rhs) noexcept
    {
        return lhs.remainder == rhs.remainder
            && lhs.factors == rhs.factors;
    }
    MAKESHIFT_NODISCARD MAKESHIFT_CONSTEXPR_CXX20 friend bool operator !=(const factorization& lhs, const factorization& rhs) noexcept
    {
        return !(lhs == rhs);
    }
};

template <typename V, dim_t NumFactors = gsl::dynamic_extent>
    struct factorization_view
{
    V remainder;
    gsl::span<const factor<V>, NumFactors> factors;

    template <dim_t OtherNumFactors,
              typename = std::enable_if_t<NumFactors == OtherNumFactors || NumFactors == gsl::dynamic_extent || OtherNumFactors == gsl::dynamic_extent>>
        factorization_view(const factorization<V, OtherNumFactors>& fct)
            : remainder(fct.remainder), factors(fct.factors)
    {
    }
};


} // inline namespace arithmetic

} // namespace makeshift


#include <makeshift/detail/arithmetic.hpp> // for checked_operations<>


namespace makeshift
{

namespace detail
{




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
template <typename V = void>
    struct plus_checked
{
    MAKESHIFT_NODISCARD constexpr V operator ()(V a, V b) const
    {
        return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::add(a, b);
    }
};
template <>
    struct plus_checked<void>
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
template <typename V = void>
    struct minus_checked
{
    MAKESHIFT_NODISCARD constexpr V operator ()(V a, V b) const
    {
        return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::subtract(a, b);
    }
};
template <>
    struct minus_checked<void>
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
template <typename V = void>
    struct multiplies_checked
{
    MAKESHIFT_NODISCARD constexpr V operator ()(V a, V b) const
    {
        return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::multiply(a, b);
    }
};
template <>
    struct multiplies_checked<void>
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
template <typename V = void>
    struct divides_checked
{
    MAKESHIFT_NODISCARD constexpr V operator ()(V n, V d) const
    {
        return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::divide(n, d);
    }
};
template <>
    struct divides_checked<void>
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
template <typename V = void>
    struct modulus_checked
{
    MAKESHIFT_NODISCARD constexpr V operator ()(V n, V d) const
    {
        return makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::modulo(n, d);
    }
};
template <>
    struct modulus_checked<void>
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
template <typename V = void>
    struct plus_or_throw
{
    MAKESHIFT_NODISCARD constexpr V operator ()(V a, V b) const
    {
        return makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::add(a, b);
    }
};
template <>
    struct plus_or_throw<void>
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
template <typename V = void>
    struct minus_or_throw
{
    MAKESHIFT_NODISCARD constexpr V operator ()(V a, V b) const
    {
        return makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::subtract(a, b);
    }
};
template <>
    struct minus_or_throw<void>
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
template <typename V = void>
    struct multiplies_or_throw
{
    MAKESHIFT_NODISCARD constexpr V operator ()(V a, V b) const
    {
        return makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::multiply(a, b);
    }
};
template <>
    struct multiplies_or_throw<void>
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
template <typename V = void>
    struct divides_or_throw
{
    MAKESHIFT_NODISCARD constexpr V operator ()(V n, V d) const
    {
        return makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::divide(n, d);
    }
};
template <>
    struct divides_or_throw<void>
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
template <typename V = void>
    struct modulus_or_throw
{
    MAKESHIFT_NODISCARD constexpr V operator ()(V n, V d) const
    {
        return makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::modulo(n, d);
    }
};
template <>
    struct modulus_or_throw<void>
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
