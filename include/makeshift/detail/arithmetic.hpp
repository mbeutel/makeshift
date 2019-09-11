
#ifndef INCLUDED_MAKESHIFT_DETAIL_ARITHMETIC_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_ARITHMETIC_HPP_


#include <limits>
#include <cstdint>      // for [u]int(8|16|32|64)_t
#include <exception>    // for terminate()
#include <type_traits>  // for integral_constant<>, make_unsigned<>, is_signed<>, is_integral<>, is_same<>
#include <system_error> // for errc, system_error

#include <gsl/gsl-lite.hpp> // for Expects()

#include <makeshift/macros.hpp> // for MAKESHIFT_FORCEINLINE


#if defined(_MSC_VER) && !defined(__clang__)
 #pragma warning( push )
 #pragma warning( disable: 4702 ) // unreachable code
#endif // defined(_MSC_VER) && !defined(__clang__)


namespace makeshift
{


template <typename V>
    struct factor;
template <typename V, int NumFactors>
    struct factorization;
template <typename T>
    struct arithmetic_result;


namespace detail
{


struct unreachable_wildcard_t
{
    template <typename T>
        [[noreturn]] operator T(void) const noexcept
    {
        std::terminate(); // unreachable
    }
};

struct errc_wildcard_t
{
    std::errc ec;

    template <typename T>
        constexpr operator arithmetic_result<T>(void) const noexcept
    {
        return { { }, ec };
    }
};

struct try_error_handler
{
    template <typename T> using result = arithmetic_result<T>;
    
    template <typename T>
        static constexpr MAKESHIFT_FORCEINLINE arithmetic_result<T> make_result(T value) noexcept
    {
        return { value, std::errc{ } };
    }
    static constexpr inline errc_wildcard_t make_error(std::errc ec) noexcept
    {
        return { ec };
    }
    template <typename T>
        static constexpr MAKESHIFT_FORCEINLINE T get_value(arithmetic_result<T> const& result) noexcept
    {
        return result.value;
    }
    template <typename T>
        static constexpr MAKESHIFT_FORCEINLINE bool is_error(arithmetic_result<T> const& result) noexcept
    {
        return result.ec != std::errc{ };
    }
    template <typename T>
        static constexpr MAKESHIFT_FORCEINLINE errc_wildcard_t passthrough_error(arithmetic_result<T> const& result) noexcept
    {
        return { result.ec };
    }
};

struct throw_error_handler
{
    template <typename T> using result = T;
    
    template <typename T>
        static constexpr MAKESHIFT_FORCEINLINE T make_result(T value) noexcept
    {
        return value;
    }
    [[noreturn]] static inline unreachable_wildcard_t make_error(std::errc ec)
    {
        throw std::system_error(std::make_error_code(ec));
    }
    template <typename T>
        static constexpr MAKESHIFT_FORCEINLINE T get_value(T result) noexcept
    {
        return result;
    }
    template <typename T>
        static constexpr MAKESHIFT_FORCEINLINE std::false_type is_error(T) noexcept
    {
        return { };
    }
    template <typename T>
        static constexpr MAKESHIFT_FORCEINLINE unreachable_wildcard_t passthrough_error(T) noexcept
    {
        std::terminate();
    }
};

struct assert_error_handler
{
    template <typename T> using result = T;
    
    template <typename T>
        static constexpr MAKESHIFT_FORCEINLINE T make_result(T value) noexcept
    {
        return value;
    }
    [[noreturn]] static inline unreachable_wildcard_t make_error(std::errc) noexcept
    {
        std::terminate();
    }
    template <typename T>
        static constexpr MAKESHIFT_FORCEINLINE T get_value(T result) noexcept
    {
        return result;
    }
    template <typename T>
        static constexpr MAKESHIFT_FORCEINLINE std::false_type is_error(T) noexcept
    {
        return { };
    }
    template <typename T>
        static constexpr MAKESHIFT_FORCEINLINE unreachable_wildcard_t passthrough_error(T) noexcept
    {
        std::terminate();
    }
};


template <typename V> struct integral_value_type_ { using type = V; };
template <typename V, V Value> struct integral_value_type_<std::integral_constant<V, Value>> { using type = V; };
template <typename V> using integral_value_type = typename integral_value_type_<V>::type;

template <typename... Vs> using common_integral_value_type = std::common_type_t<typename integral_value_type_<Vs>::type...>;

template <typename V> struct is_nonbool_integral : std::integral_constant<bool, std::is_integral<V>::value && !std::is_same<V, bool>::value> { };

