
#include <array>
#include <tuple>
#include <string>
#include <sstream>

#include <gsl-lite/gsl-lite.hpp>

#include <makeshift/metadata.hpp>
#include <makeshift/iomanip.hpp>
#include <makeshift/string.hpp>

#include <catch2/catch.hpp>

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
        CHECK_THROWS_MATCHES(mk::parse_enum<Color>("bogus"), std::runtime_error, Catch::Matchers::Exception::ExceptionMessageMatcher(
            "color: unknown value 'bogus'; supported values: { 'red', 'green' }"));
    }
}

TEST_CASE("enum_to_string()")
{
    CHECK(mk::enum_to_string(Color::red) == "red");
    CHECK(mk::enum_to_string(Color::green) == "green");
    CHECK_THROWS_AS(mk::enum_to_string(Color::none), gsl::fail_fast);
    
}


} // anonymous namespace
