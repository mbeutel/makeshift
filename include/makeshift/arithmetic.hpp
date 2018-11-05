
#ifndef INCLUDED_MAKESHIFT_ARITHMETIC_HPP_
#define INCLUDED_MAKESHIFT_ARITHMETIC_HPP_


#include <limits>
#include <type_traits> // for make_[un]signed<>, is_[un]signed<>, is_integral<>, is_enum<>, is_same<>, decay<>
#include <stdexcept>   // for runtime_error
#include <utility>     
#include <cstdint>     // for [u]int(8|16|32)_t

#include <gsl/gsl_assert> // for Expects()

#include <makeshift/version.hpp> // for MAKESHIFT_FORCEINLINE


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


struct check_error_handler
{
    static constexpr bool isNoexcept = false;
    static constexpr MAKESHIFT_FORCEINLINE void checkDivByZero(bool cond) { if (!cond) throw arithmetic_div_by_zero_error("division by zero"); }
    static constexpr MAKESHIFT_FORCEINLINE void checkShiftOp(bool cond) { if (!cond) throw arithmetic_domain_error("invalid shift operand"); }
    static constexpr MAKESHIFT_FORCEINLINE void checkOverflow(bool cond) { if (!cond) throw arithmetic_overflow_error("integer overflow"); }
    static constexpr MAKESHIFT_FORCEINLINE void checkUnderflow(bool cond) { if (!cond) throw arithmetic_overflow_error("integer underflow"); }
    static constexpr MAKESHIFT_FORCEINLINE void checkRange(bool cond) { if (!cond) throw arithmetic_overflow_error("integer out of range"); }
};
struct assume_error_handler
{
#ifdef GSL_THROW_ON_CONTRACT_VIOLATION
    static constexpr bool isNoexcept = false;
#else // GSL_THROW_ON_CONTRACT_VIOLATION
    static constexpr bool isNoexcept = true;
#endif // GSL_THROW_ON_CONTRACT_VIOLATION
    static constexpr MAKESHIFT_FORCEINLINE void checkDivByZero(bool cond) noexcept(isNoexcept) { Expects(cond); }
    static constexpr MAKESHIFT_FORCEINLINE void checkShiftOp(bool cond) noexcept(isNoexcept) { Expects(cond); }
    static constexpr MAKESHIFT_FORCEINLINE void checkOverflow(bool cond) noexcept(isNoexcept) { Expects(cond); }
    static constexpr MAKESHIFT_FORCEINLINE void checkUnderflow(bool cond) noexcept(isNoexcept) { Expects(cond); }
    static constexpr MAKESHIFT_FORCEINLINE void checkRange(bool cond) noexcept(isNoexcept) { Expects(cond); }
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

template <typename EH, typename V, int_signedness>
    struct checked_3_;
template <typename EH, typename V>
    struct checked_3_<EH, V, is_unsigned>
{
    static constexpr V divide(V lhs, V rhs) noexcept(EH::isNoexcept)
    {
        EH::checkDivByZero(rhs != 0);
        return lhs / rhs;
    }
    static constexpr V modulo(V lhs, V rhs) noexcept(EH::isNoexcept)
    {
        EH::checkDivByZero(rhs != 0);
        return lhs % rhs;
    }
    static constexpr V negate(V arg) noexcept(EH::isNoexcept)
    {
        EH::checkUnderflow(arg == 0);
        return 0;
    }
    static constexpr V subtract(V lhs, V rhs) noexcept(EH::isNoexcept)
    {
        EH::checkUnderflow(lhs >= rhs);
        return lhs - rhs;
    }
    static constexpr V shl(V lhs, V rhs) noexcept(EH::isNoexcept)
    {
            // Note that we don't fail if an unsigned left shift loses significant bits. This is usually expected behaviour,
            // but it is (deliberately) inconsistent with the checked unsigned multiplication.
        EH::checkShiftOp(rhs < sizeof(V)*8);
        return lhs << rhs;
    }
    static constexpr V shr(V lhs, V rhs) noexcept(EH::isNoexcept)
    {
        EH::checkShiftOp(rhs < sizeof(V)*8);
        return lhs >> rhs;
    }
};
template <typename EH, typename V>
    struct checked_3_<EH, V, is_signed>
{
    using U = typename std::make_unsigned<V>::type;