    // avoid pulling in type_traits.hpp
template <bool V0, typename T0, typename... Ts> struct mconjunction_ { using type = T0; };
template <typename T0, typename T1, typename... Ts> struct mconjunction_<true, T0, T1, Ts...> : mconjunction_<T1::value, T1, Ts...> { };
template <typename T0, typename... Ts> struct mconjunction : makeshift::detail::mconjunction_<T0::value, T0, Ts...>::type { };

template <typename... Vs> constexpr bool are_integral_arithmetic_types_v = mconjunction<is_nonbool_integral<Vs>...>::value;

template <bool Signed, typename... Vs> constexpr bool have_same_signedness_0_v = mconjunction<std::integral_constant<bool, std::is_signed<integral_value_type<Vs>>::value == Signed>...>::value;
template <typename... Vs> constexpr bool have_same_signedness_v = false;
template <> constexpr bool have_same_signedness_v<> = true;
template <typename V0, typename... Vs> constexpr bool have_same_signedness_v<V0, Vs...> = have_same_signedness_0_v<std::is_signed<integral_value_type<V0>>::value, Vs...>;

template <typename V> struct wider_type_;
template <typename V> using wider_type = typename wider_type_<V>::type;
template <> struct wider_type_<std::int8_t> { using type = std::int32_t; };
template <> struct wider_type_<std::int16_t> { using type = std::int32_t; };
template <> struct wider_type_<std::int32_t> { using type = std::int64_t; };
template <> struct wider_type_<std::uint8_t> { using type = std::uint32_t; };
template <> struct wider_type_<std::uint16_t> { using type = std::uint32_t; };
template <> struct wider_type_<std::uint32_t> { using type = std::uint64_t; };
template <typename V> constexpr bool has_wider_type_v = sizeof(V) <= sizeof(std::uint32_t);
template <typename V> struct has_wider_type : std::integral_constant<bool, has_wider_type_v<V>> { };

template <typename EH, typename V> using result_t = typename EH::template result<V>;


