
#include <makeshift/tuple.hpp>
#include <makeshift/variant.hpp>

#include <catch2/catch.hpp>


using namespace std::literals;

namespace mk = makeshift;


TEST_CASE("variant", "[flags]")
{
    auto number = std::variant<int, unsigned>{ 3 };
    SECTION("map_to")
    {
        auto squareVariantToInt = mk::variant_map_to<int>([](auto v) { return int(v*v); });
        CHECK(squareVariantToInt(number) == 9);
    }
    SECTION("map")
    {
        auto square = mk::variant_map([](auto x) { return x*x; });
        CHECK(square(number) == std::variant<int, unsigned>{ 9 });
    }
    SECTION("filter")
    {
        auto signedAlternatives = mk::variant_filter(mk::trait_v<std::is_signed>);
        CHECK(signedAlternatives(number) == std::variant<int>{ 3 });
    }
    SECTION("filter_or_throw")
    {
        auto signedAlternatives = mk::variant_filter_or_throw(mk::trait_v<std::is_signed>, [](auto) { return std::runtime_error(""); });
        CHECK_THROWS_AS(signedAlternatives(number), std::runtime_error);
    }
    SECTION("expand")
    {
        for (int i = 0; i < 3; ++i)
        {
            auto iv = mk::expand(i, mk::type_tuple(mk::constant<0>{ }, mk::constant<1>{ }, mk::constant<2>{ }));
            CHECK(mk::variant_to_scalar<int>(iv) == i);
        }
    }
}
