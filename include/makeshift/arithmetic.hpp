
#ifndef INCLUDED_MAKESHIFT_ARITHMETIC_HPP_
#define INCLUDED_MAKESHIFT_ARITHMETIC_HPP_


#include <array>
#include <iosfwd>       // for ostream
#include <exception>    // for terminate()
#include <stdexcept>    // for runtime_error
#include <type_traits>  // for integral_constant
#include <system_error> // for errc

#include <gsl/gsl-lite.hpp> // for Expects()

#include <makeshift/version.hpp>  // for MAKESHIFT_NODISCARD, MAKESHIFT_CONSTEXPR_CXX20

#include <makeshift/detail/arithmetic.hpp>


namespace makeshift
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

template <typename V, int NumFactors>
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


template <typename T>
    struct arithmetic_result
{
    T value;
    std::errc ec;
};


    //ᅟ
    // Computes -v.
    // Uses `Expects()` to raise error upon underflow.
    //
template <typename V>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::integral_value_type<V>
    negate(V v)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<V>, "argument must be convertible to an integral type");

    return makeshift::detail::negate<makeshift::detail::assert_error_handler>(v);
}

    //ᅟ
    // Computes -v.
    // Returns error code `std::errc::value_too_large` upon underflow.
    //
template <typename V>
    MAKESHIFT_NODISCARD constexpr arithmetic_result<makeshift::detail::integral_value_type<V>>
    try_negate(V v)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<V>, "argument must be convertible to an integral type");

    return makeshift::detail::negate<makeshift::detail::try_error_handler>(v);
}

    //ᅟ
    // Computes -v.
    // Throws `std::system_error` upon underflow.
    //
template <typename V>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::integral_value_type<V>
    negate_or_throw(V v)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<V>, "argument must be convertible to an integral type");

    return makeshift::detail::negate<makeshift::detail::throw_error_handler>(v);
}


    //ᅟ
    // Computes a + b.
    // Uses `Expects()` to raise error upon overflow.
    //
template <typename A, typename B>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::common_integral_value_type<A, B>
    add(A a, B b)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<A, B>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<A, B>, "argument types must have identical signedness");

    return makeshift::detail::add<makeshift::detail::assert_error_handler>(a, b);
}

    //ᅟ
    // Computes a + b.
    // Returns error code `std::errc::value_too_large` upon overflow.
    //
template <typename A, typename B>
    MAKESHIFT_NODISCARD constexpr arithmetic_result<makeshift::detail::common_integral_value_type<A, B>>
    try_add(A a, B b)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<A, B>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<A, B>, "argument types must have identical signedness");

    return makeshift::detail::add<makeshift::detail::try_error_handler>(a, b);
}

    //ᅟ
    // Computes a + b.
    // Throws `std::system_error` upon overflow.
    //
template <typename A, typename B>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::common_integral_value_type<A, B>
    add_or_throw(A a, B b)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<A, B>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<A, B>, "argument types must have identical signedness");

    return makeshift::detail::add<makeshift::detail::throw_error_handler>(a, b);
}


    //ᅟ
    // Computes a - b.
    // Uses `Expects()` to raise error upon overflow.
    //
template <typename A, typename B>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::common_integral_value_type<A, B>
    subtract(A a, B b)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<A, B>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<A, B>, "argument types must have identical signedness");

    return makeshift::detail::subtract<makeshift::detail::assert_error_handler>(a, b);
}

    //ᅟ
    // Computes a - b.
    // Returns error code `std::errc::value_too_large` upon overflow.
    //
template <typename A, typename B>
    MAKESHIFT_NODISCARD constexpr arithmetic_result<makeshift::detail::common_integral_value_type<A, B>>
    try_subtract(A a, B b)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<A, B>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<A, B>, "argument types must have identical signedness");

    return makeshift::detail::subtract<makeshift::detail::try_error_handler>(a, b);
}

    //ᅟ
    // Computes a - b.
    // Throws `std::system_error` upon overflow.
    //