    // The implementations below have borrowed heavily from the suggestions made and examples used in the SEI CERT C Coding Standard:
    // https://wiki.sei.cmu.edu/confluence/display/c/


template <typename EH, typename V>
    constexpr result_t<EH, integral_value_type<V>> negate_unsigned(V v)
{
    using V0 = integral_value_type<V>;

    if (v != 0) return EH::make_error(std::errc::value_too_large);
    return EH::make_result(V0(0));
}
template <typename EH, typename V>
    constexpr result_t<EH, integral_value_type<V>> negate_signed(V v)
{
    using V0 = integral_value_type<V>;

        // This assumes a two's complement representation (it will yield a false negative for one's complement integers).
    if (v == std::numeric_limits<V0>::min()) return EH::make_error(std::errc::value_too_large);
    return EH::make_result(V0(-v));
}
#if MAKESHIFT_CXXLEVEL < 17
template <typename EH, typename V>
    constexpr result_t<EH, integral_value_type<V>> negate_0(std::false_type /*isSigned*/, V v)
{
    return negate_unsigned<EH>(v);
}
template <typename EH, typename V>
    constexpr result_t<EH, integral_value_type<V>> negate_0(std::true_type /*isSigned*/, V v)
{
    return negate_signed<EH>(v);
}
#endif // MAKESHIFT_CXXLEVEL < 17
template <typename EH, typename V>
    constexpr result_t<EH, integral_value_type<V>> negate(V v)
{
    using V0 = integral_value_type<V>;

#if MAKESHIFT_CXXLEVEL >= 17
    if constexpr (std::is_signed_v<V0>)
    {
        return negate_signed<EH>(v);
    }
    else
    {
        return negate_unsigned<EH>(v);
    }
#else // MAKESHIFT_CXXLEVEL >= 17
    return negate_0<EH>(std::is_signed<V0>{ }, v);
#endif // MAKESHIFT_CXXLEVEL >= 17
}


template <typename EH, typename A, typename B>
    constexpr result_t<EH, common_integral_value_type<A, B>> add_narrow(A a, B b)
{
    using V = common_integral_value_type<A, B>;
    using W = wider_type<V>;

    W result = W(a) + W(b);
    if (result < std::numeric_limits<V>::min() || result > std::numeric_limits<V>::max()) return EH::make_error(std::errc::value_too_large);
    return EH::make_result(V(result));
}
template <typename EH, typename A, typename B>
    constexpr result_t<EH, common_integral_value_type<A, B>> add_wide_unsigned(A a, B b)
{
    using V = common_integral_value_type<A, B>;

    V result = a + b;
    if (result < a || result < b) return EH::make_error(std::errc::value_too_large);
    return EH::make_result(V(result));
}
template <typename EH, typename A, typename B>
    constexpr result_t<EH, common_integral_value_type<A, B>> add_wide_signed(A a, B b)
{
    using V = common_integral_value_type<A, B>;
    using U = std::make_unsigned_t<V>;

    V result = V(U(a) + U(b));
    if ((a <  0 && b <  0 && result >= 0)
     || (a >= 0 && b >= 0 && result <  0))
    {
        return EH::make_error(std::errc::value_too_large);
    }
    return EH::make_result(V(result));
}
#if MAKESHIFT_CXXLEVEL < 17
template <typename EH, typename BC, typename A, typename B>
    constexpr result_t<EH, common_integral_value_type<A, B>> add_0(std::true_type /*isNarrow*/, BC /*isSigned*/, A a, B b)
{
    return add_narrow<EH>(a, b);
}
template <typename EH, typename A, typename B>
    constexpr result_t<EH, common_integral_value_type<A, B>> add_0(std::false_type /*isNarrow*/, std::false_type /*isSigned*/, A a, B b)
{
    return add_wide_unsigned<EH>(a, b);
}
template <typename EH, typename A, typename B>
    constexpr result_t<EH, common_integral_value_type<A, B>> add_0(std::false_type /*isNarrow*/, std::true_type /*isSigned*/, A a, B b)
{
    return add_wide_signed<EH>(a, b);
}
#endif // MAKESHIFT_CXXLEVEL < 17
template <typename EH, typename A, typename B>
    constexpr result_t<EH, common_integral_value_type<A, B>> add(A a, B b)
{
    using V = common_integral_value_type<A, B>;

#if MAKESHIFT_CXXLEVEL >= 17
    if constexpr (has_wider_type_v<V>)
    {
        return add_narrow<EH>(a, b);
    }
    else if constexpr (std::is_signed_v<V>)
    {
        return add_wide_signed<EH>(a, b);
    }
    else
    {
        return add_wide_unsigned<EH>(a, b);
    }
#else // MAKESHIFT_CXXLEVEL >= 17
    return add_0<EH>(has_wider_type<V>{ }, std::is_signed<V>{ }, a, b);
#endif // MAKESHIFT_CXXLEVEL >= 17
}


template <typename EH, typename A, typename B>
    constexpr result_t<EH, common_integral_value_type<A, B>> subtract_unsigned(A a, B b)
{
    using V = common_integral_value_type<A, B>;

    if (a < b) return EH::make_error(std::errc::value_too_large);
    return EH::make_result(V(a - b));
}
template <typename EH, typename A, typename B>
    constexpr result_t<EH, common_integral_value_type<A, B>> subtract_signed(A a, B b)
{
    using V = common_integral_value_type<A, B>;

    if ((b > 0 && a < std::numeric_limits<V>::min() + b)
     || (b < 0 && a > std::numeric_limits<V>::max() + b))
    {
        return EH::make_error(std::errc::value_too_large);
    }
    return EH::make_result(V(a - b));
}
#if MAKESHIFT_CXXLEVEL < 17
template <typename EH, typename A, typename B>
    constexpr result_t<EH, common_integral_value_type<A, B>> subtract_0(std::false_type /*isSigned*/, A a, B b)
{
    return subtract_unsigned<EH>(a, b);
}
template <typename EH, typename A, typename B>
    constexpr result_t<EH, common_integral_value_type<A, B>> subtract_0(std::true_type /*isSigned*/, A a, B b)
{
    return subtract_signed<EH>(a, b);
}
#endif // MAKESHIFT_CXXLEVEL < 17
template <typename EH, typename A, typename B>
    constexpr result_t<EH, common_integral_value_type<A, B>> subtract(A a, B b)
{
    using V = common_integral_value_type<A, B>;

#if MAKESHIFT_CXXLEVEL >= 17
    if constexpr (std::is_signed_v<V>)
    {
        return subtract_signed<EH>(a, b);
    }
    else
    {
        return subtract_unsigned<EH>(a, b);
    }
#else // MAKESHIFT_CXXLEVEL >= 17
    return subtract_0<EH>(std::is_signed<V>{ }, a, b);
#endif // MAKESHIFT_CXXLEVEL >= 17
}


template <typename A, typename B>
    constexpr bool can_multiply_narrow(A a, B b) noexcept
{
    using V = common_integral_value_type<A, B>;
    using W = wider_type<V>;

    W result = W(a) * W(b);
    return result >= std::numeric_limits<V>::min() && result <= std::numeric_limits<V>::max();
}
template <typename A, typename B>
    constexpr bool can_multiply_wide_unsigned(A a, B b) noexcept
{
    using V = common_integral_value_type<A, B>;

    return b == 0 || a <= std::numeric_limits<V>::max() / b;
}
template <typename A, typename B>
    constexpr bool can_multiply_wide_signed(A a, B b) noexcept
{
    using V = common_integral_value_type<A, B>;

    return (a <= 0 || ((b <= 0           || a <= std::numeric_limits<V>::max() / b)
                    && (b >  0           || b >= std::numeric_limits<V>::min() / a)))
        && (a >  0 || ((b <= 0           || a >= std::numeric_limits<V>::min() / b)
                    && (b >  0 || a == 0 || b >= std::numeric_limits<V>::max() / a)));
}
#if MAKESHIFT_CXXLEVEL < 17
template <typename BC, typename A, typename B>
    constexpr bool can_multiply_0(std::true_type /*isNarrow*/, BC /*isSigned*/, A a, B b)
{
    return can_multiply_narrow(a, b);
}
template <typename A, typename B>
    constexpr bool can_multiply_0(std::false_type /*isNarrow*/, std::false_type /*isSigned*/, A a, B b)
{
    return can_multiply_wide_unsigned(a, b);
}
template <typename A, typename B>
    constexpr bool can_multiply_0(std::false_type /*isNarrow*/, std::true_type /*isSigned*/, A a, B b)
{
    return can_multiply_wide_signed(a, b);
}
#endif // MAKESHIFT_CXXLEVEL < 17
template <typename A, typename B>
    constexpr bool can_multiply(A a, B b)
{
    using V = common_integral_value_type<A, B>;

#if MAKESHIFT_CXXLEVEL >= 17
    if constexpr (has_wider_type_v<V>)
    {
        return can_multiply_narrow(a, b);
    }
    else if constexpr (std::is_signed_v<V>)
    {
        return can_multiply_wide_signed(a, b);
    }
    else
    {
        return can_multiply_wide_unsigned(a, b);
    }
#else // MAKESHIFT_CXXLEVEL >= 17
    return can_multiply_0(has_wider_type<V>{ }, std::is_signed<V>{ }, a, b);
#endif // MAKESHIFT_CXXLEVEL >= 17
}

template <typename EH, typename A, typename B>
    constexpr result_t<EH, common_integral_value_type<A, B>> multiply_narrow(A a, B b)
{
    using V = common_integral_value_type<A, B>;
    using W = wider_type<V>;

    W result = W(a) * W(b);
    if (result < std::numeric_limits<V>::min() || result > std::numeric_limits<V>::max()) return EH::make_error(std::errc::value_too_large);
    return EH::make_result(V(result));
}
template <typename EH, typename A, typename B>
    constexpr result_t<EH, common_integral_value_type<A, B>> multiply_wide_unsigned(A a, B b)
{
    using V = common_integral_value_type<A, B>;

    if (!can_multiply_wide_unsigned(a, b)) return EH::make_error(std::errc::value_too_large);
    return EH::make_result(V(a * b));
}
template <typename EH, typename A, typename B>
    constexpr result_t<EH, common_integral_value_type<A, B>> multiply_wide_signed(A a, B b)
{
    using V = common_integral_value_type<A, B>;

    if (!can_multiply_wide_signed(a, b)) return EH::make_error(std::errc::value_too_large);
    return EH::make_result(V(a * b));
}
#if MAKESHIFT_CXXLEVEL < 17
template <typename EH, typename BC, typename A, typename B>
    constexpr result_t<EH, common_integral_value_type<A, B>> multiply_0(std::true_type /*isNarrow*/, BC /*isSigned*/, A a, B b)
{
    return multiply_narrow<EH>(a, b);
}
template <typename EH, typename A, typename B>
    constexpr result_t<EH, common_integral_value_type<A, B>> multiply_0(std::false_type /*isNarrow*/, std::false_type /*isSigned*/, A a, B b)
{
    return multiply_wide_unsigned<EH>(a, b);
}
template <typename EH, typename A, typename B>
    constexpr result_t<EH, common_integral_value_type<A, B>> multiply_0(std::false_type /*isNarrow*/, std::true_type /*isSigned*/, A a, B b)
{
    return multiply_wide_signed<EH>(a, b);
}
#endif // MAKESHIFT_CXXLEVEL < 17
template <typename EH, typename A, typename B>
    constexpr result_t<EH, common_integral_value_type<A, B>> multiply(A a, B b)
{
    using V = common_integral_value_type<A, B>;

#if MAKESHIFT_CXXLEVEL >= 17
    if constexpr (has_wider_type_v<V>)
    {
        return multiply_narrow<EH>(a, b);
    }
    else if constexpr (std::is_signed_v<V>)
    {
        return multiply_wide_signed<EH>(a, b);
    }
    else
    {
        return multiply_wide_unsigned<EH>(a, b);
    }
#else // MAKESHIFT_CXXLEVEL >= 17
    return multiply_0<EH>(has_wider_type<V>{ }, std::is_signed<V>{ }, a, b);
#endif // MAKESHIFT_CXXLEVEL >= 17
}


template <typename EH, typename N, typename D>
    constexpr result_t<EH, common_integral_value_type<N, D>> divide_unsigned(N n, D d)
{
    using V = common_integral_value_type<N, D>;

    Expects(d != 0);

    return EH::make_result(V(n / d));
}
template <typename EH, typename N, typename D>
    constexpr result_t<EH, common_integral_value_type<N, D>> divide_signed(N n, D d)
{
    using V = common_integral_value_type<N, D>;

    Expects(d != 0);

    if (n == std::numeric_limits<V>::min() && d == -1) return EH::make_error(std::errc::value_too_large);
    return EH::make_result(V(n / d));
}
#if MAKESHIFT_CXXLEVEL < 17
template <typename EH, typename N, typename D>
    constexpr result_t<EH, common_integral_value_type<N, D>> divide_0(std::false_type /*isSigned*/, N n, D d)
{
    return divide_unsigned<EH>(n, d);
}
template <typename EH, typename N, typename D>
    constexpr result_t<EH, common_integral_value_type<N, D>> divide_0(std::true_type /*isSigned*/, N n, D d)
{
    return divide_signed<EH>(n, d);
}
#endif // MAKESHIFT_CXXLEVEL < 17
template <typename EH, typename N, typename D>
    constexpr result_t<EH, common_integral_value_type<N, D>> divide(N n, D d)
{
    using V = common_integral_value_type<N, D>;

#if MAKESHIFT_CXXLEVEL >= 17
    if constexpr (std::is_signed_v<V>)
    {
        return divide_signed<EH>(n, d);
    }
    else
    {
        return divide_unsigned<EH>(n, d);
    }
#else // MAKESHIFT_CXXLEVEL >= 17
    return divide_0<EH>(std::is_signed<V>{ }, n, d);
#endif // MAKESHIFT_CXXLEVEL >= 17
}


template <typename EH, typename N, typename D>
    constexpr result_t<EH, common_integral_value_type<N, D>> modulo_unsigned(N n, D d)
{
    using V = common_integral_value_type<N, D>;

    Expects(d != 0);

    return EH::make_result(V(n % d));
}
template <typename EH, typename N, typename D>
    constexpr result_t<EH, common_integral_value_type<N, D>> modulo_signed(N n, D d)
{
    using V = common_integral_value_type<N, D>;

    Expects(d != 0);

    if (n == std::numeric_limits<V>::min() && d == -1) return EH::make_error(std::errc::value_too_large);
    return EH::make_result(V(n % d));
}
#if MAKESHIFT_CXXLEVEL < 17
template <typename EH, typename N, typename D>
    constexpr result_t<EH, common_integral_value_type<N, D>> modulo_0(std::false_type /*isSigned*/, N n, D d)
{
    return modulo_unsigned<EH>(n, d);
}
template <typename EH, typename N, typename D>
    constexpr result_t<EH, common_integral_value_type<N, D>> modulo_0(std::true_type /*isSigned*/, N n, D d)
{
    return modulo_signed<EH>(n, d);
}
#endif // MAKESHIFT_CXXLEVEL < 17
template <typename EH, typename N, typename D>
    constexpr result_t<EH, common_integral_value_type<N, D>> modulo(N n, D d)
{
    using V = common_integral_value_type<N, D>;

#if MAKESHIFT_CXXLEVEL >= 17
    if constexpr (std::is_signed_v<V>)
    {
        return modulo_signed<EH>(n, d);
    }
    else
    {
        return modulo_unsigned<EH>(n, d);
    }
#else // MAKESHIFT_CXXLEVEL >= 17
    return modulo_0<EH>(std::is_signed<V>{ }, n, d);
#endif // MAKESHIFT_CXXLEVEL >= 17
}


template <typename EH, typename X, typename S>
    constexpr result_t<EH, integral_value_type<X>> shift_right(X x, S s)
{
    using V0 = integral_value_type<X>;

        // Note that we fail when shifting negative integers.
    Expects(x >= 0 && s >= 0);

    if (s >= sizeof(X)*8) return EH::make_error(std::errc::value_too_large);
    return EH::make_result(V0(x >> s));
}


template <typename EH, typename X, typename S>
    constexpr result_t<EH, integral_value_type<X>> shift_left(X x, S s)
{
    using V0 = integral_value_type<X>;

        // Note that we fail when shifting negative integers.
    Expects(x >= 0 && s >= 0);

    if (s >= sizeof(V0)*8
     || x > (std::numeric_limits<V0>::max() >> s))
    {
        return EH::make_error(std::errc::value_too_large);
    }
    return EH::make_result(V0(x << s));
}


