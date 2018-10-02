
#include <chrono>

#include <makeshift/streamable.hpp>
#include <makeshift/utility.hpp>
#include <makeshift/string.hpp>
#include <makeshift/quantity.hpp>

#include <makeshift/serializers/quantity.hpp>
#include <makeshift/serializers/stream.hpp>

#include <catch.hpp>


using namespace std::literals;

namespace mk = makeshift;
using namespace makeshift::literals;


TEST_CASE("quantity", "[flags]")
{
    auto serializer = mk::chain(
        mk::quantity_serializer<>{ },
        mk::stream_serializer<>{ }
    );

    SECTION("time")
    {
        std::chrono::milliseconds t1 = 512ms;
        CHECK(to_string(streamable(t1, serializer)) == "512 ms");
        std::chrono::milliseconds t2{ };
        from_string(streamable(t2, serializer), "512000us");
        CHECK(t2 == t1);
    }
    
    SECTION("quantity_unit")
    {
        CHECK(mk::unit_to_string("GB/s"_unit) == "GB/s");
        CHECK(mk::unit_from_string("GB/s") == "GB/s"_unit);
    }

    SECTION("arbitrary units")
    {
        mk::quantity<int, "GB/s"_unit> v1{ 1 };
        CHECK(to_string(streamable(v1, serializer)) == "1 GB/s");
        mk::quantity<int, "GB/s"_unit> v2{ };
        from_string(streamable(v2, serializer), "1 GB/s");
        CHECK(v2.value == v1.value);
        mk::dynamic_quantity<int> v3{ };
        from_string(streamable(v3, serializer), "1 GB/s");
        CHECK(v3.value == v1.value);
        CHECK(v3.unit == v1.unit);
    }
}