template <typename A, typename B>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::common_integral_value_type<A, B>
    subtract_or_throw(A a, B b)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<A, B>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<A, B>, "argument types must have identical signedness");

    return makeshift::detail::subtract<makeshift::detail::throw_error_handler>(a, b);
}


    //ᅟ
    // Computes a ∙ b.
    // Uses `Expects()` to raise error upon overflow.
    //
template <typename A, typename B>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::common_integral_value_type<A, B>
    multiply(A a, B b)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<A, B>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<A, B>, "argument types must have identical signedness");

    return makeshift::detail::multiply<makeshift::detail::assert_error_handler>(a, b);
}

    //ᅟ
    // Computes a ∙ b.
    // Returns error code `std::errc::value_too_large` upon overflow.
    //
template <typename A, typename B>
    MAKESHIFT_NODISCARD constexpr arithmetic_result<makeshift::detail::common_integral_value_type<A, B>>
    try_multiply(A a, B b)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<A, B>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<A, B>, "argument types must have identical signedness");

    return makeshift::detail::multiply<makeshift::detail::try_error_handler>(a, b);
}

    //ᅟ
    // Computes a ∙ b.
    // Throws `std::system_error` upon overflow.
    //
template <typename A, typename B>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::common_integral_value_type<A, B>
    multiply_or_throw(A a, B b)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<A, B>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<A, B>, "argument types must have identical signedness");

    return makeshift::detail::multiply<makeshift::detail::throw_error_handler>(a, b);
}


    //ᅟ
    // Computes n ÷ d for d ≠ 0.
    // Enforces preconditions with `Expects()`. Uses `Expects()` to raise error upon overflow.
    //
template <typename N, typename D>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::common_integral_value_type<N, D>
    divide(N n, D d)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<N, D>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<N, D>, "argument types must have identical signedness");

    return makeshift::detail::divide<makeshift::detail::assert_error_handler>(n, d);
}

    //ᅟ
    // Computes n ÷ d for d ≠ 0.
    // Enforces preconditions with `Expects()`. Returns error code `std::errc::value_too_large` upon overflow.
    //
template <typename N, typename D>
    MAKESHIFT_NODISCARD constexpr arithmetic_result<makeshift::detail::common_integral_value_type<N, D>>
    try_divide(N n, D d)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<N, D>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<N, D>, "argument types must have identical signedness");

    return makeshift::detail::divide<makeshift::detail::try_error_handler>(n, d);
}

    //ᅟ
    // Computes n ÷ d for d ≠ 0.
    // Enforces preconditions with `Expects()`. Throws `std::system_error` upon overflow.
    //
template <typename N, typename D>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::common_integral_value_type<N, D>
    divide_or_throw(N n, D d)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<N, D>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<N, D>, "argument types must have identical signedness");

    return makeshift::detail::divide<makeshift::detail::throw_error_handler>(n, d);
}


    //ᅟ
    // Computes n mod d for d ≠ 0.
    // Enforces preconditions with `Expects()`. Uses `Expects()` to raise error upon overflow.
    //
template <typename N, typename D>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::common_integral_value_type<N, D>
    modulo(N n, D d)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<N, D>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<N, D>, "argument types must have identical signedness");

    return makeshift::detail::modulo<makeshift::detail::assert_error_handler>(n, d);
}

    //ᅟ
    // Computes n mod d for d ≠ 0.
    // Enforces preconditions with `Expects()`. Returns error code `std::errc::value_too_large` upon overflow.
    //
template <typename N, typename D>
    MAKESHIFT_NODISCARD constexpr arithmetic_result<makeshift::detail::common_integral_value_type<N, D>>
    try_modulo(N n, D d)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<N, D>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<N, D>, "argument types must have identical signedness");

    return makeshift::detail::modulo<makeshift::detail::try_error_handler>(n, d);
}

    //ᅟ
    // Computes n mod d for d ≠ 0.
    // Enforces preconditions with `Expects()`. Throws `std::system_error` upon overflow.
    //