    // Computes ⌊x ÷ d⌋ ∙ d for x ∊ ℕ₀, d ∊ ℕ, d ≠ 0.
template <typename X, typename D>
    constexpr common_integral_value_type<X, D> floori(X x, D d)
{
    Expects(x >= 0 && d > 0);

    return x - x % d;
}


    // Computes ⌈x ÷ d⌉ ∙ d for x ∊ ℕ₀, d ∊ ℕ, d ≠ 0.
template <typename EH, typename X, typename D>
    constexpr result_t<EH, common_integral_value_type<X, D>> ceili(X x, D d)
{
    using V = common_integral_value_type<X, D>;

    Expects(x >= 0 && d > 0);

        // We have the following identities:
        //
        //     x = ⌊x ÷ d⌋ ∙ d + x mod d
        //     ⌈x ÷ d⌉ = ⌊(x + d - 1) ÷ d⌋ = ⌊(x - 1) ÷ d⌋ + 1
        //
        // Assuming x ≠ 0, we can derive the form
        //
        //     ⌈x ÷ d⌉ ∙ d = x + d - (x - 1) mod d - 1 .

    return x != 0
        ? add<EH>(x, d - (x - 1) % d - 1)
        : EH::make_result(V(0));
}


    // Computes ⌊n ÷ d⌋ for n ∊ ℕ₀, d ∊ ℕ, d ≠ 0.
template <typename N, typename D>
    constexpr common_integral_value_type<N, D> ratio_floori(N n, D d)
{
    Expects(n >= 0 && d > 0);
        
    return n / d;
}


