
#ifndef INCLUDED_MAKESHIFT_ARITHMETIC_HPP_
#define INCLUDED_MAKESHIFT_ARITHMETIC_HPP_


#include <array>


namespace makeshift
{

inline namespace arithmetic
{


    // The implementations below have borrowed heavily from the suggestions made and examples used in the SEI CERT C Coding Standard:
    // https://wiki.sei.cmu.edu/confluence/display/c/

class arithmetic_error : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};
class arithmetic_overflow_error : public arithmetic_error
{
public:
    using arithmetic_error::arithmetic_error;
};
class arithmetic_div_by_zero_error : public arithmetic_error
{
public:
    using arithmetic_error::arithmetic_error;
};
class arithmetic_domain_error : public arithmetic_error
{
public:
    using arithmetic_error::arithmetic_error;
};


template <typename V, std::size_t NumFactors>
    struct factorization
{
    V remainder;
    std::array<V, NumFactors> exponents;
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
    // Computes a + b. Uses `Expects()` to raise error upon overflow.
    //
template <typename V = void>
    struct plus_checked
{
    constexpr V operator ()(V a, V b) const
    {
        makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::add(a, b);
    }
};
template <>
    struct plus_checked<void>
{
    template <typename V>
        constexpr V operator ()(V a, V b) const
    {
        makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::add(a, b);
    }
};


    //ᅟ
    // Computes a - b. Uses `Expects()` to raise error upon overflow.
    //
template <typename V = void>
    struct minus_checked
{
    constexpr V operator ()(V a, V b) const
    {
        makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::subtract(a, b);
    }
};
template <>
    struct minus_checked<void>
{
    template <typename V>
        constexpr V operator ()(V a, V b) const
    {
        makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::subtract(a, b);
    }
};


    //ᅟ
    // Computes a ∙ b. Uses `Expects()` to raise error upon overflow.
    //
template <typename V = void>
    struct multiplies_checked
{
    constexpr V operator ()(V a, V b) const
    {
        makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::multiply(a, b);
    }
};
template <>
    struct multiplies_checked<void>
{
    template <typename V>
        constexpr V operator ()(V a, V b) const
    {
        makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::multiply(a, b);
    }
};


    //ᅟ
    // Computes n ÷ d for d ≠ 0. Uses `Expects()` to raise error upon overflow or division by 0.
    //
template <typename V = void>
    struct divides_checked
{
    constexpr V operator ()(V n, V d) const
    {
        makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::divide(n, d);
    }
};
template <>
    struct divides_checked<void>
{
    template <typename V>
        constexpr V operator ()(V n, V d) const
    {
        makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::divide(n, d);
    }
};


    //ᅟ
    // Computes n mod d for d ≠ 0. Uses `Expects()` to raise error upon overflow or division by 0.
    //
template <typename V = void>
    struct modulus_checked
{
    constexpr V operator ()(V n, V d) const
    {
        makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::modulo(n, d);
    }
};
template <>
    struct modulus_checked<void>
{
    template <typename V>
        constexpr V operator ()(V n, V d) const
    {
        makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::modulo(n, d);
    }
};


    //ᅟ
    // Computes -x. Uses `Expects()` to raise error upon underflow.
    //
template <typename V>
    constexpr V negate(V x)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::negate(x);
}

    //ᅟ
    // Computes a + b. Uses `Expects()` to raise error upon overflow.
    //
template <typename V>
    constexpr V add(V a, V b)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::add(a, b);
}

    //ᅟ
    // Computes a - b. Uses `Expects()` to raise error upon overflow.
    //
template <typename V>
    constexpr V subtract(V a, V b)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::subtract(a, b);
}

    //ᅟ
    // Computes a ∙ b. Uses `Expects()` to raise error upon overflow.
    //
template <typename V>
    constexpr V multiply(V a, V b)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::multiply(a, b);
}

    //ᅟ
    // Computes n ÷ d for d ≠ 0. Uses `Expects()` to raise error upon overflow or division by 0.
    //
template <typename V>
    constexpr V divide(V n, V d)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::divide(n, d);
}

    //ᅟ
    // Computes n mod d for d ≠ 0. Uses `Expects()` to raise error upon overflow or division by 0.
    //
template <typename V>
    constexpr V modulo(V n, V d)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::modulo(n, d);
}

    //ᅟ
    // Computes x ∙ 2ⁿ for x,n ∊ ℕ₀ (i.e. left-shifts x by n bits). Uses `Expects()` to raise error upon overflow, or if the number of bits is invalid.
    //
template <typename V>
    constexpr V shift_left(V x, V n)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::shl(x, n);
}

    //ᅟ
    // Computes ⌊x ÷ 2ⁿ⌋ for x,n ∊ ℕ₀ (i.e. right-shifts x by n bits). Uses `Expects()` to raise error if the number of bits is invalid.
    //
template <typename V>
    constexpr V shift_right(V x, V n)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::shr(x, n);
}

    //ᅟ
    // Computes bᵉ for e ∊ ℕ₀. Uses `Expects()` to raise error if arguments are invalid or if overflow occurs.
    //
