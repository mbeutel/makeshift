
#include <tuple>
#include <optional>
#include <string_view>
#include <type_traits>

#include <gsl-lite/gsl-lite.hpp>

#include <makeshift/metadata.hpp>


namespace {

namespace mk = ::makeshift;
namespace gsl = ::gsl_lite;


enum class Color0 { red, green };

enum class Color1 { red, green };
constexpr auto
reflect(gsl::type_identity<Color1>)
{
    return "Color1";
}

enum class Color2 { red, green };
constexpr auto
reflect(gsl::type_identity<Color2>)
{
    return std::tuple{
        "Color2",
        std::array{ Color2::red, Color2::green },
        "color 2"
    };
}

enum class Color3 { red, green };
constexpr auto
reflect(gsl::type_identity<Color3>)
{
    return std::array{ Color3::red, Color3::green };
}

enum class Color4 { red, green };
constexpr auto
reflect(gsl::type_identity<Color4>)
{
    return std::array{
        std::pair{ Color4::red, "red" },
        std::pair{ Color4::green, "green" }
    };
}

enum class Color5 { red, green };
constexpr auto
reflect(gsl::type_identity<Color5>)
{
    return std::tuple{
        "Color5",
        "color 5",
        std::array{
            std::tuple{ Color5::red, "red", "red color" },
            std::tuple{ Color5::green, "green", "green color" }
        },
    };
}


TEST_CASE("enum metadata")
{
    using namespace std::literals;

    constexpr auto md0 = mk::metadata_v<Color0>;
    CHECK_FALSE(mk::metadata::is_available(md0));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::name(md0)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::description(md0)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::values<Color1>(md0)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_names<Color1>(md0)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_descriptions<Color1>(md0)));

    constexpr auto md1 = mk::metadata_v<Color1>;
    CHECK(mk::metadata::name(md1) == "Color1"sv);
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::description(md1)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::values<Color1>(md1)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_names<Color1>(md1)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_descriptions<Color1>(md1)));

    constexpr auto md2 = mk::metadata_v<Color2>;
    CHECK(mk::metadata::name(md2) == "Color2");
    CHECK(mk::metadata::description(md2) == "color 2");
    CHECK(mk::metadata::values<Color2>(md2) == std::array{ Color2::red, Color2::green });
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_names<Color2>(md2)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_descriptions<Color2>(md2)));

    constexpr auto md3 = mk::metadata_v<Color3>;
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::name(md3)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::description(md3)));
    CHECK(mk::metadata::values<Color3>(md3) == std::array{ Color3::red, Color3::green });
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_names<Color3>(md3)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_descriptions<Color3>(md3)));

    constexpr auto md4 = mk::metadata_v<Color4>;
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::name(md4)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::description(md4)));
    CHECK(mk::metadata::values<Color4>(md4) == std::array{ Color4::red, Color4::green });
    CHECK(mk::metadata::value_names<Color4>(md4) == std::array{ "red"sv, "green"sv });
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_descriptions<Color4>(md4)));

    constexpr auto md5 = mk::metadata_v<Color5>;
    CHECK(mk::metadata::name(md5) == "Color5");
    CHECK(mk::metadata::description(md5) == "color 5");
    CHECK(mk::metadata::values<Color5>(md5) == std::array{ Color5::red, Color5::green });
    CHECK(mk::metadata::value_names<Color5>(md5) == std::array{ "red"sv, "green"sv });
    CHECK(mk::metadata::value_descriptions<Color5>(md5) == std::array{ "red color"sv, "green color"sv });
}


} // anonymous namespace