    // Computes ⌈n ÷ d⌉ for n ∊ ℕ₀, d ∊ ℕ, d ≠ 0.
template <typename N, typename D>
    constexpr common_integral_value_type<N, D> ratio_ceili(N n, D d)
{
    Expects(n >= 0 && d > 0);
        
    return n != 0
        ? (n - 1) / d + 1 // overflow-safe
        : 0;
}


    // Computes bᵉ for e ∊ ℕ₀.
template <typename EH, typename B, typename E>
    static constexpr result_t<EH, integral_value_type<B>> powi_unsigned(B b, E e)
{
    using V = integral_value_type<B>;

        // Conventionally, `powi(0,0)` is 1.
    if (e == 0)
    {
        return EH::make_result(V(e != 0 ? 0 : 1));
    }
    if (b == 0)
    {
        return EH::make_result(V(0));
    }

        // We assume `b > 0` henceforth.
    V m = std::numeric_limits<V>::max() / b;

    V result = 1;
    integral_value_type<E> remainingExponents = e;
    while (remainingExponents > 0)
    {
            // Ensure the multiplication cannot overflow.
        if (result > m) return EH::make_error(std::errc::value_too_large);
        result *= b;
        --remainingExponents;
    }
    return EH::make_result(result);
}
    // Computes bᵉ for e ∊ ℕ₀.
template <typename EH, typename B, typename E>
    static constexpr result_t<EH, integral_value_type<B>> powi_signed(B b, E e)
{
    using V = integral_value_type<B>;

        // In the special case of `b` assuming the smallest representable value, `sign * b` would overflow,
        // so we need to handle it separately.
    if (b == std::numeric_limits<V>::min())
    {
        if (e > 1) return EH::make_error(std::errc::value_too_large); // `powi()` would overflow for exponents greater than 1
        return EH::make_result(V(e == 0 ? V(1) : b));
    }

        // Factor out sign.
    V sign = b >= 0
        ? 1
        : e % 2 == 0
            ? 1
            : -1;

        // Compute `powi()` for unsigned positive number.
    using U = std::make_unsigned_t<V>;
    U absPow = powi_unsigned<EH>(U(sign * b), e);

        // Handle special case where result is smallest representable value.
    if (sign == -1 && absPow == U(std::numeric_limits<V>::max()) + 1)
    {
        return EH::make_result(std::numeric_limits<V>::min()); // assuming two's complement
    }

        // Convert back to signed and prefix with sign.
    if (absPow > U(std::numeric_limits<V>::max())) return EH::make_error(std::errc::value_too_large);
    return EH::make_result(V(sign * V(absPow)));
}
#if MAKESHIFT_CXXLEVEL < 17
template <typename EH, typename B, typename E>
    constexpr result_t<EH, integral_value_type<B>> powi_0(std::false_type /*isSigned*/, B b, E e)
{
    return powi_unsigned<EH>(b, e);
}
template <typename EH, typename B, typename E>
    constexpr result_t<EH, integral_value_type<B>> powi_0(std::true_type /*isSigned*/, B b, E e)
{
    return powi_signed<EH>(b, e);
}
#endif // MAKESHIFT_CXXLEVEL < 17
template <typename EH, typename B, typename E>
    constexpr result_t<EH, integral_value_type<B>> powi(B b, E e)
{
    using V = integral_value_type<B>;

        // Negative powers are not integral.
    Expects(e >= 0);

#if MAKESHIFT_CXXLEVEL >= 17
    if constexpr (std::is_signed_v<V>)
    {
        return powi_signed<EH>(b, e);
    }
    else
    {
        return powi_unsigned<EH>(b, e);
    }
#else // MAKESHIFT_CXXLEVEL >= 17
    return powi_0<EH>(std::is_signed<V>{ }, b, e);
#endif // MAKESHIFT_CXXLEVEL >= 17
}


