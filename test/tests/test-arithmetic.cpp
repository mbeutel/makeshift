
#include <limits>

#include <stdint.h>

#include <catch.hpp>

#include <makeshift/arithmetic.hpp>

#include <gsl/gsl_util> // for narrow_cast<>()


namespace mk = makeshift;


const volatile uint8_t u8 = std::numeric_limits<uint8_t>::max() - 1;
const volatile uint16_t u16 = std::numeric_limits<uint16_t>::max() - 1;
const volatile uint64_t u64 = std::numeric_limits<uint64_t>::max() - 1;
const volatile int8_t i8n = std::numeric_limits<int8_t>::min() + 1;
const volatile int8_t i8nm = std::numeric_limits<int8_t>::min();
const volatile int16_t i16n = std::numeric_limits<int16_t>::min() + 1;
const volatile int64_t i64n = std::numeric_limits<int64_t>::min() + 1;
const volatile int8_t i8p = std::numeric_limits<int8_t>::max() - 1;
const volatile int16_t i16p = std::numeric_limits<int16_t>::max() - 1;
const volatile int64_t i64p = std::numeric_limits<int64_t>::max() - 1;
const volatile uint8_t zu8 = 0;
const volatile uint64_t zu64 = 0;
const volatile int8_t zi8 = 0;
const volatile int64_t zi64 = 0;

