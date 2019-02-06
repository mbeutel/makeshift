
#ifndef INCLUDED_MAKESHIFT_ARITHMETIC_HPP_
#define INCLUDED_MAKESHIFT_ARITHMETIC_HPP_


#include <stdexcept>   // for runtime_error

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


template <typename V>
    constexpr V powi(V base, V exp)
{
        // conventionally, powi(0,0) == 1
    if (base == 0)
        return exp != 0 ? 0 : 1;

    V result = 1;
    while (exp-- > 0)
        result *= base;
    return result;
}


    //ᅟ
    // Negates an integer. Uses `Expects()` to raise error upon underflow.
    //
template <typename V>
    constexpr V checked_negate(V arg)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::negate(arg);
}

    //ᅟ
    // Adds two integers. Uses `Expects()` to raise error upon overflow.
    //
template <typename V>
    constexpr V checked_add(V lhs, V rhs)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::add(lhs, rhs);
}

    //ᅟ
    // Subtracts two integers. Uses `Expects()` to raise error upon overflow.
    //
template <typename V>
    constexpr V checked_subtract(V lhs, V rhs)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::subtract(lhs, rhs);
}

    //ᅟ
    // Multiplies two integers. Uses `Expects()` to raise error upon overflow.
    //
template <typename V>
    constexpr V checked_multiply(V lhs, V rhs)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::multiply(lhs, rhs);
}

    //ᅟ
    // Divides two integers. Uses `Expects()` to raise error upon overflow or division by 0.
    //
template <typename V>
    constexpr V checked_divide(V num, V den)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::divide(num, den);
}

    //ᅟ
    // Computes the modulus of two integers. Uses `Expects()` to raise error upon overflow or division by 0.
    //
template <typename V>
    constexpr V checked_modulo(V num, V den)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::modulo(num, den);
}

    //ᅟ
    // Left-shifts an integer by the given number of bits. Uses `Expects()` to raise error upon overflow, or if the number of bits is invalid.
    //
template <typename V>
    constexpr V checked_shift_left(V arg, V bits)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::shl(arg, bits);
}

    //ᅟ
    // Right-shifts an integer by the given number of bits. Uses `Expects()` to raise error if the number of bits is invalid.
    //
template <typename V>
    constexpr V checked_shift_right(V arg, V bits)
{
    makeshift::detail::checked_operations<makeshift::detail::assert_error_handler, V>::shr(arg, bits);
}


    //ᅟ
    // Negates an integer. Throws an exception upon underflow.
    //
template <typename V>
    constexpr V checked_negate_or_throw(V arg)
{
    makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::negate(arg);
}

    //ᅟ
    // Adds two integers. Throws an exception upon overflow.
    //
template <typename V>
    constexpr V checked_add_or_throw(V lhs, V rhs)
{
    makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::add(lhs, rhs);
}

    //ᅟ
    // Subtracts two integers. Throws an exception upon overflow.
    //
template <typename V>
    constexpr V checked_subtract_or_throw(V lhs, V rhs)
{
    makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::subtract(lhs, rhs);
}

    //ᅟ
    // Multiplies two integers. Throws an exception upon overflow.
    //
template <typename V>
    V checked_multiply_or_throw(V lhs, V rhs)
{
    makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::multiply(lhs, rhs);
}

    //ᅟ
    // Divides two integers. Throws an exception upon overflow or division by 0.
    //
template <typename V>
    constexpr V checked_divide_or_throw(V num, V den)
{
    makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::divide(num, den);
}

    //ᅟ
    // Computes the modulus of two integers. Throws an exception upon overflow or division by 0.
    //
template <typename V>
    constexpr V checked_modulo_or_throw(V num, V den)
{
    makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::modulo(num, den);
}

    //ᅟ
    // Left-shifts an integer by the given number of bits. Throws an exception upon overflow, or if the number of bits is invalid.
    //
template <typename V>
    constexpr V checked_shift_left_or_throw(V arg, V bits)
{
    makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::shl(arg, bits);
}

    //ᅟ
    // Right-shifts an integer by the given number of bits. Throws an exception if the number of bits is invalid.
    //
template <typename V>
    constexpr V checked_shift_right_or_throw(V arg, V bits)
{
    makeshift::detail::checked_operations<makeshift::detail::throw_error_handler, V>::shr(arg, bits);
}


} // inline namespace arithmetic

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_ARITHMETIC_HPP_