    // Given x,b ∊ ℕ, x > 0, b > 1, returns (r,{ {b,e} }) such that x = bᵉ + r with r ≥ 0 minimal.
template <typename X, typename B>
    constexpr factorization<common_integral_value_type<X, B>, 1> factorize_floori(X x, B b)
{
    using V = common_integral_value_type<X, B>;

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
        {
            return { x - x0, { factor<V>{ b, e } } };
        }

        V x1 = x0 * b; // = bᵉ⁺¹
        if (x1 > x)
        {
            return { x - x0, { factor<V>{ b, e } } };
        }

        x0 = x1;
        ++e;
    }
}


    // Given x,b ∊ ℕ, x > 0, b > 1, returns (r,{ {b,e} }) such that x = bᵉ - r with r ≥ 0 minimal.
template <typename EH, typename X, typename B>
    constexpr result_t<EH, factorization<common_integral_value_type<X, B>, 1>> factorize_ceili(X x, B b)
{
    using V = common_integral_value_type<X, B>;

    Expects(x > 0 && b > 1);

    auto floorFac = factorize_floori(x, b);
    if (floorFac.remainder == 0)
    {
        return EH::make_result(factorization<V, 1>{ 0, floorFac.factors });
    }

    auto xFloor = x - floorFac.remainder; // = bᵉ
    auto prodResult = multiply<EH>(xFloor, b - 1);
    if (EH::is_error(prodResult)) return EH::passthrough_error(prodResult);
    auto prod = EH::get_value(prodResult);
        
    auto rCeil = prod - floorFac.remainder; // x = bᵉ + r =: bᵉ⁺¹ - r' ⇒ r' = bᵉ(b - 1) - r
    return EH::make_result(factorization<V, 1>{
        rCeil, { factor<V>{ b, floorFac.factors[0].exponent + 1 } } // e cannot overflow
    });
}


