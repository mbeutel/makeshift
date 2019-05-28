
#include <makeshift/utility2.hpp>

#include <catch2/catch.hpp>


namespace mk = makeshift;


namespace
{

struct Vegetables : mk::define_flags<Vegetables>
{
    static constexpr auto potato = flag(1);
    static constexpr auto tomato = flag(2);

    static constexpr auto garlic = flag(4);
    static constexpr auto onion = flag(8);
    static constexpr auto chili = flag(16);

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
        FloatTypes floatTypes = mk::type_c<float>;
        CHECK(floatTypes != mk::type_c<double>);
    }
}