    static constexpr V divide(V lhs, V rhs) noexcept(EH::isNoexcept)
    {
        EH::checkOverflow(lhs != std::numeric_limits<V>::min() || rhs != -1);
        EH::checkDivByZero(rhs != 0);
        return lhs / rhs;
    }
    static constexpr V modulo(V lhs, V rhs) noexcept(EH::isNoexcept)
    {
        EH::checkOverflow(lhs != std::numeric_limits<V>::min() || rhs != -1);
        EH::checkDivByZero(rhs != 0);
        return lhs % rhs;
    }
    static constexpr V negate(V arg) noexcept(EH::isNoexcept)
    {
            // this assumes a two's complement representation (it will yield a false negative for one's complement integers)
        EH::checkUnderflow(arg != std::numeric_limits<V>::min());
        return -arg;
    }
    static constexpr V subtract(V lhs, V rhs) noexcept(EH::isNoexcept)
    {
        EH::checkUnderflow(
            (rhs <= 0 || lhs >= std::numeric_limits<V>::min() + rhs)
         && (rhs >= 0 || lhs <= std::numeric_limits<V>::max() + rhs));
        return lhs - rhs;
    }
    static constexpr V shl(V lhs, V rhs) noexcept(EH::isNoexcept)
    {
            // note that we throw when shifting negative integers
        EH::checkShiftOp(lhs >= 0 && rhs >= 0 && rhs < V(sizeof(V)*8));
        EH::checkOverflow(lhs <= (std::numeric_limits<V>::max() >> rhs));
        return lhs << rhs;
    }
    static constexpr V shr(V lhs, V rhs) noexcept(EH::isNoexcept)
    {
            // note that we throw when shifting negative integers
        EH::checkShiftOp(lhs >= 0 && rhs >= 0 && rhs < V(sizeof(V)*8));
        return lhs >> rhs;
    }
};
template <typename EH, typename V, int_width, int_signedness>
    struct checked_2_;
template <typename EH, typename V>
    struct checked_2_<EH, V, has_wider_type, is_unsigned> : checked_3_<EH, V, is_unsigned>
{
    using W = typename wider_type<V>::type;

    static constexpr V add(V lhs, V rhs) noexcept(EH::isNoexcept)
    {
        W result = W(lhs) + W(rhs);
        EH::checkOverflow(result <= std::numeric_limits<V>::max());
        return static_cast<V>(result);
    }
    static constexpr V multiply(V lhs, V rhs) noexcept(EH::isNoexcept)
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

    static constexpr V add(V lhs, V rhs) noexcept(EH::isNoexcept)
    {
        W result = W(lhs) + W(rhs);
        EH::checkOverflow(result >= std::numeric_limits<V>::min() && result <= std::numeric_limits<V>::max());
        return static_cast<V>(result);
    }
    static constexpr V multiply(V lhs, V rhs) noexcept(EH::isNoexcept)
    {
        W result = W(lhs) * W(rhs);
        EH::checkOverflow(result >= std::numeric_limits<V>::min() && result <= std::numeric_limits<V>::max());
        return static_cast<V>(result);
    }
};
template <typename EH, typename V>
    struct checked_2_<EH, V, has_no_wider_type, is_unsigned> : checked_3_<EH, V, is_unsigned>
{
    static constexpr V add(V lhs, V rhs) noexcept(EH::isNoexcept)
    {
        V result = lhs + rhs;
        EH::checkOverflow(result >= lhs && result >= rhs);
        return result;
    }
    static constexpr V multiply(V lhs, V rhs) noexcept(EH::isNoexcept)
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

    static constexpr V add(V lhs, V rhs) noexcept(EH::isNoexcept)
    {
        V result = V(U(lhs) + U(rhs));
        EH::checkOverflow(
            (lhs >= 0 || rhs >= 0 || result < 0) // at least one operand is non-negative, or the result is negative
         && (lhs <  0 || rhs <  0 || result >= 0)); // at least one operand is negative, or the result is non-negative
        return result;
    }
    static constexpr V multiply(V lhs, V rhs) noexcept(EH::isNoexcept)
    {
        EH::checkOverflow(
            (lhs <= 0 || ((rhs <= 0             || lhs <= std::numeric_limits<V>::max() / rhs)
                       && (rhs >  0             || rhs >= std::numeric_limits<V>::min() / lhs)))
         && (lhs >  0 || ((rhs <= 0             || lhs >= std::numeric_limits<V>::min() / rhs)
                       && (rhs >  0 || lhs == 0 || rhs >= std::numeric_limits<V>::max() / lhs))));
        return lhs * rhs;
    }
};
template <typename EH, typename T>
    struct checked_1_ : checked_2_<EH, typename std::decay<T>::type,
        (sizeof(T) < sizeof(std::uint64_t)) ? has_wider_type : has_no_wider_type,
        std::is_signed<typename std::decay<T>::type>::value ? is_signed : is_unsigned>
{
    using V = typename std::decay<T>::type;

    static_assert(!std::is_same<V, bool>::value, "checked arithmetic does not support bool");
    static_assert(std::is_integral<V>::value, "checked arithmetic only works with integral types");
};
template <typename EH, typename T>
    struct checked_0_ : checked_1_<EH, T>
{
    using V = std::decay_t<T>;
    