    // Computes ⌊log x ÷ log b⌋ for x,b ∊ ℕ, x > 0, b > 1.
template <typename X, typename B>
    constexpr common_integral_value_type<X, B> log_floori(X x, B b)
{
    using V = common_integral_value_type<X, B>;

    auto fac = factorize_floori(x, b);
    return fac.factors[0].exponent;
}


    // Computes ⌊log x ÷ log b⌋ for x,b ∊ ℕ, x > 0, b > 1.
template <typename X, typename B>
    constexpr common_integral_value_type<X, B> log_ceili(X x, B b)
{
    using V = common_integral_value_type<X, B>;

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
                // bᵉ > m implies bᵉ⁺¹ > M (cf. reasoning in factorize_floori()).
                // Because x ≤ M, ⌈log x ÷ log b⌉ = e + 1.
            return e + 1;
        }

        x0 *= b;
        ++e;
    }

    return e;
}


    // Given x,a,b ∊ ℕ, x > 0, a,b > 1, a ≠ b, returns (r,{ {a,i}, {b,j} }) such that x = aⁱ ∙ bʲ + r with r ≥ 0 minimal.
template <typename X, typename A, typename B>
    constexpr factorization<common_integral_value_type<X, A, B>, 2> factorize_floori(X x, A a, B b)
{
    using V = common_integral_value_type<X, A, B>;

    Expects(x > 0 && a > 1 && b > 1 && a != b);

        // adaption of algorithm in factorize_ceili() for different optimization criterion

    auto facA = factorize_floori(x, a);
        
    V i = facA.factors[0].exponent,
      j = 0;
    V ci = i,
      cj = j;
    V y = x - facA.remainder; // = aⁱ
    V cy = y; // cy ≤ x at all times

    for (;;)
    {
        if (i == 0)
        {
            return { x - cy, { factor<V>{ a, ci }, factor<V>{ b, cj } } };
        }

            // take factor a
        y /= a;
        --i;

            // give factors b as long as y ≤ x
            // (note that y ∙ b overflowing implies y ∙ b > x)
        while (can_multiply(y, b) && y * b <= x)
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


    // Given x,a,b ∊ ℕ, x > 0, a,b > 1, a ≠ b, returns (r,{ {a,i}, {b,j} }) such that x = aⁱ ∙ bʲ - r with r ≥ 0 minimal.
template <typename EH, typename X, typename A, typename B>
    constexpr result_t<EH, factorization<common_integral_value_type<X, A, B>, 2>> factorize_ceili(X x, A a, B b)
{
    using V = common_integral_value_type<X, A, B>;

    Expects(x > 0 && a > 1 && b > 1 && a != b);

        // algorithm discussed in http://stackoverflow.com/a/39050139 and slightly altered to avoid unnecessary overflows

    auto facAResult = factorize_ceili<EH>(x, a);
    if (EH::is_error(facAResult)) return EH::passthrough_error(facAResult);
    auto facA = EH::get_value(facAResult);
        
    auto y0Result = add<EH>(x, facA.remainder); // = aⁱ
    if (EH::is_error(y0Result)) return EH::passthrough_error(y0Result);
    auto y0 = EH::get_value(y0Result);
        
    V i = facA.factors[0].exponent,
      j = 0;
    V ci = i,
      cj = j;
    V y = y0;
    V cy = y; // cy ≥ x at all times

    for (;;)
    {
        if (i == 0)
        {
            return EH::make_result(factorization<V, 2>{
                cy - x, { factor<V>{ a, ci }, factor<V>{ b, cj } }
            });
        }

            // take factor a
        y /= a;
        --i;

            // give factors b until y ≥ x
        while (y < x)
        {
            auto yResult = multiply<EH>(y, b);
            if (EH::is_error(yResult)) return EH::passthrough_error(yResult);
            y = EH::get_value(yResult);
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


} // namespace detail

} // namespace makeshift


#if defined(_MSC_VER) && !defined(__clang__)
 #pragma warning( pop )
#endif // defined(_MSC_VER) && !defined(__clang__)


#endif // INCLUDED_MAKESHIFT_DETAIL_ARITHMETIC_HPP_
