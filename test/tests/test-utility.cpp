
#include <functional>

#include <makeshift/utility.hpp>
#include <makeshift/tuple.hpp>

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

int assembleDecimal(const std::tuple<mk::named_t<int, "ones"_kw>, mk::named_t<int, "tens"_kw>>& args)
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
        using Width = mk::named_t<int, "width"_kw>;
        Width val1 { 42 };
        Width val2 = { mk::name<"width"_kw> = 42 };
        CHECK(get(val1) == get(val2));
        CHECK("make"_kw + "shift"_kw == "makeshift"_kw);
    }
    SECTION("kwargs")
    {
        CHECK(assembleDecimal({ mk::name<"ones"_kw> = 7, mk::name<"tens"_kw> = 8 }) == 87);
    }
}
