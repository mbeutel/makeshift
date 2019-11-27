
#include <array>
#include <tuple>
#include <limits>
#include <cstdint> // for CHAR_BIT
#include <sstream>

#include <stdint.h>

#include <catch2/catch.hpp>

#include <makeshift/arithmetic.hpp>

#include <gsl/gsl-lite.hpp> // for narrow<>(), fail_fast


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
const volatile size_t smaxlog = mk::log_floori(smax, size_t(2));
const volatile int imin = std::numeric_limits<int>::min(),
                   imax = std::numeric_limits<int>::max();
const volatile int imaxlog = mk::log_floori(imax, 2);
const volatile int iminlog = mk::log_floori(imin / -2, 2) + 1;
const volatile int64_t lmin = std::numeric_limits<int64_t>::min(),
                         lmax = std::numeric_limits<int64_t>::max();
const volatile int64_t lmaxlog = mk::log_floori(lmax, int64_t(2));
const volatile int64_t lminlog = mk::log_floori(lmin / -2, int64_t(2)) + 1;


TEMPLATE_TEST_CASE("ratio_ceili()", "[arithmetic]", unsigned, int)
{
    static constexpr auto iMax = std::numeric_limits<TestType>::max();

    SECTION("enforces preconditions")
    {
        CHECK_THROWS(mk::ratio_ceili(0, 0));
        CHECK_THROWS(mk::ratio_ceili(1, 0));
        CHECK_THROWS(mk::ratio_ceili(2, 0));
        CHECK_THROWS(mk::ratio_ceili(-1, 1));
        CHECK_THROWS(mk::ratio_ceili(-2, 1));
    }

    SECTION("basic correctness")
    {
        auto n = GENERATE(range(TestType(0), TestType(8)));
        auto d = GENERATE(range(TestType(1), TestType(8)));

        CAPTURE(n);
        CAPTURE(d);
        TestType q = mk::ratio_ceili(n, d);
        CHECK(q >= 0);
        CHECK(q * d >= n);
        CHECK(q * d < n + d);
    }

    SECTION("borderline values")
    {
        static_assert(iMax % 2 != 0, "iMax must be odd in a two's complement representation");
        CHECK(mk::ratio_ceili(iMax, TestType(2)) == iMax / 2 + 1);
    }
}

TEMPLATE_TEST_CASE("multiply() for signed types", "[arithmetic]", std::int8_t, std::int32_t, std::int64_t)
{
    static constexpr auto iMin = std::numeric_limits<TestType>::min();
    static constexpr auto iMax = std::numeric_limits<TestType>::max();

    SECTION("basic correctness")
    {
        auto a = GENERATE(range(TestType(-7), TestType(8)));
        auto b = GENERATE(range(TestType(-7), TestType(8)));

        CAPTURE(a);
        CAPTURE(b);
        CHECK(mk::multiply(a, b) == a * b);
    }

    SECTION("borderline values")
    {
        using Tuple = std::tuple<TestType, TestType>;
        auto a_b = GENERATE(
            Tuple{ iMin,               TestType( 0) },
            Tuple{ iMax,               TestType( 0) },
            Tuple{ iMin,               TestType( 1) },
            Tuple{ iMax,               TestType( 1) },
            Tuple{ iMax,               TestType(-1) },
            Tuple{ TestType(iMax / 2), TestType( 2) },
            Tuple{ TestType(iMax / 2), TestType(-2) },
            Tuple{ TestType(iMin / 2), TestType( 2) },
            Tuple{ TestType(iMin / 3), TestType(-2) }
        );
        TestType a, b;
        std::tie(a, b) = a_b;

        CAPTURE(a);
        CAPTURE(b);
        CHECK(mk::multiply(a, b) == a * b);
        CHECK(mk::multiply(b, a) == a * b);
    }

    SECTION("throws on integer overflow")
    {
        using Tuple = std::tuple<TestType, TestType>;
        auto a_b = GENERATE(
            Tuple{ iMin,               TestType( 2) },
            Tuple{ iMin,               TestType(-1) },
            Tuple{ iMin,               TestType(-2) },
            Tuple{ iMax,               TestType( 2) },
            Tuple{ iMax,               TestType(-2) },
            Tuple{ iMin,               TestType(-1) },
            Tuple{ TestType(iMax / 2), TestType( 3) },
            Tuple{ TestType(iMax / 2), TestType(-3) },
            Tuple{ TestType(iMin / 2), TestType( 3) },
            Tuple{ TestType(iMin / 2), TestType(-2) }
        );
        TestType a, b;
        std::tie(a, b) = a_b;

        CAPTURE(a);
        CAPTURE(b);
        CHECK_THROWS(mk::multiply(a, b));
        CHECK_THROWS(mk::multiply(b, a));
    }
}

