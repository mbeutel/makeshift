
#include <tuple>
#include <optional>
#include <string_view>
#include <type_traits>

#include <gsl-lite/gsl-lite.hpp>

#include <makeshift/tuple.hpp>
#include <makeshift/constval.hpp>
#include <makeshift/metadata.hpp>

#include <catch2/catch_test_macros.hpp>


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
        mk::make_value_tuple(
            mk::value_tuple{ Color6::red, "red", "red color" },
            mk::value_tuple{ Color6::green, "green", "green color" }
        ),
    };
}

enum class Color7 { red, green };
constexpr auto
reflect(gsl::type_identity<Color7>)
{
    return mk::make_value_tuple(
        mk::value_tuple{ Color7::red, "red", "red color" },
        mk::value_tuple{ Color7::green, "green", "green color" }
    );
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
    return mk::make_value_tuple(
        mk::value_tuple{ &COO3::i, "i", "row index" },
        mk::value_tuple{ &COO3::j, "j", "column index" },
        mk::value_tuple{ &COO3::v, "v", "element" }
    );
}

struct COO4 { int i; int j; double v; };
constexpr auto
reflect(gsl::type_identity<COO4>)
{
    return mk::value_tuple{
        "COO4",
        "COO 4",
        mk::make_value_tuple(
            mk::value_tuple{ &COO4::i, "i", "row index" },
            mk::value_tuple{ &COO4::j, "j", "column index" },
            mk::value_tuple{ &COO4::v, "v", "element" }
        ),
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
        mk::make_value_tuple(
            mk::value_tuple{ &SubCOO2::v2, "v2", "element" }
        )
    };
}

struct AltLayoutSubCOO1
{
    double v;
    double v2;
    int i;
    int j;
};
constexpr auto
reflect(gsl::type_identity<AltLayoutSubCOO1>)
{
    return mk::make_value_tuple(
        mk::value_tuple{ &AltLayoutSubCOO1::v,  "v"  },
        mk::value_tuple{ &AltLayoutSubCOO1::v2, "v2" },
        mk::value_tuple{ &AltLayoutSubCOO1::i,  "i"  },
        mk::value_tuple{ &AltLayoutSubCOO1::j,  "j"  }
    );
}

struct AltLayoutCOO1
{
    double v;
    int i;
    int j;

};
constexpr auto
reflect(gsl::type_identity<AltLayoutCOO1>)
{
    return mk::make_value_tuple(
        mk::value_tuple{ &AltLayoutCOO1::v,  "v"  },
        mk::value_tuple{ &AltLayoutCOO1::i,  "i"  },
        mk::value_tuple{ &AltLayoutCOO1::j,  "j"  }
    );
}


TEST_CASE("enum metadata")
{
    using namespace std::literals;

    CHECK_FALSE(mk::metadata::is_available_for<Color0>());
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::name<Color0>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::description<Color0>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::bases<Color0>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::values<Color0>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_names<Color0>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_descriptions<Color0>()));

    CHECK(mk::metadata::is_available_for<Color1>());
    CHECK(mk::metadata::name<Color1>() == "Color1"sv);
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::description<Color1>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::bases<Color1>()));
    CHECK(mk::metadata::is_available(mk::metadata::values<Color1>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_names<Color1>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_descriptions<Color1>()));

    CHECK(mk::metadata::name<Color2>() == "Color2");
    CHECK(mk::metadata::description<Color2>() == "color 2");
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::bases<Color2>()));
    CHECK(mk::metadata::values<Color2>() == std::array{ Color2::red, Color2::green });
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_names<Color2>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_descriptions<Color2>()));

    CHECK_FALSE(mk::metadata::is_available(mk::metadata::name<Color3>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::description<Color3>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::bases<Color3>()));
    CHECK(mk::metadata::values<Color3>() == std::array{ Color3::red, Color3::green });
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_names<Color3>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_descriptions<Color3>()));

    CHECK_FALSE(mk::metadata::is_available(mk::metadata::name<Color4>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::description<Color4>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::bases<Color4>()));
    CHECK(mk::metadata::values<Color4>() == std::array{ Color4::red, Color4::green });
    CHECK(mk::metadata::value_names<Color4>() == std::array{ "red"sv, "green"sv });
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_descriptions<Color4>()));

    CHECK(mk::metadata::find_value_index(Color4::red) == 0);
    CHECK(mk::metadata::find_value_index(Color4::green) == 1);
    CHECK(mk::metadata::search_value_index(Color4(42)) == -1);
    CHECK_THROWS_AS(mk::metadata::find_value_index(Color4(42)), gsl::fail_fast);

    CHECK(mk::metadata::name<Color5>() == "Color5");
    CHECK(mk::metadata::description<Color5>() == "color 5");
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::bases<Color5>()));
    CHECK(mk::metadata::values<Color5>() == std::array{ Color5::red, Color5::green });
    CHECK(mk::metadata::value_names<Color5>() == std::array{ "red"sv, "green"sv });
    CHECK(mk::metadata::value_descriptions<Color5>() == std::array{ "red color"sv, "green color"sv });

    CHECK(mk::metadata::name<Color6>() == "Color6");
    CHECK(mk::metadata::description<Color6>() == "color 6");
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::bases<Color6>()));
    CHECK(mk::metadata::values<Color6>() == std::array{ Color6::red, Color6::green });
    CHECK(mk::metadata::value_names<Color6>() == std::array{ "red"sv, "green"sv });
    CHECK(mk::metadata::value_descriptions<Color6>() == std::array{ "red color"sv, "green color"sv });

    CHECK_FALSE(mk::metadata::is_available(mk::metadata::name<Color7>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::description<Color7>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::bases<Color7>()));
    CHECK(mk::metadata::values<Color7>() == std::array{ Color7::red, Color7::green });
    CHECK(mk::metadata::value_names<Color7>() == std::array{ "red"sv, "green"sv });
    CHECK(mk::metadata::value_descriptions<Color7>() == std::array{ "red color"sv, "green color"sv });
}