template <typename N, typename D>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::common_integral_value_type<N, D>
    modulo_or_throw(N n, D d)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<N, D>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<N, D>, "argument types must have identical signedness");

    return makeshift::detail::modulo<makeshift::detail::throw_error_handler>(n, d);
}


    //ᅟ
    // Computes x ∙ 2ˢ for x,s ∊ ℕ₀ (i.e. left-shifts x by s bits).
    // Enforces preconditions with `Expects()`. Uses `Expects()` to raise error upon overflow.
    //
template <typename X, typename S>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::common_integral_value_type<X, S>
    shift_left(X x, S s)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<X, S>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<X, S>, "argument types must have identical signedness");

    return makeshift::detail::shift_left<makeshift::detail::assert_error_handler>(x, s);
}

    //ᅟ
    // Computes x ∙ 2ˢ for x,s ∊ ℕ₀ (i.e. left-shifts x by s bits).
    // Enforces preconditions with `Expects()`. Returns error code `std::errc::value_too_large` upon overflow.
    //
template <typename X, typename S>
    MAKESHIFT_NODISCARD constexpr arithmetic_result<makeshift::detail::common_integral_value_type<X, S>>
    try_shift_left(X x, S s)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<X, S>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<X, S>, "argument types must have identical signedness");

    return makeshift::detail::shift_left<makeshift::detail::try_error_handler>(x, s);
}

    //ᅟ
    // Computes x ∙ 2ˢ for x,s ∊ ℕ₀ (i.e. left-shifts x by s bits).
    // Enforces preconditions with `Expects()`. Throws `std::system_error` upon overflow.
    //
template <typename X, typename S>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::common_integral_value_type<X, S>
    shift_left_or_throw(X x, S s)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<X, S>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<X, S>, "argument types must have identical signedness");

    return makeshift::detail::shift_left<makeshift::detail::throw_error_handler>(x, s);
}


    //ᅟ
    // Computes ⌊x ÷ 2ˢ⌋ for x,s ∊ ℕ₀ (i.e. right-shifts x by s bits).
    // Enforces preconditions with `Expects()`. Uses `Expects()` to raise error upon overflow.
    //
template <typename X, typename S>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::common_integral_value_type<X, S>
    shift_right(X x, S s)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<X, S>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<X, S>, "argument types must have identical signedness");

    return makeshift::detail::shift_right<makeshift::detail::assert_error_handler>(x, s);
}

    //ᅟ
    // Computes ⌊x ÷ 2ˢ⌋ for x,s ∊ ℕ₀ (i.e. right-shifts x by s bits).
    // Enforces preconditions with `Expects()`. Returns error code `std::errc::value_too_large` upon overflow.
    //
template <typename X, typename S>
    MAKESHIFT_NODISCARD constexpr arithmetic_result<makeshift::detail::common_integral_value_type<X, S>>
    try_shift_right(X x, S s)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<X, S>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<X, S>, "argument types must have identical signedness");

    return makeshift::detail::shift_right<makeshift::detail::try_error_handler>(x, s);
}

    //ᅟ
    // Computes ⌊x ÷ 2ˢ⌋ for x,s ∊ ℕ₀ (i.e. right-shifts x by s bits).
    // Enforces preconditions with `Expects()`. Throws `std::system_error` upon overflow.
    //
template <typename X, typename S>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::common_integral_value_type<X, S>
    shift_right_or_throw(X x, S s)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<X, S>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<X, S>, "argument types must have identical signedness");

    return makeshift::detail::shift_right<makeshift::detail::throw_error_handler>(x, s);
}


    //ᅟ
    // Computes bᵉ for e ∊ ℕ₀.
    // Enforces preconditions with `Expects()`. Uses `Expects()` to raise error upon overflow.
    //