TEMPLATE_TEST_CASE("multiply() for unsigned types", "[arithmetic]", std::uint8_t, std::uint32_t, std::uint64_t)
{
    static constexpr TestType iMax = std::numeric_limits<TestType>::max();

    SECTION("basic correctness")
    {
        auto a = GENERATE(range(TestType(0), TestType(8)));
        auto b = GENERATE(range(TestType(0), TestType(8)));

        CAPTURE(a);
        CAPTURE(b);
        CHECK(mk::multiply(a, b) == a * b);
    }

    SECTION("borderline values")
    {
        using Tuple = std::tuple<TestType, TestType>;
        auto a_b = GENERATE(
            Tuple{ iMax,               TestType(0) },
            Tuple{ iMax,               TestType(1) },
            Tuple{ TestType(iMax / 2), TestType(2) }
        );
        TestType a, b;
        std::tie(a, b) = a_b;

        CAPTURE(a);
        CAPTURE(b);
        CHECK(mk::multiply(a, b) == a * b);
        CHECK(mk::multiply(b, a) == a * b);
    }

    SECTION("throws on integer overflow")
    {
        using Tuple = std::tuple<TestType, TestType>;
        auto a_b = GENERATE(
            Tuple{ iMax,               TestType(2) },
            Tuple{ TestType(iMax / 2), TestType(3) }
        );
        TestType a, b;
        std::tie(a, b) = a_b;

        CAPTURE(a);
        CAPTURE(b);
        CHECK_THROWS(mk::multiply(a, b));
        CHECK_THROWS(mk::multiply(b, a));
    }
}

TEMPLATE_TEST_CASE("sqrti()", "[arithmetic]", unsigned, int)
{
    static constexpr auto iMax = std::numeric_limits<TestType>::max();

    SECTION("enforces preconditions")
    {
        CHECK_THROWS(mk::detail::sqrti(-1));
        CHECK_THROWS(mk::detail::sqrti(-2));
    }

    SECTION("basic correctness")
    {
        auto x = GENERATE(range(TestType(0), TestType(100)));

        CAPTURE(x);

        auto sqrtX = mk::detail::sqrti(x);
        CHECK(sqrtX*sqrtX <= x);
        CHECK((sqrtX + 1)*(sqrtX + 1) > x);
    }

    SECTION("borderline values")
    {
        auto x = GENERATE(as<TestType>{ }, iMax / 2, iMax - 2, iMax - 1, iMax);

        CAPTURE(x);

        auto sqrtX = mk::detail::sqrti(x);
        CHECK(sqrtX*sqrtX <= x);
        CHECK(sqrtX*sqrtX > x - 2*sqrtX - 1); // equivalent to `(sqrtX + 1)*(sqrtX + 1) > x` but without the possibility of overflow
    }
}

TEMPLATE_TEST_CASE("square()", "[arithmetic]", int)
{
    static constexpr auto iMax = std::numeric_limits<TestType>::max();
    static constexpr auto sqrtIMax = mk::detail::sqrti(iMax);

    SECTION("basic correctness")
    {
        auto x = GENERATE(range(TestType(-12), TestType(13)), TestType(sqrtIMax - 1), sqrtIMax);

        CAPTURE(x);

        auto sqrX = mk::square(x);
        CHECK(sqrX == x*x);
    }

    SECTION("throws on integer overflow")
    {
        int x = GENERATE(sqrtIMax + 1, sqrtIMax * 2);

        CAPTURE(x);

        CHECK_THROWS(mk::square(x));
    }
}

