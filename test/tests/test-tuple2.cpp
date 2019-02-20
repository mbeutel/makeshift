
#include <string>
#include <functional> // for plus<>

#include <makeshift/tuple2.hpp>

#include <catch.hpp>


using namespace std::literals;

namespace mk = makeshift;


TEST_CASE("tuple", "[flags]")
{
    auto numbers = std::tuple{ 2, 3u };
    SECTION("foreach")
    {
        int sum = 0;
        mk::tuple_foreach2([&](auto x) { sum += int(x); }, numbers);
        CHECK(sum == 5);
    }
    SECTION("transform")
    {
        auto square = mk::tuple_transform2([](auto x) { return x*x; }, numbers);
        CHECK(square == std::tuple{ 4, 9u });
    }
    SECTION("index")
    {
        int offset = 100;
        auto result_tuple = std::tuple{ 0, 0, 0 };
        auto lhs_tuple = std::tuple{ 10, 20, 30 };
        auto rhs_scalar = 1;
        mk::tuple_foreach2([offset, rhs = rhs_scalar](auto& result, auto lhs, std::size_t index)
        {
            result = lhs + rhs + int(index*offset);
        }, result_tuple, lhs_tuple, mk::tuple_index);
        CHECK(result_tuple == std::tuple{ 11, 121, 231 });
    }
    SECTION("array-transform")
    {
    }
}
