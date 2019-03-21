
#include <makeshift/utility2.hpp>

#include <catch2/catch.hpp>


namespace mk = makeshift;


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


using FloatTypes = mk::type_enum<float, double>;


} // anonymous namespace


TEST_CASE("utility2", "[flags]")
{
    SECTION("flags-enum")
    {
        
    }

    SECTION("type-enum")
    {
        FloatTypes floatTypes = mk::type_v<float>;
        CHECK(floatTypes != mk::type_v<double>);
    }
}