TEMPLATE_TEST_CASE("log_floori()", "[arithmetic]", int)
{
    static constexpr auto iMax = std::numeric_limits<TestType>::max();

    SECTION("basic correctness")
    {
        using Tuple = std::tuple<TestType, TestType, TestType>;
        auto x_b_log = GENERATE(
            Tuple{ 1,        2,        0                                         },
            Tuple{ 2,        2,        1                                         },
            Tuple{ 3,        2,        1                                         },
            Tuple{ 8,        2,        3                                         },
            Tuple{ iMax,     2,        TestType(sizeof(TestType) * CHAR_BIT - 2) },
            Tuple{ iMax,     iMax,     1                                         },
            Tuple{ iMax - 1, iMax,     0                                         },
            Tuple{ iMax,     iMax - 1, 1                                         },
            Tuple{ 1,        iMax,     0                                         },
            Tuple{ 2,        iMax,     0                                         }
        );
        TestType x, b, log;
        std::tie(x, b, log) = x_b_log;
        
        CAPTURE(x);
        CAPTURE(b);
        CHECK(mk::log_floori(x, b) == log);
    }
}

TEMPLATE_TEST_CASE("log_ceili()", "[arithmetic]", int)
{
    static constexpr auto iMax = std::numeric_limits<TestType>::max();

    SECTION("basic correctness")
    {
        using Tuple = std::tuple<TestType, TestType, TestType>;
        auto x_b_log = GENERATE(
            Tuple{ 1,        2,        0                                         },
            Tuple{ 2,        2,        1                                         },
            Tuple{ 3,        2,        2                                         },
            Tuple{ 8,        2,        3                                         },
            Tuple{ iMax,     2,        TestType(sizeof(TestType) * CHAR_BIT) - 1 },
            Tuple{ iMax,     iMax,     1                                         },
            Tuple{ iMax - 1, iMax,     1                                         },
            Tuple{ iMax,     iMax - 1, 2                                         },
            Tuple{ 1,        iMax,     0                                         },
            Tuple{ 2,        iMax,     1                                         }
        );
        TestType x, b, log;
        std::tie(x, b, log) = x_b_log;
        
        CAPTURE(x);
        CAPTURE(b);
        CHECK(mk::log_ceili(x, b) == log);
    }
}

TEMPLATE_TEST_CASE("factorize_floori()", "[arithmetic]", int)
{
    SECTION("basic correctness")
    {
        using Tuple = std::tuple<TestType, TestType, TestType, TestType, TestType, TestType>;
        auto x_a_b_r_ea_eb = GENERATE(
            Tuple{ 1, 2, 3, 0, 0, 0 },
            Tuple{ 2, 2, 3, 0, 1, 0 },
            Tuple{ 3, 2, 3, 0, 0, 1 },
            Tuple{ 4, 2, 3, 0, 2, 0 },
            Tuple{ 5, 2, 3, 1, 2, 0 },
            Tuple{ 6, 2, 3, 0, 1, 1 },
            Tuple{ 7, 2, 3, 1, 1, 1 },
            Tuple{ 8, 2, 3, 0, 3, 0 }
        );
        TestType x, a, b, r, ea, eb;
        std::tie(x, a, b, r, ea, eb) = x_a_b_r_ea_eb;
        
        CAPTURE(x);
        CAPTURE(a);
        CAPTURE(b);
        CHECK(mk::factorize_floori(x, a, b) == mk::factorization<TestType, 2>{ r, { mk::factor<TestType>{ a, ea }, mk::factor<TestType>{ b, eb } } });
    }
}