template <typename B, typename E>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::common_integral_value_type<B, E>
    powi(B b, E e)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<B, E>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<B, E>, "argument types must have identical signedness");

    return makeshift::detail::powi<makeshift::detail::assert_error_handler>(b, e);
}

    //ᅟ
    // Computes bᵉ for e ∊ ℕ₀.
    // Enforces preconditions with `Expects()`. Returns error code `std::errc::value_too_large` upon overflow.
    //
template <typename B, typename E>
    MAKESHIFT_NODISCARD constexpr arithmetic_result<makeshift::detail::common_integral_value_type<B, E>>
    try_powi(B b, E e)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<B, E>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<B, E>, "argument types must have identical signedness");

    return makeshift::detail::powi<makeshift::detail::try_error_handler>(b, e);
}

    //ᅟ
    // Computes bᵉ for e ∊ ℕ₀.
    // Enforces preconditions with `Expects()`. Throws `std::system_error` upon overflow.
    //
template <typename B, typename E>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::common_integral_value_type<B, E>
    powi_or_throw(B b, E e)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<B, E>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<B, E>, "argument types must have identical signedness");

    return makeshift::detail::powi<makeshift::detail::throw_error_handler>(b, e);
}


    //ᅟ
    // Computes ⌊x ÷ d⌋ ∙ d for x ∊ ℕ₀, d ∊ ℕ, d ≠ 0.
    // Enforces preconditions with `Expects()`.
    //
template <typename X, typename D>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::common_integral_value_type<X, D>
    floori(X x, D d)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<X, D>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<X, D>, "argument types must have identical signedness");

    return makeshift::detail::floori(x, d);
}


    //ᅟ
    // Computes ⌈x ÷ d⌉ ∙ d for x ∊ ℕ₀, d ∊ ℕ, d ≠ 0.
    // Enforces preconditions with `Expects()`. Uses `Expects()` to raise error upon overflow.
    //
template <typename X, typename D>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::common_integral_value_type<X, D>
    ceili(X x, D d)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<X, D>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<X, D>, "argument types must have identical signedness");

    return makeshift::detail::ceili<makeshift::detail::assert_error_handler>(x, d);
}

    //ᅟ
    // Computes xᵉ for d ∊ ℕ₀.
    // Enforces preconditions with `Expects()`. Returns error code `std::errc::value_too_large` upon overflow.
    //
template <typename X, typename D>
    MAKESHIFT_NODISCARD constexpr arithmetic_result<makeshift::detail::common_integral_value_type<X, D>>
    try_ceili(X x, D d)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<X, D>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<X, D>, "argument types must have identical signedness");

    return makeshift::detail::ceili<makeshift::detail::try_error_handler>(x, d);
}

    //ᅟ
    // Computes xᵉ for d ∊ ℕ₀.
    // Enforces preconditions with `Expects()`. Throws `std::system_error` upon overflow.
    //
template <typename X, typename D>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::common_integral_value_type<X, D>
    ceili_or_throw(X x, D d)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<X, D>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<X, D>, "argument types must have identical signedness");

    return makeshift::detail::ceili<makeshift::detail::throw_error_handler>(x, d);
}


    //ᅟ
    // Computes ⌊n ÷ d⌋ for n ∊ ℕ₀, d ∊ ℕ, d ≠ 0.
    // Enforces preconditions with `Expects()`.
    //
template <typename N, typename D>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::common_integral_value_type<N, D>
    ratio_floori(N n, D d)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<N, D>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<N, D>, "argument types must have identical signedness");

    return makeshift::detail::ratio_floori(n, d);
}


    //ᅟ
    // Computes ⌈n ÷ d⌉ for n ∊ ℕ₀, d ∊ ℕ, d ≠ 0.
    // Enforces preconditions with `Expects()`. Uses `Expects()` to raise error upon overflow.
    //
