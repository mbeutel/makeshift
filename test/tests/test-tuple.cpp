
#include <functional> // for plus<>

#include <makeshift/tuple.hpp>

#include <catch.hpp>

namespace ac = asc::cptools;

void testTupleReduce(void)
{
    auto sumTuple = ac::tuple_freduce(std::plus<int>{ });
    auto numbers = std::make_tuple(2, 3u);
    int sum = sumTuple(0, numbers);
    CHECK(sum == 5);
}
void testTupleForeach(void)
{
    int sum = 0;
    auto sumUp = ac::tuple_foreach([&](auto x) { sum += int(x); });
    auto numbers = std::make_tuple(2, 3u);
    sumUp(numbers);
    CHECK(sum == 5);
}
void testTupleMap(void)
{
    auto square = ac::tuple_fmap([](auto x) { return x*x; });
    auto numbers = std::make_tuple(2, 3u);
    auto squaredNumbers = square(numbers);
    CHECK(squaredNumbers == std::make_tuple(4, 9u));
}

TEST_CASE("tuple", "[flags]")
{
    SECTION("reduce")
    {
        testTupleReduce();
    }
    SECTION("foreach")
    {
        testTupleForeach();
    }
    SECTION("map")
    {
        testTupleMap();
    }
}