TEMPLATE_TEST_CASE("factorize_ceili()", "[arithmetic]", int)
{
    SECTION("basic correctness")
    {
        using Tuple = std::tuple<TestType, TestType, TestType, TestType, TestType, TestType>;
        auto x_a_b_r_ea_eb = GENERATE(
            Tuple{ 1, 2, 3, 0, 0, 0 },
            Tuple{ 2, 2, 3, 0, 1, 0 },
            Tuple{ 3, 2, 3, 0, 0, 1 },
            Tuple{ 4, 2, 3, 0, 2, 0 },
            Tuple{ 5, 2, 3, 1, 1, 1 },
            Tuple{ 6, 2, 3, 0, 1, 1 },
            Tuple{ 7, 2, 3, 1, 3, 0 },
            Tuple{ 8, 2, 3, 0, 3, 0 }
        );
        TestType x, a, b, r, ea, eb;
        std::tie(x, a, b, r, ea, eb) = x_a_b_r_ea_eb;
        
        CAPTURE(x);
        CAPTURE(a);
        CAPTURE(b);
        CHECK(mk::factorize_ceili(x, a, b) == mk::factorization<TestType, 2>{ r, { mk::factor<TestType>{ a, ea }, mk::factor<TestType>{ b, eb } } });
    }
}

TEST_CASE("negate()")
{
    SECTION("throws on integer overflow")
    {
        CHECK_THROWS_AS(mk::negate_or_throw(u8), std::system_error);
        CHECK_THROWS_AS(mk::negate_or_throw(i8nm), std::system_error);
    }
}