template <typename N, typename D>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::common_integral_value_type<N, D>
    ratio_ceili(N n, D d)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<N, D>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<N, D>, "argument types must have identical signedness");

    return makeshift::detail::ratio_ceili<makeshift::detail::assert_error_handler>(n, d);
}

    //ᅟ
    // Computes ⌈n ÷ d⌉ for n ∊ ℕ₀, d ∊ ℕ, d ≠ 0.
    // Enforces preconditions with `Expects()`. Returns error code `std::errc::value_too_large` upon overflow.
    //
template <typename N, typename D>
    MAKESHIFT_NODISCARD constexpr arithmetic_result<makeshift::detail::common_integral_value_type<N, D>>
    try_ratio_ceili(N n, D d)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<N, D>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<N, D>, "argument types must have identical signedness");

    return makeshift::detail::ratio_ceili<makeshift::detail::try_error_handler>(n, d);
}

    //ᅟ
    // Computes ⌈n ÷ d⌉ for n ∊ ℕ₀, d ∊ ℕ, d ≠ 0.
    // Enforces preconditions with `Expects()`. Throws `std::system_error` upon overflow.
    //
template <typename N, typename D>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::common_integral_value_type<N, D>
    ratio_ceili_or_throw(N n, D d)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<N, D>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<N, D>, "argument types must have identical signedness");

    return makeshift::detail::ratio_ceili<makeshift::detail::throw_error_handler>(n, d);
}


    //ᅟ
    // Computes ⌊log x ÷ log b⌋ for x,b ∊ ℕ, x > 0, b > 1.
    // Enforces preconditions with `Expects()`.
    //
template <typename X, typename B>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::common_integral_value_type<X, B>
    log_floori(X x, B b)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<X, B>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<X, B>, "argument types must have identical signedness");

    return makeshift::detail::log_floori(x, b);
}


    //ᅟ
    // Computes ⌈log x ÷ log b⌉ for x,b ∊ ℕ, x > 0, b > 1.
    // Enforces preconditions with `Expects()`.
    //
template <typename X, typename B>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::common_integral_value_type<X, B>
    log_ceili(X x, B b)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<X, B>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<X, B>, "argument types must have identical signedness");

    return makeshift::detail::log_ceili(x, b);
}


    //ᅟ
    // Given x,b ∊ ℕ, x > 0, b > 1, returns (r,{ {b,e} }) such that x = bᵉ + r with r ≥ 0 minimal.
    // Enforces preconditions with `Expects()`.
    //
template <typename X, typename B>
    MAKESHIFT_NODISCARD constexpr factorization<makeshift::detail::common_integral_value_type<X, B>, 1>
    factorize_floori(X x, B b)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<X, B>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<X, B>, "argument types must have identical signedness");

    return makeshift::detail::factorize_floori(x, b);
}


    //ᅟ
    // Given x,b ∊ ℕ, x > 0, b > 1, returns (r,{ {b,e} }) such that x = bᵉ - r with r ≥ 0 minimal.
    // Enforces preconditions with `Expects()`. Uses `Expects()` to raise error upon overflow.
    //
template <typename X, typename B>
    MAKESHIFT_NODISCARD constexpr factorization<makeshift::detail::common_integral_value_type<X, B>, 1>
    factorize_ceili(X x, B b)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<X, B>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<X, B>, "argument types must have identical signedness");

    return makeshift::detail::factorize_ceili<makeshift::detail::assert_error_handler>(x, b);
}

    //ᅟ
    // Given x,b ∊ ℕ, x > 0, b > 1, returns (r,{ {b,e} }) such that x = bᵉ - r with r ≥ 0 minimal.
    // Enforces preconditions with `Expects()`. Returns error code `std::errc::value_too_large` upon overflow.
    //
