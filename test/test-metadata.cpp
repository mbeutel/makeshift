
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

struct SubCOO0a : COO4 { };
constexpr auto
reflect(gsl::type_identity<SubCOO0a>)
{
    return gsl::type_identity<COO4>{ };
}

struct SubCOO0b : COO4 { };
constexpr auto
reflect(gsl::type_identity<SubCOO0b>)
{
    return mk::value_tuple{ gsl::type_identity<COO4>{ } };
}

struct SubCOO1 : COO4 { double v2; };
constexpr auto
reflect(gsl::type_identity<SubCOO1>)
{
    return mk::value_tuple{
        mk::value_tuple{ gsl::type_identity<COO4>{ } },
        mk::value_tuple{ &SubCOO1::v2 }
    };
}

struct SubCOO2 : COO4 { double v2; };
constexpr auto
reflect(gsl::type_identity<SubCOO2>)
{
    return mk::value_tuple{
        mk::value_tuple{ gsl::type_identity<COO4>{ } },
        mk::value_tuple{
            mk::value_tuple{ &SubCOO2::v2, "v2", "element" }
        }
    };
}


TEST_CASE("enum metadata")
{
    using namespace std::literals;

    constexpr auto md0 = mk::metadata_v<Color0>;
    CHECK_FALSE(mk::metadata::is_available(md0));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::name<Color0>(md0)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::description<Color0>(md0)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::bases<Color0>(md0)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::values<Color0>(md0)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_names<Color0>(md0)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_descriptions<Color0>(md0)));

    constexpr auto md1 = mk::metadata_v<Color1>;
    CHECK(mk::metadata::name<Color1>(md1) == "Color1"sv);
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::description<Color1>(md1)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::bases<Color1>(md1)));
    CHECK(mk::metadata::is_available(mk::metadata::values<Color1>(md1)));
    CHECK(mk::metadata::is_available(mk::metadata::value_names<Color1>(md1)));
    CHECK(mk::metadata::is_available(mk::metadata::value_descriptions<Color1>(md1)));

    constexpr auto md2 = mk::metadata_v<Color2>;
    CHECK(mk::metadata::name<Color2>(md2) == "Color2");
    CHECK(mk::metadata::description<Color2>(md2) == "color 2");
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::bases<Color2>(md2)));
    CHECK(mk::metadata::values<Color2>(md2) == std::array{ Color2::red, Color2::green });
    CHECK(mk::metadata::is_available(mk::metadata::value_names<Color2>(md2)));
    CHECK(mk::metadata::is_available(mk::metadata::value_descriptions<Color2>(md2)));

    constexpr auto md3 = mk::metadata_v<Color3>;
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::name<Color3>(md3)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::description<Color3>(md3)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::bases<Color3>(md3)));
    CHECK(mk::metadata::values<Color3>(md3) == std::array{ Color3::red, Color3::green });
    CHECK(mk::metadata::is_available(mk::metadata::value_names<Color3>(md3)));
    CHECK(mk::metadata::is_available(mk::metadata::value_descriptions<Color3>(md3)));

    constexpr auto md4 = mk::metadata_v<Color4>;
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::name<Color4>(md4)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::description<Color4>(md4)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::bases<Color4>(md4)));
    CHECK(mk::metadata::values<Color4>(md4) == std::array{ Color4::red, Color4::green });
    CHECK(mk::metadata::value_names<Color4>(md4) == std::array{ "red"sv, "green"sv });
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_descriptions<Color4>(md4)));

    constexpr auto md5 = mk::metadata_v<Color5>;
    CHECK(mk::metadata::name<Color5>(md5) == "Color5");
    CHECK(mk::metadata::description<Color5>(md5) == "color 5");
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::bases<Color5>(md5)));
    CHECK(mk::metadata::values<Color5>(md5) == std::array{ Color5::red, Color5::green });
    CHECK(mk::metadata::value_names<Color5>(md5) == std::array{ "red"sv, "green"sv });
    CHECK(mk::metadata::value_descriptions<Color5>(md5) == std::array{ "red color"sv, "green color"sv });

    constexpr auto md6 = mk::metadata_v<Color6>;
    CHECK(mk::metadata::name<Color6>(md6) == "Color6");
    CHECK(mk::metadata::description<Color6>(md6) == "color 6");
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::bases<Color6>(md6)));
    CHECK(mk::metadata::values<Color6>(md6) == std::array{ Color6::red, Color6::green });
    CHECK(mk::metadata::value_names<Color6>(md6) == std::array{ "red"sv, "green"sv });
    CHECK(mk::metadata::value_descriptions<Color6>(md6) == std::array{ "red color"sv, "green color"sv });

    constexpr auto md7 = mk::metadata_v<Color7>;
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::name<Color7>(md7)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::description<Color7>(md7)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::bases<Color7>(md7)));
    CHECK(mk::metadata::values<Color7>(md7) == std::array{ Color7::red, Color7::green });
    CHECK(mk::metadata::value_names<Color7>(md7) == std::array{ "red"sv, "green"sv });
    CHECK(mk::metadata::value_descriptions<Color7>(md7) == std::array{ "red color"sv, "green color"sv });
}

