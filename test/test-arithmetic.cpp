
#include <limits>
#include <sstream>

#include <stdint.h>

#include <catch2/catch.hpp>

#include <makeshift/arithmetic.hpp>

#include <gsl/gsl_util> // for narrow<>(), fail_fast


namespace mk = makeshift;


using std::uint8_t;
using std::uint16_t;
using std::uint64_t;
using std::int8_t;
using std::int16_t;
using std::int64_t;
using std::size_t;


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

const volatile size_t smax = std::numeric_limits<size_t>::max();
const volatile size_t smaxlog = mk::log_floor(smax, size_t(2));
const volatile int imin = std::numeric_limits<int>::min(),
                   imax = std::numeric_limits<int>::max();
const volatile int imaxlog = mk::log_floor(imax, 2);
const volatile int iminlog = mk::log_floor(imin / -2, 2) + 1;
const volatile int64_t lmin = std::numeric_limits<int64_t>::min(),
                         lmax = std::numeric_limits<int64_t>::max();
const volatile int64_t lmaxlog = mk::log_floor(lmax, int64_t(2));
const volatile int64_t lminlog = mk::log_floor(lmin / -2, int64_t(2)) + 1;


TEST_CASE("factorize")
{
    SECTION("log")
    {
        CHECK(mk::log_floor(1, 2) == 0);
        CHECK(mk::log_floor(2, 2) == 1);
        CHECK(mk::log_floor(3, 2) == 1);
        CHECK(mk::log_floor(8, 2) == 3);

        CHECK(mk::log_ceil(1, 2) == 0);
        CHECK(mk::log_ceil(2, 2) == 1);
        CHECK(mk::log_ceil(3, 2) == 2);
        CHECK(mk::log_ceil(8, 2) == 3);
    }

    SECTION("factorize")
    {
        CHECK(mk::factorize_ceil(1, 2, 3) == mk::factorization<int, 2>{ 0, { mk::factor<int>{ 2, 0 }, mk::factor<int>{ 3, 0 } } });
        CHECK(mk::factorize_ceil(2, 2, 3) == mk::factorization<int, 2>{ 0, { mk::factor<int>{ 2, 1 }, mk::factor<int>{ 3, 0 } } });
        CHECK(mk::factorize_ceil(3, 2, 3) == mk::factorization<int, 2>{ 0, { mk::factor<int>{ 2, 0 }, mk::factor<int>{ 3, 1 } } });
        CHECK(mk::factorize_ceil(4, 2, 3) == mk::factorization<int, 2>{ 0, { mk::factor<int>{ 2, 2 }, mk::factor<int>{ 3, 0 } } });
        CHECK(mk::factorize_ceil(5, 2, 3) == mk::factorization<int, 2>{ 1, { mk::factor<int>{ 2, 1 }, mk::factor<int>{ 3, 1 } } });
        CHECK(mk::factorize_ceil(6, 2, 3) == mk::factorization<int, 2>{ 0, { mk::factor<int>{ 2, 1 }, mk::factor<int>{ 3, 1 } } });
        CHECK(mk::factorize_ceil(7, 2, 3) == mk::factorization<int, 2>{ 1, { mk::factor<int>{ 2, 3 }, mk::factor<int>{ 3, 0 } } });
        CHECK(mk::factorize_ceil(8, 2, 3) == mk::factorization<int, 2>{ 0, { mk::factor<int>{ 2, 3 }, mk::factor<int>{ 3, 0 } } });

        CHECK(mk::factorize_floor(1, 2, 3) == mk::factorization<int, 2>{ 0, { mk::factor<int>{ 2, 0 }, mk::factor<int>{ 3, 0 } } });
        CHECK(mk::factorize_floor(2, 2, 3) == mk::factorization<int, 2>{ 0, { mk::factor<int>{ 2, 1 }, mk::factor<int>{ 3, 0 } } });
        CHECK(mk::factorize_floor(3, 2, 3) == mk::factorization<int, 2>{ 0, { mk::factor<int>{ 2, 0 }, mk::factor<int>{ 3, 1 } } });
        CHECK(mk::factorize_floor(4, 2, 3) == mk::factorization<int, 2>{ 0, { mk::factor<int>{ 2, 2 }, mk::factor<int>{ 3, 0 } } });
        CHECK(mk::factorize_floor(5, 2, 3) == mk::factorization<int, 2>{ 1, { mk::factor<int>{ 2, 2 }, mk::factor<int>{ 3, 0 } } });
        CHECK(mk::factorize_floor(6, 2, 3) == mk::factorization<int, 2>{ 0, { mk::factor<int>{ 2, 1 }, mk::factor<int>{ 3, 1 } } });
        CHECK(mk::factorize_floor(7, 2, 3) == mk::factorization<int, 2>{ 1, { mk::factor<int>{ 2, 1 }, mk::factor<int>{ 3, 1 } } });
        CHECK(mk::factorize_floor(8, 2, 3) == mk::factorization<int, 2>{ 0, { mk::factor<int>{ 2, 3 }, mk::factor<int>{ 3, 0 } } });
    }
}

