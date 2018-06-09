
#include <limits>

#include <stdint.h>

#include <catch.hpp>

#include <makeshift/arithmetic.hpp>

namespace mk = makeshift;
using makeshift::checked;
using makeshift::checked_cast;


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
        CHECK_THROWS_AS(lu8 = unchecked(-checked(u8)), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li8 = unchecked(-checked(i8nm)), mk::arithmetic_overflow_error);

            // addition
        CHECK_THROWS_AS(lu8 = unchecked(checked(u8) + u8), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li8 = unchecked(checked(i8p) + i8p), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li8 = unchecked(checked(i8n) + i8n), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(lu64 = unchecked(checked(u64) + u64), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li64 = unchecked(checked(i64p) + i64p), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li64 = unchecked(checked(i64n) + i64n), mk::arithmetic_overflow_error);

            // subtraction
        CHECK_THROWS_AS(lu8 = unchecked(checked(zu8) - u8), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li8 = unchecked(checked(i8p) - i8n), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li8 = unchecked(checked(i8n) - i8p), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(lu64 = unchecked(checked(zu64) - u64), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li64 = unchecked(checked(i64p) - i64n), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li64 = unchecked(checked(i64n) - i64p), mk::arithmetic_overflow_error);

            // multiplication
        CHECK_THROWS_AS(lu8 = unchecked(checked(u8) * u8), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li8 = unchecked(checked(i8p) * i8p), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li8 = unchecked(checked(i8p) * i8n), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li8 = unchecked(checked(i8n) * i8n), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(lu64 = unchecked(checked(u64) * u64), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li64 = unchecked(checked(i64p) * i64p), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li64 = unchecked(checked(i64p) * i64n), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(li64 = unchecked(checked(i64n) * i64n), mk::arithmetic_overflow_error);

            // division
        CHECK_THROWS_AS(lu8 = unchecked(checked(u8) / zu8), mk::arithmetic_div_by_zero_error);
        CHECK_THROWS_AS(li8 = unchecked(checked(i8p) / zi8), mk::arithmetic_div_by_zero_error);
        CHECK_THROWS_AS(li8 = unchecked(checked(i8nm) / -1), mk::arithmetic_overflow_error);

            // modulo
        CHECK_THROWS_AS(lu8 = unchecked(checked(u8) % zu8), mk::arithmetic_div_by_zero_error);
        CHECK_THROWS_AS(li8 = unchecked(checked(i8p) % zi8), mk::arithmetic_div_by_zero_error);
        CHECK_THROWS_AS(li8 = unchecked(checked(i8nm) % -1), mk::arithmetic_overflow_error);

            // left shift
        CHECK_THROWS_AS(lu8 = unchecked(checked(u8) << 10), mk::arithmetic_domain_error);
        CHECK_THROWS_AS(li8 = unchecked(checked(zi8) << 10), mk::arithmetic_domain_error);
        CHECK_THROWS_AS(li8 = unchecked(checked(zi8) << -1), mk::arithmetic_domain_error);
        CHECK_THROWS_AS(li8 = unchecked(checked(i8n) << 0), mk::arithmetic_domain_error);
        CHECK_THROWS_AS(li8 = unchecked(checked(i8p) << 2), mk::arithmetic_overflow_error);

            // right shift
        CHECK_THROWS_AS(lu8 = unchecked(checked(u8) >> 10), mk::arithmetic_domain_error);
        CHECK_THROWS_AS(li8 = unchecked(checked(zi8) >> 10), mk::arithmetic_domain_error);
        CHECK_THROWS_AS(li8 = unchecked(checked(zi8) >> -1), mk::arithmetic_domain_error);
        CHECK_THROWS_AS(li8 = unchecked(checked(i8n) >> 0), mk::arithmetic_domain_error);
    }
}

TEST_CASE("checked_cast", "testing checked_cast<>()")
{
    SECTION("pass")
    {
        volatile uint8_t lu8;
        volatile uint16_t lu16;
        volatile int8_t li8;
        volatile int16_t li16;

            // uint <-> uint
        CHECK_NOTHROW((lu16 = checked_cast<uint16_t,  uint8_t>( u8))); REQUIRE(lu16 == u8);
        CHECK_NOTHROW(( lu8 = checked_cast< uint8_t,  uint8_t>( u8))); REQUIRE( lu8 == u8);
        CHECK_NOTHROW(( lu8 = checked_cast< uint8_t, uint16_t>( u8))); REQUIRE( lu8 == u8);

            // int <-> int
        CHECK_NOTHROW((li16 = checked_cast< int16_t,   int8_t>(i8n))); REQUIRE(li16 == i8n);
        CHECK_NOTHROW((li16 = checked_cast< int16_t,   int8_t>(i8p))); REQUIRE(li16 == i8p);
        CHECK_NOTHROW(( li8 = checked_cast<  int8_t,   int8_t>(i8n))); REQUIRE( li8 == i8n);
        CHECK_NOTHROW(( li8 = checked_cast<  int8_t,   int8_t>(i8p))); REQUIRE( li8 == i8p);
        CHECK_NOTHROW(( li8 = checked_cast<  int8_t,  int16_t>(i8n))); REQUIRE( li8 == i8n);
        CHECK_NOTHROW(( li8 = checked_cast<  int8_t,  int16_t>(i8p))); REQUIRE( li8 == i8p);

            // uint <-> int
        CHECK_NOTHROW((lu16 = checked_cast<uint16_t,   int8_t>(i8p))); REQUIRE(lu16 == i8p);
        CHECK_NOTHROW((li16 = checked_cast< int16_t,  uint8_t>(i8p))); REQUIRE(li16 == i8p);
        CHECK_NOTHROW(( lu8 = checked_cast< uint8_t,   int8_t>(i8p))); REQUIRE( lu8 == i8p);
        CHECK_NOTHROW(( li8 = checked_cast<  int8_t,  uint8_t>(i8p))); REQUIRE( li8 == i8p);
        CHECK_NOTHROW(( lu8 = checked_cast< uint8_t,  int16_t>(i8p))); REQUIRE( lu8 == i8p);
        CHECK_NOTHROW(( li8 = checked_cast<  int8_t, uint16_t>(i8p))); REQUIRE( li8 == i8p);
    }

    SECTION("fail")
    {
        volatile uint8_t lu8;
        volatile uint16_t lu16;
        volatile int8_t li8;

            // uint <-> uint
        CHECK_THROWS_AS(( lu8 = checked_cast< uint8_t, uint16_t>(u16)), mk::arithmetic_overflow_error);

            // int <-> int
        CHECK_THROWS_AS(( li8 = checked_cast<  int8_t,  int16_t>(i16n)), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(( li8 = checked_cast<  int8_t,  int16_t>(i16p)), mk::arithmetic_overflow_error);

            // uint <-> int
        CHECK_THROWS_AS((lu16 = checked_cast<uint16_t,   int8_t>( i8n)), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(( lu8 = checked_cast< uint8_t,   int8_t>( i8n)), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(( li8 = checked_cast<  int8_t,  uint8_t>( u8 )), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(( lu8 = checked_cast< uint8_t,  int16_t>( i8n)), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(( lu8 = checked_cast< uint8_t,  int16_t>(i16n)), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(( lu8 = checked_cast< uint8_t,  int16_t>(i16p)), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(( li8 = checked_cast<  int8_t, uint16_t>( u8 )), mk::arithmetic_overflow_error);
        CHECK_THROWS_AS(( li8 = checked_cast<  int8_t, uint16_t>(u16 )), mk::arithmetic_overflow_error);
    }
}
