
#include <makeshift/variant2.hpp>

#include <catch2/catch.hpp>


namespace mk = makeshift;


enum class Precision { single, double_ };

struct Params
{
    Precision precision;
    int gangSize;
    int numThreadsX;
    int numThreadsY;
};

TEST_CASE("variant2")
{
    
    SECTION("foreach")
    {
        makeshift::detail::object_t<Params> values;
        constexpr auto a1 = (values = { { Precision::single, 4, 32, 32 }, { Precision::double_, 2, 32, 32 } });
        constexpr auto a2 = (values(&Params::precision) = { Precision::single, Precision::double_ })
                          * (values(&Params::gangSize) = { 1, 2, 4 })
                          * (values(&Params::numThreadsX, &Params::numThreadsY) = { { 16, 16 }, { 32, 32 } });
        constexpr auto allValues = makeshift::detail::_values_in(a2);
    }
    
}