TEST_CASE("checked")
{
    SECTION("fail")
    {
        volatile uint8_t lu8;
        volatile uint64_t lu64;
        volatile int8_t li8;
        volatile int64_t li64;

            // negation
        CHECK_THROWS_AS(lu8 = mk::negate_or_throw(u8), mk::arithmetic_overflow);
        CHECK_THROWS_AS(li8 = mk::negate_or_throw(i8nm), mk::arithmetic_overflow);

            // addition
        CHECK_THROWS_AS(lu8 = mk::add_or_throw(u8, u8), mk::arithmetic_overflow);
        CHECK_THROWS_AS(li8 = mk::add_or_throw(i8p, i8p), mk::arithmetic_overflow);
        CHECK_THROWS_AS(li8 = mk::add_or_throw(i8n, i8n), mk::arithmetic_overflow);
        CHECK_THROWS_AS(lu64 = mk::add_or_throw(u64, u64), mk::arithmetic_overflow);
        CHECK_THROWS_AS(li64 = mk::add_or_throw(i64p, i64p), mk::arithmetic_overflow);
        CHECK_THROWS_AS(li64 = mk::add_or_throw(i64n, i64n), mk::arithmetic_overflow);

            // subtraction
        CHECK_THROWS_AS(lu8 = mk::subtract_or_throw(zu8, u8), mk::arithmetic_overflow);
        CHECK_THROWS_AS(li8 = mk::subtract_or_throw(i8p, i8n), mk::arithmetic_overflow);
        CHECK_THROWS_AS(li8 = mk::subtract_or_throw(i8n, i8p), mk::arithmetic_overflow);
        CHECK_THROWS_AS(lu64 = mk::subtract_or_throw(zu64, u64), mk::arithmetic_overflow);
        CHECK_THROWS_AS(li64 = mk::subtract_or_throw(i64p, i64n), mk::arithmetic_overflow);
        CHECK_THROWS_AS(li64 = mk::subtract_or_throw(i64n, i64p), mk::arithmetic_overflow);

            // multiplication
        CHECK_THROWS_AS(lu8 = mk::multiply_or_throw(u8, u8), mk::arithmetic_overflow);
        CHECK_THROWS_AS(li8 = mk::multiply_or_throw(i8p, i8p), mk::arithmetic_overflow);
        CHECK_THROWS_AS(li8 = mk::multiply_or_throw(i8p, i8n), mk::arithmetic_overflow);
        CHECK_THROWS_AS(li8 = mk::multiply_or_throw(i8n, i8n), mk::arithmetic_overflow);
        CHECK_THROWS_AS(lu64 = mk::multiply_or_throw(u64, u64), mk::arithmetic_overflow);
        CHECK_THROWS_AS(li64 = mk::multiply_or_throw(i64p, i64p), mk::arithmetic_overflow);
        CHECK_THROWS_AS(li64 = mk::multiply_or_throw(i64p, i64n), mk::arithmetic_overflow);
        CHECK_THROWS_AS(li64 = mk::multiply_or_throw(i64n, i64n), mk::arithmetic_overflow);

            // division
        CHECK_THROWS_AS(lu8 = mk::divide_or_throw(u8, zu8), gsl::fail_fast);
        CHECK_THROWS_AS(li8 = mk::divide_or_throw(i8p, zi8), gsl::fail_fast);
        CHECK_THROWS_AS(li8 = mk::divide_or_throw(i8nm, int8_t(-1)), mk::arithmetic_overflow);

            // modulo
        CHECK_THROWS_AS(lu8 = mk::modulo_or_throw(u8, zu8), gsl::fail_fast);
        CHECK_THROWS_AS(li8 = mk::modulo_or_throw(i8p, zi8), gsl::fail_fast);
        CHECK_THROWS_AS(li8 = mk::modulo_or_throw(i8nm, int8_t(-1)), mk::arithmetic_overflow);

            // left shift
        CHECK_THROWS_AS(lu8 = mk::shift_left_or_throw(u8, uint8_t(10)), gsl::fail_fast);
        CHECK_THROWS_AS(li8 = mk::shift_left_or_throw(zi8, int8_t(10)), gsl::fail_fast);
        CHECK_THROWS_AS(li8 = mk::shift_left_or_throw(zi8, int8_t(-1)), gsl::fail_fast);
        CHECK_THROWS_AS(li8 = mk::shift_left_or_throw(i8n, int8_t(0)), gsl::fail_fast);
        CHECK_THROWS_AS(li8 = mk::shift_left_or_throw(i8p, int8_t(2)), mk::arithmetic_overflow);

            // right shift
        CHECK_THROWS_AS(lu8 = mk::shift_right(u8, uint8_t(10)), gsl::fail_fast);
        CHECK_THROWS_AS(li8 = mk::shift_right(zi8, int8_t(10)), gsl::fail_fast);
        CHECK_THROWS_AS(li8 = mk::shift_right(zi8, int8_t(-1)), gsl::fail_fast);
        CHECK_THROWS_AS(li8 = mk::shift_right(i8n, int8_t(0)), gsl::fail_fast);
    }

    SECTION("multiply")
    {
        SECTION("good")
        {
            CHECK(mk::multiply_or_throw(size_t(0), size_t(1)) == size_t(0));
            CHECK(mk::multiply_or_throw(size_t(2), size_t(3)) == size_t(6));
            CHECK(mk::multiply_or_throw(size_t(3), size_t(4)) == size_t(12));

            CHECK(mk::multiply_or_throw(smax, size_t(1)) == smax);
            CHECK(mk::multiply_or_throw(smax, size_t(0)) == size_t(0));
            CHECK(mk::multiply_or_throw(size_t(1), smax) == smax);
            CHECK(mk::multiply_or_throw(size_t(0), smax) == size_t(0));
            CHECK(mk::multiply_or_throw(smax / 2, size_t(2)) == (smax / size_t(2)) * size_t(2));
            CHECK(mk::multiply_or_throw(size_t(2), smax / 2) == (smax / size_t(2)) * size_t(2));

            CHECK(mk::multiply_or_throw(smax / 2 - 1, size_t(2)) == (smax / 2 - 1) * 2); // make sure the implementation doesn't use floating-point numbers

            CHECK(mk::multiply_or_throw(0, 1) == 0);
            CHECK(mk::multiply_or_throw(2, 3) == 6);
            CHECK(mk::multiply_or_throw(3, 4) == 12);

            CHECK(mk::multiply_or_throw(imax, 1) == imax);
            CHECK(mk::multiply_or_throw(imax, 0) == 0);
            CHECK(mk::multiply_or_throw(1, imax) == imax);
            CHECK(mk::multiply_or_throw(0, imax) == 0);
            CHECK(mk::multiply_or_throw(imax / 2, 2) == (imax / 2) * 2);
            CHECK(mk::multiply_or_throw(2, imax / 2) == (imax / 2) * 2);
            CHECK(mk::multiply_or_throw(imin / 2, 2) == imin);
            CHECK(mk::multiply_or_throw(2, imin / 2) == imin);
            CHECK(mk::multiply_or_throw(imin / -2, -2) == imin);
            CHECK(mk::multiply_or_throw(-2, imin / -2) == imin);

            CHECK(mk::multiply_or_throw(imin, 1) == imin);
            CHECK(mk::multiply_or_throw(1, imin) == imin);

            CHECK(mk::multiply_or_throw(imin + 1, -1) == imax);
            CHECK(mk::multiply_or_throw(-1, imin + 1) == imax);

            CHECK(mk::multiply_or_throw<int64_t>(lmax, 1) == lmax);
            CHECK(mk::multiply_or_throw<int64_t>(lmax, 0) == 0);
            CHECK(mk::multiply_or_throw<int64_t>(1, lmax) == lmax);
            CHECK(mk::multiply_or_throw<int64_t>(0, lmax) == 0);
            CHECK(mk::multiply_or_throw<int64_t>(lmax / 2, 2) == (lmax / 2) * 2);
            CHECK(mk::multiply_or_throw<int64_t>(2, lmax / 2) == (lmax / 2) * 2);
            CHECK(mk::multiply_or_throw<int64_t>(lmin / 2, 2) == lmin);
            CHECK(mk::multiply_or_throw<int64_t>(2, lmin / 2) == lmin);
            CHECK(mk::multiply_or_throw<int64_t>(lmin / -2, -2) == lmin);
            CHECK(mk::multiply_or_throw<int64_t>(-2, lmin / -2) == lmin);

            CHECK(mk::multiply_or_throw<int64_t>(lmax / 2 - 1, 2) == (lmax / 2 - 1) * 2); // make sure the implementation doesn't use floating-point numbers

            CHECK(mk::multiply_or_throw<int64_t>(lmin, 1) == lmin);
            CHECK(mk::multiply_or_throw<int64_t>(1, lmin) == lmin);

            CHECK(mk::multiply_or_throw<int64_t>(lmin + 1, -1) == lmax);
            CHECK(mk::multiply_or_throw<int64_t>(-1, lmin + 1) == lmax);

            CHECK(mk::multiply_or_throw(-1, 1) == -1);
            CHECK(mk::multiply_or_throw(1, -1) == -1);
        }

        SECTION("bad")
        {
            CHECK_THROWS_AS(mk::multiply_or_throw(smax, size_t(2)), mk::arithmetic_overflow);
            CHECK_THROWS_AS(mk::multiply_or_throw(size_t(2), smax), mk::arithmetic_overflow);
            CHECK_THROWS_AS(mk::multiply_or_throw(smax / size_t(2), size_t(4)), mk::arithmetic_overflow);
            CHECK_THROWS_AS(mk::multiply_or_throw(size_t(4), smax / size_t(2)), mk::arithmetic_overflow);

            CHECK_THROWS_AS(mk::multiply_or_throw(imin, -1), mk::arithmetic_overflow);
            CHECK_THROWS_AS(mk::multiply_or_throw(-1, imin), mk::arithmetic_overflow);
            CHECK_THROWS_AS(mk::multiply_or_throw(imin + 1, 2), mk::arithmetic_overflow);
            CHECK_THROWS_AS(mk::multiply_or_throw(2, imin + 1), mk::arithmetic_overflow);
            CHECK_THROWS_AS(mk::multiply_or_throw(imin + 1, -2), mk::arithmetic_overflow);
            CHECK_THROWS_AS(mk::multiply_or_throw(-2, imin + 1), mk::arithmetic_overflow);
            CHECK_THROWS_AS(mk::multiply_or_throw(imax - 1, -2), mk::arithmetic_overflow);
            CHECK_THROWS_AS(mk::multiply_or_throw(-2, imax - 1), mk::arithmetic_overflow);
            CHECK_THROWS_AS(mk::multiply_or_throw(imax - 1, 2), mk::arithmetic_overflow);
            CHECK_THROWS_AS(mk::multiply_or_throw(2, imax - 1), mk::arithmetic_overflow);

            CHECK_THROWS_AS(mk::multiply_or_throw<int64_t>(lmin, -1), mk::arithmetic_overflow);
            CHECK_THROWS_AS(mk::multiply_or_throw<int64_t>(-1, lmin), mk::arithmetic_overflow);
            CHECK_THROWS_AS(mk::multiply_or_throw<int64_t>(lmin + 1, 2), mk::arithmetic_overflow);
            CHECK_THROWS_AS(mk::multiply_or_throw<int64_t>(2, lmin + 1), mk::arithmetic_overflow);
            CHECK_THROWS_AS(mk::multiply_or_throw<int64_t>(lmin + 1, -2), mk::arithmetic_overflow);
            CHECK_THROWS_AS(mk::multiply_or_throw<int64_t>(-2, lmin + 1), mk::arithmetic_overflow);
            CHECK_THROWS_AS(mk::multiply_or_throw<int64_t>(lmax - 1, -2), mk::arithmetic_overflow);
            CHECK_THROWS_AS(mk::multiply_or_throw<int64_t>(-2, lmax - 1), mk::arithmetic_overflow);
            CHECK_THROWS_AS(mk::multiply_or_throw<int64_t>(lmax - 1, 2), mk::arithmetic_overflow);
            CHECK_THROWS_AS(mk::multiply_or_throw<int64_t>(2, lmax - 1), mk::arithmetic_overflow);

                // This check is to foil the idea of comparing the result to check for overflow, wrongly assuming that a wrapped multiplication
                // violates monotonicity.
                // 5¹³ =  0x48C27395
                // 5¹⁴ = 0x16BCC41E9, which truncates to something still greater than 5¹³
            CHECK_THROWS_AS(mk::multiply_or_throw<int32_t>(0x48C27395L, 5), mk::arithmetic_overflow);
            CHECK_THROWS_AS(mk::multiply_or_throw(uint32_t(0x48C27395UL), uint32_t(5)), mk::arithmetic_overflow);
                // 3⁴⁰ =  0xA8B8B452291FE821
                // 3⁴¹ = 0x1FA2A1CF67B5FB863, which truncates to something still greater than 3⁴⁰
            CHECK_THROWS_AS(mk::multiply_or_throw(uint64_t(0xA8B8B452291FE821ULL), uint64_t(3)), mk::arithmetic_overflow);
        }
    }

    SECTION("powi")
    {
        SECTION("good")
        {
            CHECK(mk::powi_or_throw(size_t(0), size_t(0)) == size_t(1));
            CHECK(mk::powi_or_throw(size_t(2), size_t(0)) == size_t(1));
            CHECK(mk::powi_or_throw(size_t(0), size_t(3)) == size_t(0));
            CHECK(mk::powi_or_throw(size_t(1), size_t(3)) == size_t(1));
            CHECK(mk::powi_or_throw(size_t(2), size_t(3)) == size_t(8));
            CHECK(mk::powi_or_throw(smax, size_t(1)) == smax);
            CHECK_NOTHROW(mk::powi_or_throw(size_t(2), smaxlog));

            CHECK(mk::powi_or_throw(0, 0) == 1);
            CHECK(mk::powi_or_throw(2, 0) == 1);
            CHECK(mk::powi_or_throw(0, 3) == 0);
            CHECK(mk::powi_or_throw(1, 3) == 1);
            CHECK(mk::powi_or_throw(2, 3) == 8);
            CHECK(mk::powi_or_throw(-1, 3) == -1);
            CHECK(mk::powi_or_throw(imin, 1) == imin);
            CHECK(mk::powi_or_throw(imax, 1) == imax);
            CHECK_NOTHROW(mk::powi_or_throw(2, imaxlog));
            CHECK(mk::powi_or_throw(-2, iminlog) == imin);
        }

        SECTION("bad")
        {
            CHECK_THROWS_AS(mk::powi_or_throw(size_t(2), smaxlog + 1), mk::arithmetic_overflow);
            CHECK_THROWS_AS(mk::powi_or_throw(smax, size_t(2)), mk::arithmetic_overflow);

            CHECK_THROWS_AS(mk::powi_or_throw(0, -1), gsl::fail_fast); // domain error
            CHECK_THROWS_AS(mk::powi_or_throw(2, imaxlog + 1), mk::arithmetic_overflow);
            CHECK_THROWS_AS(mk::powi_or_throw(-2, iminlog + 1), mk::arithmetic_overflow);
            CHECK_THROWS_AS(mk::powi_or_throw(imax, 2), mk::arithmetic_overflow);
            CHECK_THROWS_AS(mk::powi_or_throw(imin, 2), mk::arithmetic_overflow);

                // This check is to foil the idea of comparing the result to check for overflow, wrongly assuming that a wrapped multiplication
                // violates monotonicity.
                // 5¹³ =  0x48C27395
                // 5¹⁴ = 0x16BCC41E9, which truncates to something still greater than 5¹³
            CHECK(mk::powi_or_throw(uint32_t(5), uint32_t(13)) == 0x48C27395UL);
            CHECK_THROWS_AS(mk::powi_or_throw(uint32_t(5), uint32_t(14)), mk::arithmetic_overflow);
                // 3⁴⁰ =  0xA8B8B452291FE821
                // 3⁴¹ = 0x1FA2A1CF67B5FB863, which truncates to something still greater than 3⁴⁰
                // This check also fails with a rounding error if the implementation uses floating-point numbers.
            CHECK(mk::powi_or_throw(uint64_t(3), uint64_t(40)) == 0xA8B8B452291FE821ULL);
            CHECK_THROWS_AS(mk::powi_or_throw(uint64_t(3), uint64_t(41)), mk::arithmetic_overflow);
        }
    }

    SECTION("ceili")
    {
        CHECK(mk::ceili(0, 3) == 0);
        CHECK(mk::ceili(1, 3) == 3);
        CHECK(mk::ceili(2, 3) == 3);
        CHECK(mk::ceili(3, 3) == 3);
        CHECK(mk::ceili(4, 3) == 6);

        CHECK(mk::ceili(0u, 3u) == 0u);
        CHECK(mk::ceili(1u, 3u) == 3u);
        CHECK(mk::ceili(2u, 3u) == 3u);
        CHECK(mk::ceili(3u, 3u) == 3u);
        CHECK(mk::ceili(4u, 3u) == 6u);
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
        CHECK_THROWS_AS(( lu8 = gsl::narrow< uint8_t>(uint16_t(u16))), gsl::narrowing_error);

            // int <-> int
        CHECK_THROWS_AS(( li8 = gsl::narrow<  int8_t>( int16_t(i16n))), gsl::narrowing_error);
        CHECK_THROWS_AS(( li8 = gsl::narrow<  int8_t>( int16_t(i16p))), gsl::narrowing_error);

            // uint <-> int
        CHECK_THROWS_AS((lu16 = gsl::narrow<uint16_t>(  int8_t( i8n))), gsl::narrowing_error);
        CHECK_THROWS_AS(( lu8 = gsl::narrow< uint8_t>(  int8_t( i8n))), gsl::narrowing_error);
        CHECK_THROWS_AS(( li8 = gsl::narrow<  int8_t>( uint8_t( u8 ))), gsl::narrowing_error);
        CHECK_THROWS_AS(( lu8 = gsl::narrow< uint8_t>( int16_t( i8n))), gsl::narrowing_error);
        CHECK_THROWS_AS(( lu8 = gsl::narrow< uint8_t>( int16_t(i16n))), gsl::narrowing_error);
        CHECK_THROWS_AS(( lu8 = gsl::narrow< uint8_t>( int16_t(i16p))), gsl::narrowing_error);
        CHECK_THROWS_AS(( li8 = gsl::narrow<  int8_t>(uint16_t( u8 ))), gsl::narrowing_error);
        CHECK_THROWS_AS(( li8 = gsl::narrow<  int8_t>(uint16_t(u16 ))), gsl::narrowing_error);
    }
}
