
#include <functional>

#include <makeshift/utility.hpp>
#include <makeshift/tuple.hpp>

#include <catch2/catch.hpp>

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

int assembleDecimal(const std::tuple<mk::named_t<int, "ones"_k>, mk::named_t<int, "tens"_k>>& args)
{
    return 10*mk::get_by_key<"tens"_k>(args) + mk::get_by_key<"ones"_k>(args);
}

} // anonymous namespace

TEST_CASE("types", "[flags]")
{
    SECTION("flags-enum")
    {
        
    }
    SECTION("keys")
    {
        using Width = mk::named_t<int, "width"_k>;
        Width val1 { 42 };
        Width val2 = { mk::name_v<"width"_k> = 42 };
        CHECK(get(val1) == get(val2));
        CHECK("make"_k + "shift"_k == "makeshift"_k);
    }
    SECTION("kwargs")
    {
        CHECK(assembleDecimal({ mk::name_v<"ones"_k> = 7, mk::name_v<"tens"_k> = 8 }) == 87);
    }
}
