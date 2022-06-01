
#include <array>
#include <tuple>
#include <string>
#include <sstream>
#include <string_view>

#include <gsl-lite/gsl-lite.hpp>

#include <makeshift/metadata.hpp>
#include <makeshift/iomanip.hpp>
#include <makeshift/string.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_tostring.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>


namespace {

namespace mk = ::makeshift;
namespace gsl = ::gsl_lite;


enum class Color { none, red, green };
constexpr auto
reflect(gsl::type_identity<Color>)
{
    return std::tuple{
        "Color",
        "color",
        std::array{
            std::pair{ Color::red, "red" },
            std::pair{ Color::green, "green" }
        }
    };
}

TEST_CASE("as_enum()")
{
    SECTION("read")
    {
        auto sstr1 = std::istringstream("  green , ");
        Color color = Color::none;
        sstr1 >> mk::as_enum(color);
        REQUIRE(sstr1.good());
        CHECK(sstr1.get() == ' ');
        CHECK(sstr1.get() == ',');
        CHECK(color == Color::green);

        auto sstr2 = std::istringstream("  bogus+ ");
        sstr2 >> mk::as_enum(color);
        CHECK_FALSE(sstr2.good());
        CHECK(color == Color::green);
    }
    SECTION("write")
    {
        auto sstr1 = std::ostringstream{ };
        sstr1 << mk::as_enum(Color::green);
        CHECK(sstr1.good());
        CHECK(sstr1.str() == "green");
    }
}

TEST_CASE("parse_enum()")
{
    SECTION("pass")
    {
        CHECK(mk::parse_enum<Color>(" red  ") == Color::red);
        CHECK(mk::parse_enum<Color>("  green ") == Color::green);
    }
    SECTION("fail")
    {
        auto str = GENERATE("bogus", " +green", "green+");
        CAPTURE(str);
        CHECK_THROWS_AS(mk::parse_enum<Color>(str), std::runtime_error);
    }
    SECTION("fail-message")
    {
        CHECK_THROWS_MATCHES(mk::parse_enum<Color>("bogus"), std::runtime_error, Catch::Matchers::ExceptionMessageMatcher(
            "color: unknown value 'bogus'; supported values: { 'red', 'green' }"));
    }
}

TEST_CASE("enum_to_string()")
{
    CHECK(mk::enum_to_string(Color::red) == "red");
    CHECK(mk::enum_to_string(Color::green) == "green");
    CHECK_THROWS_AS(mk::enum_to_string(Color::none), gsl::fail_fast);
}


enum class Vegetables
{
    none     = 0,
    tomato   = 0b0001,
    onion    = 0b0010,
    eggplant = 0b0100,
    tomato_onion = tomato | onion,
    other    = 0b1000
};
gsl_DEFINE_ENUM_BITMASK_OPERATORS(Vegetables)
constexpr auto
reflect(gsl::type_identity<Vegetables>)
{
    return std::tuple{
        "Vegetables",
        std::array{
            std::pair{ Vegetables::none, "none" },
            std::pair{ Vegetables::tomato_onion, "tomato_onion" },
            std::pair{ Vegetables::tomato, "tomato" },
            std::pair{ Vegetables::onion, "onion" },
            std::pair{ Vegetables::eggplant, "eggplant" }
        }
    };
}

TEST_CASE("as_flags()")
{
    using namespace std::literals;

    SECTION("read")
    {
        SECTION("pass")
        {
            auto [str, suffix, flags] = GENERATE(
                std::tuple{ " "sv, ""sv, Vegetables::none },
                std::tuple{ "none ,"sv, ","sv, Vegetables::none },
                std::tuple{ " ]"sv, "]"sv, Vegetables::none },
                std::tuple{ "none )"sv, ")"sv, Vegetables::none },
                std::tuple{ " tomato| onion "sv, ""sv, Vegetables::tomato | Vegetables::onion },
                std::tuple{ " tomato +onion "sv, ""sv, Vegetables::tomato | Vegetables::onion },
                std::tuple{ " tomato_onion] "sv, "]"sv, Vegetables::tomato | Vegetables::onion },
                std::tuple{ " eggplant "sv, ""sv, Vegetables::eggplant });
            CAPTURE(str);
            CAPTURE(suffix);
            CAPTURE(flags);
            auto sstr = std::istringstream(std::string(str));
            auto flags_read = Vegetables::other;
            sstr >> mk::as_flags(flags_read);
            REQUIRE(sstr);
            CHECK(flags_read == flags);
            if (!suffix.empty())
            {
                CHECK(sstr.get() == suffix[0]);
            }
        }
        SECTION("fail")
        {
            auto str = GENERATE("x]", "x", "tomato+x", "eggplant|x");
            CAPTURE(str);
            auto sstr = std::istringstream(std::string(str));
            auto flags_read = Vegetables::other;
            sstr >> mk::as_flags(flags_read);
            CHECK_FALSE(sstr);
        }
    }
    SECTION("write")
    {
        auto [flags, str] = GENERATE(
            std::tuple{ Vegetables::none, "none" },
            std::tuple{ Vegetables::eggplant, "eggplant" },
            std::tuple{ Vegetables::tomato_onion, "tomato+onion" },
            std::tuple{ Vegetables::eggplant | Vegetables::tomato, "tomato+eggplant" });
        CAPTURE(flags);
        CAPTURE(str);
        auto sstr = std::ostringstream{ };
        sstr << mk::as_flags(flags);
        REQUIRE(sstr);
        CHECK(sstr.str() == str);
    }
}

TEST_CASE("parse_flags()")
{
    using namespace std::literals;

    SECTION("pass")
    {
        auto [str, flags] = GENERATE(
            std::tuple{ " "sv, Vegetables::none },
            std::tuple{ "none "sv, Vegetables::none },
            std::tuple{ " tomato| onion"sv, Vegetables::tomato | Vegetables::onion },
            std::tuple{ " tomato +onion "sv, Vegetables::tomato | Vegetables::onion },
            std::tuple{ " tomato_onion "sv, Vegetables::tomato | Vegetables::onion },
            std::tuple{ " eggplant "sv, Vegetables::eggplant });
        CAPTURE(str);
        CAPTURE(flags);
        CHECK(mk::parse_flags<Vegetables>(str) == flags);
    }
    SECTION("fail")
    {
        auto str = GENERATE("[none", "x]", "x", "tomato+x", "eggplant|x", "eggplant]", "eggplant ]");
        CAPTURE(str);
        CHECK_THROWS(mk::parse_flags<Vegetables>(str));
    }
    SECTION("fail-message")
    {
        CHECK_THROWS_MATCHES(mk::parse_flags<Vegetables>("bogus"), std::runtime_error, Catch::Matchers::ExceptionMessageMatcher(
            "Vegetables: unknown value 'bogus'; supported values: a '+'-delimited subset of { 'tomato', 'onion', 'eggplant', 'none', 'tomato_onion' }"));
    }
}

TEST_CASE("flags_to_string()")
{
    SECTION("pass")
    {
        auto [flags, str] = GENERATE(
            std::tuple{ Vegetables::none, "none" },
            std::tuple{ Vegetables::eggplant, "eggplant" },
            std::tuple{ Vegetables::tomato_onion, "tomato+onion" },
            std::tuple{ Vegetables::eggplant | Vegetables::tomato, "tomato+eggplant" });
        CAPTURE(flags);
        CAPTURE(str);
        CHECK(mk::flags_to_string(flags) == str);
    }
    SECTION("failfast")
    {
        CHECK_THROWS_AS(mk::flags_to_string(Vegetables::eggplant | Vegetables::other), gsl::fail_fast);
    }
}


} // anonymous namespace
