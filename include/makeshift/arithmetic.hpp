
#ifndef INCLUDED_MAKESHIFT_ARITHMETIC_HPP_
#define INCLUDED_MAKESHIFT_ARITHMETIC_HPP_


#include <tuple>
#include <stdexcept> // for runtime_error

#include <gsl/gsl_assert> // for Expects()

#include <makeshift/version.hpp> // for MAKESHIFT_FORCEINLINE

#include <makeshift/detail/arithmetic.hpp> // for checked_operations<>


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


} // namespace detail


inline namespace arithmetic
{


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
    // Computes a ÷ b. Uses `Expects()` to raise error upon overflow or division by 0.
    //
template <typename V>
    constexpr V divide(V n, V d)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::divide(n, d);
}

    //ᅟ
    // Computes a mod b. Uses `Expects()` to raise error upon overflow or division by 0.
    //
template <typename V>
    constexpr V modulo(V n, V d)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::modulo(n, d);
}

    //ᅟ
    // Computes x ∙ 2ⁿ for x,n ∊ ℕ (i.e. left-shifts x by n bits). Uses `Expects()` to raise error upon overflow, or if the number of bits is invalid.
    //
template <typename V>
    constexpr V shift_left(V x, V n)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::shl(x, n);
}

    //ᅟ
    // Computes ⌊x ÷ 2ⁿ⌋ for x,n ∊ ℕ (i.e. right-shifts x by n bits). Uses `Expects()` to raise error if the number of bits is invalid.
    //
template <typename V>
    constexpr V shift_right(V x, V n)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::shr(x, n);
}

    //ᅟ
    // Computes bᵉ for e ∊ ℕ. Uses `Expects()` to raise error if arguments are invalid or if overflow occurs.
    //
template <typename V, typename N>
    constexpr V powi(V b, N e)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::powi(b, e);
}


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
    // Computes a ÷ b. Throws exception upon overflow or division by 0.
    //
template <typename V>
    constexpr V divide_or_throw(V n, V d)
{
    makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::divide(n, d);
}

    //ᅟ
    // Computes a mod b. Throws exception upon overflow or division by 0.
    //
template <typename V>
    constexpr V modulo_or_throw(V n, V d)
{
    makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::modulo(n, d);
}

    //ᅟ
    // Computes x ∙ 2ⁿ for x,n ∊ ℕ (i.e. left-shifts x by n bits). Throws exception upon overflow, or if the number of bits is invalid.
    //
template <typename V>
    constexpr V shift_left_or_throw(V x, V n)
{
    makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::shl(x, n);
}

    //ᅟ
    // Computes ⌊x ÷ 2ⁿ⌋ for x,n ∊ ℕ (i.e. right-shifts x by n bits). Throws exception if the number of bits is invalid.
    //
template <typename V>
    constexpr V shift_right_or_throw(V x, V n)
{
    makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::shr(x, n);
}

    //ᅟ
    // Computes bᵉ for e ∊ ℕ. Throws exception if arguments are invalid or if overflow occurs.
    //
template <typename V, typename N>
    constexpr V powi_or_throw(V b, N e)
{
    makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::powi(b, e);
}


} // inline namespace arithmetic

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_ARITHMETIC_HPP_