    enum class type : V { };

    friend constexpr type operator +(type arg) noexcept { return arg; }
    friend constexpr type operator -(type arg) { return type(checked_1_<EH, T>::negate(V(arg))); }
    friend constexpr type operator  +(type lhs, type rhs) { return type(checked_1_<EH, T>::add(V(lhs), V(rhs))); }
    friend constexpr type operator  +(V lhs, type rhs) { return type(checked_1_<EH, T>::add(lhs, V(rhs))); }
    friend constexpr type operator  +(type lhs, V rhs) { return type(checked_1_<EH, T>::add(V(lhs), rhs)); }
    friend constexpr type operator  -(type lhs, type rhs) { return type(checked_1_<EH, T>::subtract(V(lhs), V(rhs))); }
    friend constexpr type operator  -(V lhs, type rhs) { return type(checked_1_<EH, T>::subtract(lhs, V(rhs))); }
    friend constexpr type operator  -(type lhs, V rhs) { return type(checked_1_<EH, T>::subtract(V(lhs), rhs)); }
    friend constexpr type operator  *(type lhs, type rhs) { return type(checked_1_<EH, T>::multiply(V(lhs), V(rhs))); }
    friend constexpr type operator  *(V lhs, type rhs) { return type(checked_1_<EH, T>::multiply(lhs, V(rhs))); }
    friend constexpr type operator  *(type lhs, V rhs) { return type(checked_1_<EH, T>::multiply(V(lhs), rhs)); }
    friend constexpr type operator  /(type lhs, type rhs) { return type(checked_1_<EH, T>::divide(V(lhs), V(rhs))); }
    friend constexpr type operator  /(V lhs, type rhs) { return type(checked_1_<EH, T>::divide(lhs, V(rhs))); }
    friend constexpr type operator  /(type lhs, V rhs) { return type(checked_1_<EH, T>::divide(V(lhs), rhs)); }
    friend constexpr type operator  %(type lhs, type rhs) { return type(checked_1_<EH, T>::modulo(V(lhs), V(rhs))); }
    friend constexpr type operator  %(V lhs, type rhs) { return type(checked_1_<EH, T>::modulo(lhs, V(rhs))); }
    friend constexpr type operator  %(type lhs, V rhs) { return type(checked_1_<EH, T>::modulo(V(lhs), rhs)); }
    friend constexpr type operator <<(type lhs, type rhs) { return type(checked_1_<EH, T>::shl(V(lhs), V(rhs))); }
    friend constexpr type operator <<(V lhs, type rhs) { return type(checked_1_<EH, T>::shl(lhs), V(rhs)); }
    friend constexpr type operator <<(type lhs, V rhs) { return type(checked_1_<EH, T>::shl(V(lhs), rhs)); }
    friend constexpr type operator >>(type lhs, type rhs) { return type(checked_1_<EH, T>::shr(V(lhs), V(rhs))); }
    friend constexpr type operator >>(V lhs, type rhs) { return type(checked_1_<EH, T>::shr(lhs), V(rhs)); }
    friend constexpr type operator >>(type lhs, V rhs) { return type(checked_1_<EH, T>::shr(V(lhs), rhs)); }
    friend constexpr type& operator  +=(type& lhs, type rhs) { return (lhs = lhs + rhs); }
    friend constexpr type& operator  -=(type& lhs, type rhs) { return (lhs = lhs - rhs); }
    friend constexpr type& operator  *=(type& lhs, type rhs) { return (lhs = lhs * rhs); }
    friend constexpr type& operator  /=(type& lhs, type rhs) { return (lhs = lhs / rhs); }
    friend constexpr type& operator  %=(type& lhs, type rhs) { return (lhs = lhs % rhs); }
    friend constexpr type& operator <<=(type& lhs, type rhs) { return (lhs = lhs << rhs); }
    friend constexpr type& operator >>=(type& lhs, type rhs) { return (lhs = lhs >> rhs); }
    friend constexpr bool operator ==(V lhs, type rhs) noexcept { return lhs == V(rhs); }
    friend constexpr bool operator ==(type lhs, V rhs) noexcept { return V(lhs) == rhs; }
    friend constexpr bool operator !=(V lhs, type rhs) noexcept { return lhs != V(rhs); }
    friend constexpr bool operator !=(type lhs, V rhs) noexcept { return V(lhs) != rhs; }
    friend constexpr bool operator < (V lhs, type rhs) noexcept { return lhs <  V(rhs); }
    friend constexpr bool operator < (type lhs, V rhs) noexcept { return V(lhs) <  rhs; }
    friend constexpr bool operator <=(V lhs, type rhs) noexcept { return lhs <= V(rhs); }
    friend constexpr bool operator <=(type lhs, V rhs) noexcept { return V(lhs) <= rhs; }
    friend constexpr bool operator >=(V lhs, type rhs) noexcept { return lhs >= V(rhs); }
    friend constexpr bool operator >=(type lhs, V rhs) noexcept { return V(lhs) >= rhs; }
    friend constexpr bool operator > (V lhs, type rhs) noexcept { return lhs >  V(rhs); }
    friend constexpr bool operator > (type lhs, V rhs) noexcept { return V(lhs) >  rhs; }

