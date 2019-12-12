
#include <makeshift/enum.hpp>

#include <catch2/catch.hpp>


namespace mk = makeshift;


namespace
{


enum class Vegetables
{
    potato = 1,
    tomato = 2,
    garlic = 4,
    onion = 8,
    chili = 16,
    spicy = garlic | chili
};
MAKESHIFT_DEFINE_ENUM_BITMASK_OPERATORS(Vegetables)


using FloatTypes = mk::type_enum<float, double>;


} // anonymous namespace


TEST_CASE("enum")
{
    SECTION("bitmask")
    {
        Vegetables v = Vegetables::potato;
        CHECK((v | v) == Vegetables::potato);
        CHECK((v & ~v) == Vegetables{ });
        CHECK((v ^ v) == Vegetables{ });
        CHECK((v |= v) == Vegetables::potato);
        CHECK((v &= v) == Vegetables::potato);
        CHECK((v ^= v) == Vegetables{ });
        CHECK(v == Vegetables{ });
    }

    SECTION("type-enum")
    {
        FloatTypes floatTypes = mk::type_c<float>;
        CHECK(floatTypes != mk::type_c<double>);

            // The following switch statement should be warning-free. Omitting one of the cases should yield a warning.
        switch (floatTypes)
        {
        case mk::type<float>{ }:
            break;
        case mk::type_c<double>:
            break;
        }
    }
}