TEST_CASE("struct metadata")
{
    using namespace std::literals;

    constexpr auto md0 = mk::metadata_v<COO0>;
    CHECK_FALSE(mk::metadata::is_available(md0));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::name<COO0>(md0)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::description<COO0>(md0)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::bases<COO0>(md0)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::values<COO0>(md0)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_names<COO0>(md0)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_descriptions<COO0>(md0)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::members<COO0>(md0)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::member_names<COO0>(md0)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::member_descriptions<COO0>(md0)));

    constexpr auto md1 = mk::metadata_v<COO1>;
    CHECK(mk::metadata::bases<COO1>(md1) == std::tuple{ });
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::name<COO1>(md1)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::description<COO1>(md1)));
    CHECK(mk::metadata::members<COO1>(md1) == mk::value_tuple{ &COO1::i, &COO1::j, &COO1::v });

    constexpr auto md2 = mk::metadata_v<COO2>;
    CHECK(mk::metadata::name<COO2>(md2) == "COO2");
    CHECK(mk::metadata::description<COO2>(md2) == "COO 2");
    CHECK(mk::metadata::bases<COO2>(md2) == std::tuple{ });
    CHECK(mk::metadata::members<COO2>(md2) == mk::value_tuple{ &COO2::i, &COO2::j, &COO2::v });
    CHECK(mk::metadata::is_available(mk::metadata::member_names<COO2>(md2)));
    CHECK(mk::metadata::is_available(mk::metadata::member_descriptions<COO2>(md2)));

    constexpr auto md3 = mk::metadata_v<COO3>;
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::name<COO3>(md3)));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::description<COO3>(md3)));
    CHECK(mk::metadata::bases<COO3>(md3) == std::tuple{ });
    CHECK(mk::metadata::members<COO3>(md3) == mk::value_tuple{ &COO3::i, &COO3::j, &COO3::v });
    CHECK(mk::metadata::member_names<COO3>(md3) == std::array{ "i"sv, "j"sv, "v"sv });
    CHECK(mk::metadata::member_descriptions<COO3>(md3) == std::array{ "row index"sv, "column index"sv, "element"sv });

    constexpr auto md4 = mk::metadata_v<COO4>;
    CHECK(mk::metadata::name<COO4>(md4) == "COO4");
    CHECK(mk::metadata::description<COO4>(md4) == "COO 4");
    CHECK(mk::metadata::bases<COO4>(md4) == std::tuple{ });
    CHECK(mk::metadata::members<COO4>(md4) == mk::value_tuple{ &COO4::i, &COO4::j, &COO4::v });
    CHECK(mk::metadata::member_names<COO4>(md4) == std::array{ "i"sv, "j"sv, "v"sv });
    CHECK(mk::metadata::member_descriptions<COO4>(md4) == std::array{ "row index"sv, "column index"sv, "element"sv });

    constexpr auto md5 = mk::metadata_v<SubCOO0a>;
    static_assert(std::is_same_v<decltype(mk::metadata::bases<SubCOO0a>(md5)), std::tuple<gsl::type_identity<COO4>>>);

    constexpr auto md6 = mk::metadata_v<SubCOO0b>;
    static_assert(std::is_same_v<decltype(mk::metadata::bases<SubCOO0b>(md6)), mk::value_tuple<gsl::type_identity<COO4>>>);

    constexpr auto md7 = mk::metadata_v<SubCOO1>;
    static_assert(std::is_same_v<decltype(mk::metadata::bases<SubCOO1>(md7)), mk::value_tuple<gsl::type_identity<COO4>>>);
    CHECK(mk::metadata::members<SubCOO1>(md7) == mk::value_tuple{ &SubCOO1::v2 });

    constexpr auto md8 = mk::metadata_v<SubCOO2>;
    static_assert(std::is_same_v<decltype(mk::metadata::bases<SubCOO2>(md8)), mk::value_tuple<gsl::type_identity<COO4>>>);
    CHECK(mk::metadata::members<SubCOO2>(md8) == std::tuple{ &SubCOO2::v2 });

#if gsl_CPP20_OR_GREATER
    SECTION("tie()")
    {
        COO1 coo1{ 4, 5, 6. };
        CHECK(mk::tie(coo1) == std::tuple{ 4, 5, 6. });
        mk::tie(coo1) = std::tuple{ 1, 2, 3. };
        CHECK(coo1.i == 1);
        CHECK(coo1.j == 2);
        CHECK(coo1.v == 3.);
        COO2 coo2;
        mk::tie(coo2) = mk::tie(coo1);
        const COO2 ccoo2 = coo2;
        CHECK(mk::tie(ccoo2) == mk::tie(coo1));

        SubCOO1 sc1{ COO4{ 1, 2, 3. }, 4. };
        SubCOO1 sc2{ COO4{ 5, 6, 7. }, 8. };
        CHECK(mk::tie(sc1) != mk::tie(sc2));
        mk::tie(sc2) = mk::tie(sc1);
        CHECK(mk::tie(sc1) == mk::tie(sc2));
        sc2.v = 7.;
        CHECK(mk::tie(sc1) != mk::tie(sc2));
        sc2.v = 3.;
        CHECK(mk::tie(sc1) == mk::tie(sc2));
        sc2.v2 = 8.;
        CHECK(mk::tie(sc1) != mk::tie(sc2));
    }
#endif // gsl_CPP20_OR_GREATER
}


} // anonymous namespace