        //ᅟ
        // Converts the given value to the unchecked version of the argument type.
        //
    friend constexpr V unchecked(type arg) noexcept { return V(arg); }
};

template <typename T, bool IsEnum> struct is_checked_;
template <typename T> struct is_checked_<T, true> : std::is_same<T, typename checked_0_<check_error_handler, std::underlying_type_t<T>>::type> { };
template <typename T> struct is_checked_<T, false> : std::false_type { };

template <typename T, bool IsChecked> struct checked_;
template <typename T> struct checked_<T, true> { using type = T; };
template <typename T> struct checked_<T, false> : checked_0_<check_error_handler, T> { };

template <typename T, bool IsChecked> struct unchecked_;
template <typename T> struct unchecked_<T, true> : std::underlying_type<T> { };
template <typename T> struct unchecked_<T, false> { using type = T; };


} // namespace detail


inline namespace arithmetic
{


    //ᅟ
    // Determines whether `T` is a checked type, i.e. whether arithmetic operations on `T` are checked at runtime.
    //
template <typename T> struct is_checked : makeshift::detail::is_checked_<T, std::is_enum<T>::value> { };

    //ᅟ
    // Determines whether `T` is a checked type, i.e. whether arithmetic operations on `T` are checked at runtime.
    //
template <typename T> constexpr bool is_checked_v = is_checked<T>::value;


    //ᅟ
    // Obtains the checked version of the scalar integer type `T`, or `T` if it already is a checked type.
    //
template <typename T> struct make_checked : makeshift::detail::checked_<T, is_checked_v<T>> { };

    //ᅟ
    // Obtains the checked version of the scalar integer type `T`, or `T` if it already is a checked type.
    //
template <typename T> using make_checked_t = typename make_checked<T>::type;


    //ᅟ
    // Obtains the unchecked version of the checked scalar integer type `T`, or `T` if it already is an unchecked type.
    //
template <typename T> struct make_unchecked : makeshift::detail::unchecked_<T, is_checked_v<T>> { };

    //ᅟ
    // Obtains the unchecked version of the checked scalar integer type `T`, or `T` if it already is an unchecked type.
    //
template <typename T> using make_unchecked_t = typename make_unchecked<T>::type;


    //ᅟ
    // Converts the given value to the checked version of the scalar integer type `T`, or to `T` if it already is a checked type.
    // Arithmetic operations are checked even if one of the arguments is unchecked; the result is always a checked type.
    //ᅟ
    //ᅟ    int lhs = ..., rhs = ...;
    //ᅟ    make_checked_t<int> checked_sum = checked(lhs) + rhs;
    //ᅟ    int sum = unchecked(checked_sum);
    //
template <typename T>
    constexpr make_checked_t<T> checked(T val) noexcept
{
    return make_checked_t<T>(val);
}


    //ᅟ
    // Converts the given value to the checked version of the scalar integer type `T`, or to `T` if it already is an unchecked type.
    //
template <typename T>
    constexpr make_unchecked_t<T> unchecked(T val) noexcept
{
    return make_unchecked_t<T>(val);
}


} // inline namespace arithmetic


namespace detail
{

struct trivial_cast_
{
    template <typename DstT, typename SrcT>
        static DstT invoke(SrcT src) noexcept
    {
        return static_cast<DstT>(src);
    }
};
template <typename EH>
    struct unsigned_to_any_narrowing_cast_
{
    template <typename DstT, typename SrcT>
        static DstT invoke(SrcT src) noexcept(EH::isNoexcept)
    {
        if (src > static_cast<SrcT>(std::numeric_limits<DstT>::max())) throw arithmetic_overflow_error("integer out of range");
        return static_cast<DstT>(src);
    }
};

enum int_conversion { narrowing, non_narrowing };

template <typename EH, int_signedness DstSignedness, int_signedness SrcSignedness, int_conversion> struct checked_cast_;