template <typename X, typename B>
    MAKESHIFT_NODISCARD constexpr factorization<arithmetic_result<makeshift::detail::common_integral_value_type<X, B>>, 1>
    try_factorize_ceili(X x, B b)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<X, B>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<X, B>, "argument types must have identical signedness");

    return makeshift::detail::factorize_ceili<makeshift::detail::try_error_handler>(x, b);
}

    //ᅟ
    // Given x,b ∊ ℕ, x > 0, b > 1, returns (r,{ {b,e} }) such that x = bᵉ - r with r ≥ 0 minimal.
    // Enforces preconditions with `Expects()`. Throws `std::system_error` upon overflow.
    //
template <typename X, typename B>
    MAKESHIFT_NODISCARD constexpr factorization<makeshift::detail::common_integral_value_type<X, B>, 1>
    factorize_ceili_or_throw(X x, B b)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<X, B>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<X, B>, "argument types must have identical signedness");

    return makeshift::detail::factorize_ceili<makeshift::detail::throw_error_handler>(x, b);
}


    //ᅟ
    // Given x,a,b ∊ ℕ, x > 0, a,b > 1, a ≠ b, returns (r,{ {a,i}, {b,j} }) such that x = aⁱ ∙ bʲ + r with r ≥ 0 minimal.
    // Enforces preconditions with `Expects()`.
    //
template <typename X, typename A, typename B>
    MAKESHIFT_NODISCARD constexpr factorization<makeshift::detail::common_integral_value_type<X, A, B>, 2>
    factorize_floori(X x, A a, B b)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<X, A, B>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<X, A, B>, "argument types must have identical signedness");

    return makeshift::detail::factorize_floori(x, a, b);
}


    //ᅟ
    // Given x,a,b ∊ ℕ, x > 0, a,b > 1, a ≠ b, returns (r,{ {a,i}, {b,j} }) such that x = aⁱ ∙ bʲ - r with r ≥ 0 minimal.
    // Enforces preconditions with `Expects()`. Uses `Expects()` to raise error upon overflow.
    //
template <typename X, typename A, typename B>
    MAKESHIFT_NODISCARD constexpr factorization<makeshift::detail::common_integral_value_type<X, A, B>, 2>
    factorize_ceili(X x, A a, B b)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<X, A, B>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<X, A, B>, "argument types must have identical signedness");

    return makeshift::detail::factorize_ceili<makeshift::detail::assert_error_handler>(x, a, b);
}

    //ᅟ
    // Given x,a,b ∊ ℕ, x > 0, a,b > 1, a ≠ b, returns (r,{ {a,i}, {b,j} }) such that x = aⁱ ∙ bʲ - r with r ≥ 0 minimal.
    // Enforces preconditions with `Expects()`. Returns error code `std::errc::value_too_large` upon overflow.
    //
template <typename X, typename A, typename B>
    MAKESHIFT_NODISCARD constexpr factorization<arithmetic_result<makeshift::detail::common_integral_value_type<X, A, B>>, 2>
    try_factorize_ceili(X x, A a, B b)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<X, A, B>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<X, A, B>, "argument types must have identical signedness");

    return makeshift::detail::factorize_ceili<makeshift::detail::try_error_handler>(x, a, b);
}

    //ᅟ
    // Given x,a,b ∊ ℕ, x > 0, a,b > 1, a ≠ b, returns (r,{ {a,i}, {b,j} }) such that x = aⁱ ∙ bʲ - r with r ≥ 0 minimal.
    // Enforces preconditions with `Expects()`. Throws `std::system_error` upon overflow.
    //
template <typename X, typename A, typename B>
    MAKESHIFT_NODISCARD constexpr factorization<makeshift::detail::common_integral_value_type<X, A, B>, 2>
    factorize_ceili_or_throw(X x, A a, B b)
{
    static_assert(makeshift::detail::are_integral_arithmetic_types_v<X, A, B>, "arguments must be convertible to an integral type");
    static_assert(makeshift::detail::have_same_signedness_v<X, A, B>, "argument types must have identical signedness");

    return makeshift::detail::factorize_ceili<makeshift::detail::throw_error_handler>(x, a, b);
}


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_ARITHMETIC_HPP_