template <typename V, typename N>
    constexpr V powi(V b, N e)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::powi(b, e);
}

    //ᅟ
    // Computes ⌊n ÷ d⌋ for n ∊ ℕ₀, d ∊ ℕ, d ≠ 0. Uses `Expects()` to raise error if arguments are invalid.
    //
template <typename V>
    constexpr V ratio_floor(V n, V d)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::powi(b, e);
}

    //ᅟ
    // Computes ⌈n ÷ d⌉ for n ∊ ℕ₀, d ∊ ℕ, d ≠ 0. Uses `Expects()` to raise error if arguments are invalid.
    //
template <typename V>
    constexpr V ratio_ceil(V n, V d)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::powi(b, e);
}

    //ᅟ
    // Computes ⌊log x ÷ log b⌋ for x,b ∊ ℕ, x > 0, b > 1. Uses `Expects()` to raise error if arguments are invalid.
    //
template <typename V>
    constexpr V log_floor(V x, V b)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::factorize_floor(x, b);
}

    //ᅟ
    // Computes ⌈log x ÷ log b⌉ for x,b ∊ ℕ, x > 0, b > 1. Uses `Expects()` to raise error if arguments are invalid.
    //
template <typename V>
    constexpr V log_ceil(V x, V b)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::factorize_floor(x, b);
}

    //ᅟ
    // Given x,b ∊ ℕ, x > 0, b > 1, computes (r,{ e }) such that x = bᵉ + r with r ≥ 0 minimal. Uses `Expects()` to raise error if arguments are invalid.
    //
template <typename V>
    constexpr factorization<V, 1> factorize_floor(V x, V b)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::factorize_floor(x, b);
}

    //ᅟ
    // Given x,b ∊ ℕ, x > 0, b > 1, computes (r,{ e }) such that x = bᵉ - r with r ≥ 0 minimal. Uses `Expects()` to raise error if arguments are invalid.
    //
template <typename V>
    constexpr factorization<V, 1> factorize_ceil(V x, V b)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::factorize_ceil(x, b);
}

    //ᅟ
    // Given x,a,b ∊ ℕ, x > 0, a,b > 1, a ≠ b, computes (r,{ i,j }) such that x = aⁱ ∙ bʲ - r with r ≥ 0 minimal. Uses `Expects()` to raise error if arguments are invalid.
    //
template <typename V>
    constexpr factorization<V, 2> factorize_floor(V x, V a, V b)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::factorize_floor(x, a, b);
}

    //ᅟ
    // Given x,a,b ∊ ℕ, x > 0, a,b > 1, a ≠ b, computes (r,{ i,j }) such that x = aⁱ ∙ bʲ - r with r ≥ 0 minimal. Uses `Expects()` to raise error if arguments are invalid.
    //
template <typename V>
    constexpr factorization<V, 2> factorize_ceil(V x, V a, V b)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::factorize_ceil(x, a, b);
}


    // TODO: all `*_or_throw()` functions should only throw upon overflow, not on domain error.

    //ᅟ
    // Computes -x. Throws exception upon underflow.
    //
template <typename V>
    constexpr V negate_or_throw(V x)
{
    makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::negate(x);
}

    //ᅟ
    // Computes a + b. Throws exception upon overflow.
    //
template <typename V>
    constexpr V add_or_throw(V a, V b)
{
    makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::add(a, b);
}

    //ᅟ
    // Computes a - b. Throws exception upon overflow.
    //
template <typename V>
    constexpr V subtract_or_throw(V a, V b)
{
    makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::subtract(a, b);
}

    //ᅟ
    // Computes a ∙ b. Throws exception upon overflow.
    //
template <typename V>
    V multiply_or_throw(V a, V b)
{
    makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::multiply(a, b);
}

    //ᅟ
    // Computes n ÷ d for d ≠ 0. Throws exception upon overflow or division by 0.
    //
template <typename V>
    constexpr V divide_or_throw(V n, V d)
{
    makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::divide(n, d);
}

    //ᅟ
    // Computes n mod d for d ≠ 0. Throws exception upon overflow or division by 0.
    //
template <typename V>
    constexpr V modulo_or_throw(V n, V d)
{
    makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::modulo(n, d);
}

    //ᅟ
    // Computes x ∙ 2ⁿ for x,n ∊ ℕ₀ (i.e. left-shifts x by n bits). Throws exception upon overflow, or if the number of bits is invalid.
    //
template <typename V>
    constexpr V shift_left_or_throw(V x, V n)
{
    makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::shl(x, n);
}

    //ᅟ
    // Computes ⌊x ÷ 2ⁿ⌋ for x,n ∊ ℕ₀ (i.e. right-shifts x by n bits). Throws exception if the number of bits is invalid.
    //
template <typename V>
    constexpr V shift_right_or_throw(V x, V n)
{
    makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::shr(x, n);
}

    //ᅟ
    // Computes bᵉ for e ∊ ℕ₀. Throws exception if arguments are invalid or if overflow occurs.
    //
template <typename V, typename N>
    constexpr V powi_or_throw(V b, N e)
{
    makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::powi(b, e);
}


} // inline namespace arithmetic

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_ARITHMETIC_HPP_