TEST_CASE("checked", "testing checked arithmetic")
{
    volatile uint8_t lu8;
    volatile uint64_t lu64;
    volatile int8_t li8;
    volatile int64_t li64;

    SECTION("fail")
    {
            // negation
        CHECK_THROWS_AS(lu8 = mk::checked_negate_or_throw(u8), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li8 = mk::checked_negate_or_throw(i8nm), mk::arithmetic_overflow_error);

            // addition
        CHECK_THROWS_AS(lu8 = mk::checked_add_or_throw(u8, u8), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li8 = mk::checked_add_or_throw(i8p, i8p), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li8 = mk::checked_add_or_throw(i8n, i8n), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(lu64 = mk::checked_add_or_throw(u64, u64), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li64 = mk::checked_add_or_throw(i64p, i64p), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li64 = mk::checked_add_or_throw(i64n, i64n), mk::arithmetic_overflow_error);

            // subtraction
        CHECK_THROWS_AS(lu8 = mk::checked_subtract_or_throw(zu8, u8), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li8 = mk::checked_subtract_or_throw(i8p, i8n), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li8 = mk::checked_subtract_or_throw(i8n, i8p), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(lu64 = mk::checked_subtract_or_throw(zu64, u64), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li64 = mk::checked_subtract_or_throw(i64p, i64n), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li64 = mk::checked_subtract_or_throw(i64n, i64p), mk::arithmetic_overflow_error);

            // multiplication
        CHECK_THROWS_AS(lu8 = mk::checked_multiply_or_throw(u8, u8), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li8 = mk::checked_multiply_or_throw(i8p, i8p), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li8 = mk::checked_multiply_or_throw(i8p, i8n), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li8 = mk::checked_multiply_or_throw(i8n, i8n), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(lu64 = mk::checked_multiply_or_throw(u64, u64), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li64 = mk::checked_multiply_or_throw(i64p, i64p), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li64 = mk::checked_multiply_or_throw(i64p, i64n), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li64 = mk::checked_multiply_or_throw(i64n, i64n), mk::arithmetic_overflow_error);

            // division
        CHECK_THROWS_AS(lu8 = mk::checked_divide_or_throw(u8, zu8), mk::arithmetic_div_by_zero_error);
        CHECK_THROWS_AS(li8 = mk::checked_divide_or_throw(i8p, zi8), mk::arithmetic_div_by_zero_error);
        CHECK_THROWS_AS(li8 = mk::checked_divide_or_throw(i8nm, int8_t(-1)), mk::arithmetic_overflow_error);

            // modulo
        CHECK_THROWS_AS(lu8 = mk::checked_modulo_or_throw(u8, zu8), mk::arithmetic_div_by_zero_error);
        CHECK_THROWS_AS(li8 = mk::checked_modulo_or_throw(i8p, zi8), mk::arithmetic_div_by_zero_error);
        CHECK_THROWS_AS(li8 = mk::checked_modulo_or_throw(i8nm, int8_t(-1)), mk::arithmetic_overflow_error);

            // left shift
        CHECK_THROWS_AS(lu8 = mk::checked_shift_left_or_throw(u8, uint8_t(10)), mk::arithmetic_domain_error);
        CHECK_THROWS_AS(li8 = mk::checked_shift_left_or_throw(zi8, int8_t(10)), mk::arithmetic_domain_error);
        CHECK_THROWS_AS(li8 = mk::checked_shift_left_or_throw(zi8, int8_t(-1)), mk::arithmetic_domain_error);
        CHECK_THROWS_AS(li8 = mk::checked_shift_left_or_throw(i8n, int8_t(0)), mk::arithmetic_domain_error);
        CHECK_THROWS_AS(li8 = mk::checked_shift_left_or_throw(i8p, int8_t(2)), mk::arithmetic_overflow_error);

            // right shift
        CHECK_THROWS_AS(lu8 = mk::checked_shift_right_or_throw(u8, uint8_t(10)), mk::arithmetic_domain_error);
        CHECK_THROWS_AS(li8 = mk::checked_shift_right_or_throw(zi8, int8_t(10)), mk::arithmetic_domain_error);
        CHECK_THROWS_AS(li8 = mk::checked_shift_right_or_throw(zi8, int8_t(-1)), mk::arithmetic_domain_error);
        CHECK_THROWS_AS(li8 = mk::checked_shift_right_or_throw(i8n, int8_t(0)), mk::arithmetic_domain_error);
    }
}

TEST_CASE("gsl::narrow", "testing gsl::narrow<>()")
{
    SECTION("pass")
    {
        volatile uint8_t lu8;
        volatile uint16_t lu16;
        volatile int8_t li8;
        volatile int16_t li16;

            // uint <-> uint
        CHECK_NOTHROW((lu16 = gsl::narrow<uint16_t>( uint8_t( u8)))); REQUIRE(lu16 == u8);
        CHECK_NOTHROW(( lu8 = gsl::narrow< uint8_t>( uint8_t( u8)))); REQUIRE( lu8 == u8);
        CHECK_NOTHROW(( lu8 = gsl::narrow< uint8_t>(uint16_t( u8)))); REQUIRE( lu8 == u8);

            // int <-> int
        CHECK_NOTHROW((li16 = gsl::narrow< int16_t>(  int8_t(i8n)))); REQUIRE(li16 == i8n);
        CHECK_NOTHROW((li16 = gsl::narrow< int16_t>(  int8_t(i8p)))); REQUIRE(li16 == i8p);
        CHECK_NOTHROW(( li8 = gsl::narrow<  int8_t>(  int8_t(i8n)))); REQUIRE( li8 == i8n);
        CHECK_NOTHROW(( li8 = gsl::narrow<  int8_t>(  int8_t(i8p)))); REQUIRE( li8 == i8p);
        CHECK_NOTHROW(( li8 = gsl::narrow<  int8_t>( int16_t(i8n)))); REQUIRE( li8 == i8n);
        CHECK_NOTHROW(( li8 = gsl::narrow<  int8_t>( int16_t(i8p)))); REQUIRE( li8 == i8p);

            // uint <-> int
        CHECK_NOTHROW((lu16 = gsl::narrow<uint16_t>(  int8_t(i8p)))); REQUIRE(lu16 == i8p);
        CHECK_NOTHROW((li16 = gsl::narrow< int16_t>( uint8_t(i8p)))); REQUIRE(li16 == i8p);
        CHECK_NOTHROW(( lu8 = gsl::narrow< uint8_t>(  int8_t(i8p)))); REQUIRE( lu8 == i8p);
        CHECK_NOTHROW(( li8 = gsl::narrow<  int8_t>( uint8_t(i8p)))); REQUIRE( li8 == i8p);
        CHECK_NOTHROW(( lu8 = gsl::narrow< uint8_t>( int16_t(i8p)))); REQUIRE( lu8 == i8p);
        CHECK_NOTHROW(( li8 = gsl::narrow<  int8_t>(uint16_t(i8p)))); REQUIRE( li8 == i8p);
    }

    SECTION("fail")
    {
        volatile uint8_t lu8;
        volatile uint16_t lu16;
        volatile int8_t li8;

            // uint <-> uint
        CHECK_THROWS(( lu8 = gsl::narrow< uint8_t>(uint16_t(u16))));

            // int <-> int
        CHECK_THROWS(( li8 = gsl::narrow<  int8_t>( int16_t(i16n))));
        CHECK_THROWS(( li8 = gsl::narrow<  int8_t>( int16_t(i16p))));

            // uint <-> int
        CHECK_THROWS((lu16 = gsl::narrow<uint16_t>(  int8_t( i8n))));
        CHECK_THROWS(( lu8 = gsl::narrow< uint8_t>(  int8_t( i8n))));
        CHECK_THROWS(( li8 = gsl::narrow<  int8_t>( uint8_t( u8 ))));
        CHECK_THROWS(( lu8 = gsl::narrow< uint8_t>( int16_t( i8n))));
        CHECK_THROWS(( lu8 = gsl::narrow< uint8_t>( int16_t(i16n))));
        CHECK_THROWS(( lu8 = gsl::narrow< uint8_t>( int16_t(i16p))));
        CHECK_THROWS(( li8 = gsl::narrow<  int8_t>(uint16_t( u8 ))));
        CHECK_THROWS(( li8 = gsl::narrow<  int8_t>(uint16_t(u16 ))));
    }
}