    // non-narrowing casts
template <typename EH, int_signedness Signedness> struct checked_cast_<EH, Signedness, Signedness, non_narrowing> : trivial_cast_ { };
template <typename EH> struct checked_cast_<EH, is_signed,   is_unsigned, non_narrowing> : trivial_cast_ { }; // unsigned-to-signed cast
template <typename EH> struct checked_cast_<EH, is_unsigned, is_signed,   non_narrowing> // signed-to-unsigned cast
{
    template <typename DstT, typename SrcT>
        static DstT invoke(SrcT src) noexcept(EH::isNoexcept)
    {
        EH::checkRange(src >= 0);
        return static_cast<DstT>(src);
    }
};

    // narrowing casts
template <typename EH> struct checked_cast_<EH, is_signed,   is_signed,   narrowing> // signed-to-signed narrowing cast
{
    template <typename DstT, typename SrcT>
        static DstT invoke(SrcT src) noexcept(EH::isNoexcept)
    {
        EH::checkRange(src >= static_cast<SrcT>(std::numeric_limits<DstT>::min()) && src <= static_cast<SrcT>(std::numeric_limits<DstT>::max()));
        return static_cast<DstT>(src);
    }
};
template <typename EH> struct checked_cast_<EH, is_unsigned, is_unsigned, narrowing> : unsigned_to_any_narrowing_cast_<EH> { }; // unsigned-to-unsigned narrowing cast
template <typename EH> struct checked_cast_<EH, is_unsigned, is_signed,   narrowing> // signed-to-unsigned narrowing cast
{
    template <typename DstT, typename SrcT>
        static DstT invoke(SrcT src) noexcept(EH::isNoexcept)
    {
        EH::checkRange(src >= 0 && src <= static_cast<SrcT>(std::numeric_limits<DstT>::max()));
        return static_cast<DstT>(src);
    }
};
template <typename EH> struct checked_cast_<EH, is_signed,   is_unsigned, narrowing> : unsigned_to_any_narrowing_cast_<EH> { }; // unsigned-to-signed narrowing cast
        

template <typename T, bool IsChecked> struct make_checked_if;
template <typename T> struct make_checked_if<T, true> : make_checked<T> { };
template <typename T> struct make_checked_if<T, false> { using type = T; };
template <typename T, bool IsChecked> using make_checked_if_t = typename make_checked_if<T, IsChecked>::type;

} // namespace detail


inline namespace arithmetic
{


    //ᅟ
    // Performs a cast between different integer types and checks for overflow at runtime.
    //ᅟ
    //ᅟ    int size = checked_cast<int>(vec.size());
    //
template <typename DstT, typename SrcT>
    typename makeshift::detail::make_checked_if<DstT, is_checked_v<std::decay_t<SrcT>>>::type
    checked_cast(SrcT src)
{
    using RSrc = std::decay_t<SrcT>;
    using RDst = makeshift::detail::make_checked_if_t<DstT, is_checked_v<std::decay_t<SrcT>>>;
    
    using UTDst = make_unchecked_t<DstT>;
    using UTSrc = make_unchecked_t<RSrc>;
    
    static_assert(!std::is_same<DstT, bool>::value && !std::is_same<RSrc, bool>::value, "checked_cast<>() does not support bool");
    static_assert(std::is_integral<UTDst>::value && std::is_integral<UTSrc>::value, "checked_cast<>() only works with integral types");

    using UDst = std::make_unsigned_t<UTDst>;
    using USrc = std::make_unsigned_t<UTSrc>;
    constexpr bool isNarrowing = static_cast<UDst>(std::numeric_limits<UTDst>::max()) < static_cast<USrc>(std::numeric_limits<UTSrc>::max());
    return RDst(makeshift::detail::checked_cast_<makeshift::detail::check_error_handler,
            std::is_signed<UTDst>::value ? makeshift::detail::is_signed : makeshift::detail::is_unsigned,
            std::is_signed<UTSrc>::value ? makeshift::detail::is_signed : makeshift::detail::is_unsigned,
            isNarrowing ? makeshift::detail::narrowing : makeshift::detail::non_narrowing>
        ::template invoke<UTDst, UTSrc>(UTSrc(src)));
}


} // inline namespace arithmetic

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_ARITHMETIC_HPP_
