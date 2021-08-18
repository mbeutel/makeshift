
#include <tuple>
#include <optional>
#include <string_view>
#include <type_traits>

#include <gsl-lite/gsl-lite.hpp>

#include <makeshift/tuple.hpp>
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
    return mk::value_tuple{
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
    return mk::value_tuple{
        "Color5",
        "color 5",
        std::array{
            mk::value_tuple{ Color5::red, "red", "red color" },
            mk::value_tuple{ Color5::green, "green", "green color" }
        },
    };
}

enum class Color6 { red, green };
constexpr auto
reflect(gsl::type_identity<Color6>)
{
    return mk::value_tuple{
        "Color6",
        "color 6",
        mk::value_tuple{
            mk::value_tuple{ Color6::red, "red", "red color" },
            mk::value_tuple{ Color6::green, "green", "green color" }
        },
    };
}

enum class Color7 { red, green };
constexpr auto
reflect(gsl::type_identity<Color7>)
{
    return mk::value_tuple{
        mk::value_tuple{ Color7::red, "red", "red color" },
        mk::value_tuple{ Color7::green, "green", "green color" }
    };
}

struct COO0 { int i; int j; double v; };

struct COO1 { int i; int j; double v; };
constexpr auto
reflect(gsl::type_identity<COO1>)
{
    return mk::value_tuple{ &COO1::i, &COO1::j, &COO1::v };
}

struct COO2 { int i; int j; double v; };
constexpr auto
reflect(gsl::type_identity<COO2>)
{
    return mk::value_tuple{
        "COO2",
        mk::value_tuple{ &COO2::i, &COO2::j, &COO2::v },
        "COO 2"
    };
}

struct COO3 { int i; int j; double v; };
constexpr auto
reflect(gsl::type_identity<COO3>)
{
    return mk::value_tuple{
        mk::value_tuple{ &COO3::i, "i", "row index" },
        mk::value_tuple{ &COO3::j, "j", "column index" },
        mk::value_tuple{ &COO3::v, "v", "element" }
    };
}

struct COO4 { int i; int j; double v; };
constexpr auto
reflect(gsl::type_identity<COO4>)
{
    return mk::value_tuple{
        "COO4",
        "COO 4",
        mk::value_tuple{
            mk::value_tuple{ &COO4::i, "i", "row index" },
            mk::value_tuple{ &COO4::j, "j", "column index" },
            mk::value_tuple{ &COO4::v, "v", "element" }
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
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::values<Color0>(md0)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_names<Color0>(md0)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_descriptions<Color0>(md0)));

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

    constexpr auto md6 = mk::metadata_v<Color6>;
    CHECK(mk::metadata::name(md6) == "Color6");
    CHECK(mk::metadata::description(md6) == "color 6");
    CHECK(mk::metadata::values<Color6>(md6) == std::array{ Color6::red, Color6::green });
    CHECK(mk::metadata::value_names<Color6>(md6) == std::array{ "red"sv, "green"sv });
    CHECK(mk::metadata::value_descriptions<Color6>(md6) == std::array{ "red color"sv, "green color"sv });

    constexpr auto md7 = mk::metadata_v<Color7>;
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::name(md7)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::description(md7)));
    CHECK(mk::metadata::values<Color7>(md7) == std::array{ Color7::red, Color7::green });
    CHECK(mk::metadata::value_names<Color7>(md7) == std::array{ "red"sv, "green"sv });
    CHECK(mk::metadata::value_descriptions<Color7>(md7) == std::array{ "red color"sv, "green color"sv });
}

TEST_CASE("struct metadata")
{
    using namespace std::literals;

    constexpr auto md0 = mk::metadata_v<COO0>;
    CHECK_FALSE(mk::metadata::is_available(md0));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::name(md0)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::description(md0)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::values<COO0>(md0)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_names<COO0>(md0)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_descriptions<COO0>(md0)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::members<COO0>(md0)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::member_names<COO0>(md0)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::member_descriptions<COO0>(md0)));

    constexpr auto md1 = mk::metadata_v<COO1>;
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::name(md1)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::description(md1)));
    CHECK(mk::metadata::members<COO1>(md1) == mk::value_tuple{ &COO1::i, &COO1::j, &COO1::v });

    constexpr auto md2 = mk::metadata_v<COO2>;
    CHECK(mk::metadata::name(md2) == "COO2");
    CHECK(mk::metadata::description(md2) == "COO 2");
    CHECK(mk::metadata::members<COO2>(md2) == mk::value_tuple{ &COO2::i, &COO2::j, &COO2::v });
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::member_names<COO2>(md2)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::member_descriptions<COO2>(md2)));

    constexpr auto md3 = mk::metadata_v<COO3>;
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::name(md3)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::description(md3)));
    CHECK(mk::metadata::members<COO3>(md3) == mk::value_tuple{ &COO3::i, &COO3::j, &COO3::v });
    CHECK(mk::metadata::member_names<COO3>(md3) == std::array{ "i"sv, "j"sv, "v"sv });
    CHECK(mk::metadata::member_descriptions<COO3>(md3) == std::array{ "row index"sv, "column index"sv, "element"sv });

    constexpr auto md4 = mk::metadata_v<COO4>;
    CHECK(mk::metadata::name(md4) == "COO4");
    CHECK(mk::metadata::description(md4) == "COO 4");
    CHECK(mk::metadata::members<COO4>(md4) == mk::value_tuple{ &COO4::i, &COO4::j, &COO4::v });
    CHECK(mk::metadata::member_names<COO4>(md4) == std::array{ "i"sv, "j"sv, "v"sv });
    CHECK(mk::metadata::member_descriptions<COO4>(md4) == std::array{ "row index"sv, "column index"sv, "element"sv });
}


} // anonymous namespace
