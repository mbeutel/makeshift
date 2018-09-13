
#include <string>
#include <functional> // for plus<>

#include <makeshift/functional.hpp> // for overload()
#include <makeshift/tuple.hpp>

#include <catch.hpp>


using namespace std::literals;

namespace mk = makeshift;


void testTupleReduce(void)
{
    auto sumTuple = mk::tuple_fold(std::plus<int>{ });
    auto numbers = std::make_tuple(2, 3u);
    int sum = sumTuple(0, numbers);
    CHECK(sum == 5);
}
void testTupleForeach(void)
{
    int sum = 0;
    auto sumUp = mk::tuple_foreach([&](auto x) { sum += int(x); });
    auto numbers = std::make_tuple(2, 3u);
    sumUp(numbers);
    CHECK(sum == 5);
}
void testTupleMap(void)
{
    auto square = mk::tuple_map([](auto x) { return x*x; });
    auto numbers = std::make_tuple(2, 3u);
    auto squaredNumbers = square(numbers);
    CHECK(squaredNumbers == std::make_tuple(4, 9u));
}
void testIndex(void)
{
    int offset = 100;
    auto myFunc = mk::tuple_foreach([offset](auto& result, auto lhs, auto rhs, std::size_t index)
    {
        result = lhs + rhs + int(index*offset);
    });
    auto result_tuple = std::make_tuple(0, 0, 0);
    auto lhs_tuple = std::make_tuple(10, 20, 30);
    auto rhs_scalar = 1;
    myFunc(result_tuple, lhs_tuple, rhs_scalar, mk::tuple_index);
    CHECK(result_tuple == std::make_tuple(11, 121, 231));
}
void testOverloads(void)
{
    auto typenamesOf = mk::tuple_map(
        mk::overload(
            [](int) { return "int"s; },
            [](float) { return "float"s; }
        )
    );
    auto someTuple = std::make_tuple(42, 13.37f, 0);
    auto joinStrings = mk::tuple_fold(
        [](const std::string& lhs, const std::string& rhs)
        {
            return !lhs.empty()
                ? lhs + ", " + rhs
                : rhs;
        });
    auto typenames = typenamesOf(someTuple);
    auto result = joinStrings("", typenames);
    CHECK(result == "int, float, int");
}
void testFunctionVariants(void)
{
    auto numbers = std::make_tuple(0, 1u, 2);
    mk::tuple_foreach(numbers, [](auto& val)
    {
        ++val;
    });
    CHECK(numbers == std::make_tuple(1, 2u, 3));
    auto squares = mk::tuple_map(numbers, [](auto x) { return x*x; });
    CHECK(squares == std::make_tuple(1, 4u, 9));
    auto sumOfSquares = mk::tuple_fold(squares, 0, std::plus<>());
    CHECK(sumOfSquares == 14);
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
    SECTION("index")
    {
        testIndex();
    }
    SECTION("overloads")
    {
        testOverloads();
    }
    SECTION("function-variants")
    {
        testFunctionVariants();
    }
}
