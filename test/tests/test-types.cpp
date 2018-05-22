
#include <functional>

#include <makeshift/types.hpp>

#include <catch.hpp>

namespace mk = makeshift;
using namespace makeshift::literals;

namespace
{

struct Vegetables : mk::define_flags<Vegetables>
{
    static constexpr flag potato { 1 };
    static constexpr flag tomato { 2 };

    static constexpr flag garlic { 4 };
    static constexpr flag onion { 8 };
    static constexpr flag chili { 16 };

    static constexpr flags spicy = garlic | chili;
};
using Ratatouille = Vegetables::flags;

int assembleDecimal(const std::tuple<mk::named<int, "ones"_kw>, mk::named<int, "tens"_kw>>& args)
{
    return 10*mk::get<"tens"_kw>(args) + mk::get<"ones"_kw>(args);
}

} // anonymous namespace

TEST_CASE("types", "[flags]")
{
    SECTION("flags-enum")
    {
        
    }
    SECTION("keys")
    {
        using NamedInt = mk::named<int, "width"_kw>;
        NamedInt val1 { 42 };
        NamedInt val2 = { mk::name<"width"_kw> = 42 };
        CHECK(val1.value == val2.value);
        CHECK("make"_kw + "shift"_kw == "makeshift"_kw);
        CHECK("foo"_kw / "bar"_kw == "foo/bar"_kw);
    }
    SECTION("kwargs")
    {
        CHECK(assembleDecimal(std::make_tuple(mk::name<"ones"_kw> = 7, mk::name<"tens"_kw> = 8)) == 87);
    }
    SECTION("tuple")
    {
        auto numbers = std::make_tuple(0, 1u, 2);
        mk::tuple_foreach(numbers, [](auto& val)
        {
            ++val;
        });
        CHECK(numbers == std::make_tuple(1, 2u, 3));
        auto squares = mk::tuple_map(numbers, [](auto x) { return x*x; });
        CHECK(squares == std::make_tuple(1, 4u, 9));
        auto sumOfSquares = mk::tuple_reduce(squares, 0, std::plus<>());
        CHECK(sumOfSquares == 14);
    }
}