TEST_CASE("struct metadata")
{
    using namespace std::literals;

    CHECK_FALSE(mk::metadata::is_available_for<COO0>());
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::name<COO0>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::description<COO0>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::bases<COO0>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::values<COO0>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_names<COO0>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::value_descriptions<COO0>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::exclusive_members<COO0>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::exclusive_member_names<COO0>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::exclusive_member_descriptions<COO0>()));

    CHECK(mk::metadata::is_available_for<COO1>());
    CHECK(mk::metadata::bases<COO1>() == std::tuple{ });
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::name<COO1>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::description<COO1>()));
    CHECK(mk::metadata::exclusive_members<COO1>() == mk::value_tuple{ &COO1::i, &COO1::j, &COO1::v });

    CHECK(mk::metadata::name<COO2>() == "COO2");
    CHECK(mk::metadata::description<COO2>() == "COO 2");
    CHECK(mk::metadata::bases<COO2>() == std::tuple{ });
    CHECK(mk::metadata::exclusive_members<COO2>() == mk::value_tuple{ &COO2::i, &COO2::j, &COO2::v });
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::exclusive_member_names<COO2>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::exclusive_member_descriptions<COO2>()));

    CHECK_FALSE(mk::metadata::is_available(mk::metadata::name<COO3>()));
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::description<COO3>()));
    CHECK(mk::metadata::bases<COO3>() == std::tuple{ });
    CHECK(mk::metadata::exclusive_members<COO3>() == mk::value_tuple{ &COO3::i, &COO3::j, &COO3::v });
    CHECK(mk::metadata::exclusive_member_names<COO3>() == std::array{ "i"sv, "j"sv, "v"sv });
    CHECK(mk::metadata::exclusive_member_descriptions<COO3>() == std::array{ "row index"sv, "column index"sv, "element"sv });

    CHECK(mk::metadata::name<COO4>() == "COO4");
    CHECK(mk::metadata::description<COO4>() == "COO 4");
    CHECK(mk::metadata::bases<COO4>() == std::tuple{ });
    CHECK(mk::metadata::exclusive_members<COO4>() == mk::value_tuple{ &COO4::i, &COO4::j, &COO4::v });
    CHECK(mk::metadata::exclusive_member_names<COO4>() == std::array{ "i"sv, "j"sv, "v"sv });
    CHECK(mk::metadata::exclusive_member_descriptions<COO4>() == std::array{ "row index"sv, "column index"sv, "element"sv });

    static_assert(std::is_same_v<decltype(mk::metadata::bases<SubCOO0a>()), std::tuple<gsl::type_identity<COO4>>>);

    static_assert(std::is_same_v<decltype(mk::metadata::bases<SubCOO0b>()), mk::value_tuple<gsl::type_identity<COO4>>>);

    static_assert(std::is_same_v<decltype(mk::metadata::bases<SubCOO1>()), mk::value_tuple<gsl::type_identity<COO4>>>);
    CHECK(mk::metadata::exclusive_members<SubCOO1>() == mk::value_tuple{ &SubCOO1::v2 });
    CHECK(mk::metadata::members<mk::value_tuple, SubCOO1>() == mk::value_tuple{ &COO4::i, &COO4::j, &COO4::v, &SubCOO1::v2 });
    CHECK_FALSE(mk::metadata::is_available(mk::metadata::member_names<SubCOO1>()));

    static_assert(std::is_same_v<decltype(mk::metadata::bases<SubCOO2>()), mk::value_tuple<gsl::type_identity<COO4>>>);
    CHECK(mk::metadata::exclusive_members<SubCOO2>() == mk::value_tuple{ &SubCOO2::v2 });
    CHECK(mk::metadata::exclusive_member_names<SubCOO2>() == std::array{ "v2"sv });
    CHECK(mk::metadata::members<mk::value_tuple, SubCOO2>() == mk::value_tuple{ &COO4::i, &COO4::j, &COO4::v, &SubCOO2::v2 });
    CHECK(mk::metadata::member_names<SubCOO2>() == std::array{ "i"sv, "j"sv, "v"sv, "v2"sv });

    SECTION("tie_members()")
    {
        COO1 coo1{ 4, 5, 6. };
        CHECK(mk::tie_members(coo1) == std::tuple{ 4, 5, 6. });
        mk::tie_members(coo1) = std::tuple{ 1, 2, 3. };
        CHECK(coo1.i == 1);
        CHECK(coo1.j == 2);
        CHECK(coo1.v == 3.);
        COO2 coo2;
        mk::tie_members(coo2) = mk::tie_members(coo1);
        const COO2 ccoo2 = coo2;
        CHECK(mk::tie_members(ccoo2) == mk::tie_members(coo1));

        SubCOO1 sc1{ COO4{ 1, 2, 3. }, 4. };
        SubCOO2 sc2{ COO4{ 5, 6, 7. }, 8. };
        CHECK(mk::tie_members(sc1) != mk::tie_members(sc2));
        mk::tie_members(sc2) = mk::tie_members(sc1);
        CHECK(mk::tie_members(sc1) == mk::tie_members(sc2));
        sc2.v = 7.;
        CHECK(mk::tie_members(sc1) != mk::tie_members(sc2));
        sc2.v = 3.;
        CHECK(mk::tie_members(sc1) == mk::tie_members(sc2));
        sc2.v2 = 8.;
        CHECK(mk::tie_members(sc1) != mk::tie_members(sc2));

        AltLayoutSubCOO1 alsc1a{ };
        mk::template_for(
            [&alsc1a, &sc2](auto memberC, auto memberNameC)
            {
                constexpr auto altMember = mk::metadata::find_member_by_name<AltLayoutSubCOO1>(memberNameC);
                alsc1a.*altMember = sc2.*memberC();
            },
            MAKESHIFT_CONSTVAL(mk::metadata::members<SubCOO2>()), MAKESHIFT_CONSTVAL(mk::metadata::member_names<SubCOO2>()));
        CHECK(alsc1a.i == sc2.i);
        CHECK(alsc1a.j == sc2.j);
        CHECK(alsc1a.v == sc2.v);
        CHECK(alsc1a.v2 == sc2.v2);
        AltLayoutSubCOO1 alsc1b{ };
        auto subNamesC = MAKESHIFT_CONSTVAL(mk::metadata::member_names<SubCOO2>());
        mk::tie_members_by_name(alsc1b, subNamesC) = mk::tie_members_by_name(sc2, subNamesC);
        CHECK(alsc1b.i == sc2.i);
        CHECK(alsc1b.j == sc2.j);
        CHECK(alsc1b.v == sc2.v);
        CHECK(alsc1b.v2 == sc2.v2);
        AltLayoutCOO1 alc1a{ };
        mk::template_for(
            [&alc1a, &sc2]([[maybe_unused]] auto memberC, auto memberNameC)
            {
                constexpr auto altMember = mk::metadata::search_member_by_name<AltLayoutCOO1>(memberNameC);
                if constexpr (mk::metadata::is_available(altMember))
                {
                    alc1a.*altMember = sc2.*memberC();
                }
            },
            MAKESHIFT_CONSTVAL(mk::metadata::members<SubCOO2>()), MAKESHIFT_CONSTVAL(mk::metadata::member_names<SubCOO2>()));
        CHECK(alc1a.i == sc2.i);
        CHECK(alc1a.j == sc2.j);
        CHECK(alc1a.v == sc2.v);
        AltLayoutCOO1 alc1b{ };
        auto namesC = MAKESHIFT_CONSTVAL(mk::metadata::member_names<COO4>());
        mk::tie_members_by_name(alc1b, namesC) = mk::tie_members_by_name(sc2, namesC);
        CHECK(alc1b.i == sc2.i);
        CHECK(alc1b.j == sc2.j);
        CHECK(alc1b.v == sc2.v);
    }
}


} // anonymous namespace