TEST_CASE("arithmetic")
{
    volatile uint8_t lu8;
    volatile uint64_t lu64;
    volatile int8_t li8;
    volatile int64_t li64;

    SECTION("add")
    {
        CHECK_THROWS_AS(lu8 = mk::add_or_throw(u8, u8), std::system_error);
        CHECK_THROWS_AS(li8 = mk::add_or_throw(i8p, i8p), std::system_error);
        CHECK_THROWS_AS(li8 = mk::add_or_throw(i8n, i8n), std::system_error);
        CHECK_THROWS_AS(lu64 = mk::add_or_throw(u64, u64), std::system_error);
        CHECK_THROWS_AS(li64 = mk::add_or_throw(i64p, i64p), std::system_error);
        CHECK_THROWS_AS(li64 = mk::add_or_throw(i64n, i64n), std::system_error);
    }

    SECTION("subtract")
    {
        CHECK_THROWS_AS(lu8 = mk::subtract_or_throw(zu8, u8), std::system_error);
        CHECK_THROWS_AS(li8 = mk::subtract_or_throw(i8p, i8n), std::system_error);
        CHECK_THROWS_AS(li8 = mk::subtract_or_throw(i8n, i8p), std::system_error);
        CHECK_THROWS_AS(lu64 = mk::subtract_or_throw(zu64, u64), std::system_error);
        CHECK_THROWS_AS(li64 = mk::subtract_or_throw(i64p, i64n), std::system_error);
        CHECK_THROWS_AS(li64 = mk::subtract_or_throw(i64n, i64p), std::system_error);
    }

    SECTION("multiply")
    {
        CHECK_THROWS_AS(lu8 = mk::multiply_or_throw(u8, u8), std::system_error);
        CHECK_THROWS_AS(li8 = mk::multiply_or_throw(i8p, i8p), std::system_error);
        CHECK_THROWS_AS(li8 = mk::multiply_or_throw(i8p, i8n), std::system_error);
        CHECK_THROWS_AS(li8 = mk::multiply_or_throw(i8n, i8n), std::system_error);
        CHECK_THROWS_AS(lu64 = mk::multiply_or_throw(u64, u64), std::system_error);
        CHECK_THROWS_AS(li64 = mk::multiply_or_throw(i64p, i64p), std::system_error);
        CHECK_THROWS_AS(li64 = mk::multiply_or_throw(i64p, i64n), std::system_error);
        CHECK_THROWS_AS(li64 = mk::multiply_or_throw(i64n, i64n), std::system_error);
    }

    SECTION("divide")
    {
        CHECK_THROWS_AS(lu8 = mk::divide_or_throw(u8, zu8), gsl::fail_fast);
        CHECK_THROWS_AS(li8 = mk::divide_or_throw(i8p, zi8), gsl::fail_fast);
        CHECK_THROWS_AS(li8 = mk::divide_or_throw(i8nm, int8_t(-1)), std::system_error);
    }

    SECTION("modulo")
    {
        CHECK_THROWS_AS(lu8 = mk::modulo_or_throw(u8, zu8), gsl::fail_fast);
        CHECK_THROWS_AS(li8 = mk::modulo_or_throw(i8p, zi8), gsl::fail_fast);
        CHECK_THROWS_AS(li8 = mk::modulo_or_throw(i8nm, int8_t(-1)), std::system_error);
    }

    SECTION("shift_left")
    {
        CHECK_THROWS_AS(lu8 = mk::shift_left_or_throw(u8, uint8_t(10)), std::system_error);
        CHECK_THROWS_AS(li8 = mk::shift_left_or_throw(zi8, int8_t(10)), std::system_error);
        CHECK_THROWS_AS(li8 = mk::shift_left_or_throw(zi8, int8_t(-1)), gsl::fail_fast);
        CHECK_THROWS_AS(li8 = mk::shift_left_or_throw(i8n, int8_t(0)), gsl::fail_fast);
        CHECK_THROWS_AS(li8 = mk::shift_left_or_throw(i8p, int8_t(2)), std::system_error);
    }

    SECTION("shift_right")
    {
        CHECK_THROWS_AS(lu8 = mk::shift_right_or_throw(u8, uint8_t(10)), std::system_error);
        CHECK_THROWS_AS(li8 = mk::shift_right_or_throw(zi8, int8_t(10)), std::system_error);
        CHECK_THROWS_AS(li8 = mk::shift_right_or_throw(zi8, int8_t(-1)), gsl::fail_fast);
        CHECK_THROWS_AS(li8 = mk::shift_right_or_throw(i8n, int8_t(0)), gsl::fail_fast);
    }

    SECTION("multiply-extended")
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
            CHECK_THROWS_AS(mk::multiply_or_throw(smax, size_t(2)), std::system_error);
            CHECK_THROWS_AS(mk::multiply_or_throw(size_t(2), smax), std::system_error);
            CHECK_THROWS_AS(mk::multiply_or_throw(smax / size_t(2), size_t(4)), std::system_error);
            CHECK_THROWS_AS(mk::multiply_or_throw(size_t(4), smax / size_t(2)), std::system_error);

            CHECK_THROWS_AS(mk::multiply_or_throw(imin, -1), std::system_error);
            CHECK_THROWS_AS(mk::multiply_or_throw(-1, imin), std::system_error);
            CHECK_THROWS_AS(mk::multiply_or_throw(imin + 1, 2), std::system_error);
            CHECK_THROWS_AS(mk::multiply_or_throw(2, imin + 1), std::system_error);
            CHECK_THROWS_AS(mk::multiply_or_throw(imin + 1, -2), std::system_error);
            CHECK_THROWS_AS(mk::multiply_or_throw(-2, imin + 1), std::system_error);
            CHECK_THROWS_AS(mk::multiply_or_throw(imax - 1, -2), std::system_error);
            CHECK_THROWS_AS(mk::multiply_or_throw(-2, imax - 1), std::system_error);
            CHECK_THROWS_AS(mk::multiply_or_throw(imax - 1, 2), std::system_error);
            CHECK_THROWS_AS(mk::multiply_or_throw(2, imax - 1), std::system_error);

            CHECK_THROWS_AS(mk::multiply_or_throw<int64_t>(lmin, -1), std::system_error);
            CHECK_THROWS_AS(mk::multiply_or_throw<int64_t>(-1, lmin), std::system_error);
            CHECK_THROWS_AS(mk::multiply_or_throw<int64_t>(lmin + 1, 2), std::system_error);
            CHECK_THROWS_AS(mk::multiply_or_throw<int64_t>(2, lmin + 1), std::system_error);
            CHECK_THROWS_AS(mk::multiply_or_throw<int64_t>(lmin + 1, -2), std::system_error);
            CHECK_THROWS_AS(mk::multiply_or_throw<int64_t>(-2, lmin + 1), std::system_error);
            CHECK_THROWS_AS(mk::multiply_or_throw<int64_t>(lmax - 1, -2), std::system_error);
            CHECK_THROWS_AS(mk::multiply_or_throw<int64_t>(-2, lmax - 1), std::system_error);
            CHECK_THROWS_AS(mk::multiply_or_throw<int64_t>(lmax - 1, 2), std::system_error);
            CHECK_THROWS_AS(mk::multiply_or_throw<int64_t>(2, lmax - 1), std::system_error);

                // This check is to foil the idea of comparing the result to check for overflow, wrongly assuming that a wrapped multiplication
                // violates monotonicity.
                // 5¹³ =  0x48C27395
                // 5¹⁴ = 0x16BCC41E9, which truncates to something still greater than 5¹³
            CHECK_THROWS_AS(mk::multiply_or_throw<int32_t>(0x48C27395L, 5), std::system_error);
            CHECK_THROWS_AS(mk::multiply_or_throw(uint32_t(0x48C27395UL), uint32_t(5)), std::system_error);
                // 3⁴⁰ =  0xA8B8B452291FE821
                // 3⁴¹ = 0x1FA2A1CF67B5FB863, which truncates to something still greater than 3⁴⁰
            CHECK_THROWS_AS(mk::multiply_or_throw(uint64_t(0xA8B8B452291FE821ULL), uint64_t(3)), std::system_error);
        }
    }

    SECTION("powi")
    {
        SECTION("good")
        {
            for (int b = 1; b <= 5; ++b)
            {
                CAPTURE(b);
                int r = 1;
                for (int e = 0; e <= 6; ++e)
                {
                    CAPTURE(e);
                    CHECK(mk::powi_or_throw(b, e) == r);
                    CHECK(int(mk::powi_or_throw(unsigned(b), unsigned(e))) == r);
                    r *= b;
                }
            }

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
            CHECK_THROWS(mk::powi_or_throw(int8_t(-128), 2));
            CHECK(mk::powi_or_throw(int8_t(-2), 7) == int8_t(-128));
        }

        SECTION("bad")
        {
            CHECK_THROWS_AS(mk::powi_or_throw(size_t(2), smaxlog + 1), std::system_error);
            CHECK_THROWS_AS(mk::powi_or_throw(smax, size_t(2)), std::system_error);

            CHECK_THROWS_AS(mk::powi_or_throw(0, -1), gsl::fail_fast); // domain error
            CHECK_THROWS_AS(mk::powi_or_throw(2, imaxlog + 1), std::system_error);
            CHECK_THROWS_AS(mk::powi_or_throw(-2, iminlog + 1), std::system_error);
            CHECK_THROWS_AS(mk::powi_or_throw(imax, 2), std::system_error);
            CHECK_THROWS_AS(mk::powi_or_throw(imin, 2), std::system_error);

                // This check is to foil the idea of comparing the result to check for overflow, wrongly assuming that a wrapped multiplication
                // violates monotonicity.
                // 5¹³ =  0x48C27395
                // 5¹⁴ = 0x16BCC41E9, which truncates to something still greater than 5¹³
            CHECK(mk::powi_or_throw(uint32_t(5), uint32_t(13)) == 0x48C27395UL);
            CHECK_THROWS_AS(mk::powi_or_throw(uint32_t(5), uint32_t(14)), std::system_error);
                // 3⁴⁰ =  0xA8B8B452291FE821
                // 3⁴¹ = 0x1FA2A1CF67B5FB863, which truncates to something still greater than 3⁴⁰
                // This check also fails with a rounding error if the implementation uses floating-point numbers.
            CHECK(mk::powi_or_throw(uint64_t(3), uint64_t(40)) == 0xA8B8B452291FE821ULL);
            CHECK_THROWS_AS(mk::powi_or_throw(uint64_t(3), uint64_t(41)), std::system_error);
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
