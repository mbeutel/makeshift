
#include <string>
#include <functional> // for plus<>

#include <makeshift/functional.hpp> // for overload()
#include <makeshift/tuple.hpp>

#include <catch2/catch.hpp>


using namespace std::literals;

namespace mk = makeshift;


TEST_CASE("tuple", "[flags]")
{
    auto numbers = std::tuple{ 2, 3u };
    SECTION("reduce")
    {
        auto sumTuple = mk::tuple_fold(std::plus<int>{ });
        CHECK(sumTuple(0, numbers) == 5);
    }
    SECTION("foreach")
    {
        int sum = 0;
        auto sumUp = mk::tuple_foreach([&](auto x) { sum += int(x); });
        sumUp(numbers);
        CHECK(sum == 5);
    }
    SECTION("map")
    {
        auto square = mk::tuple_map([](auto x) { return x*x; });
        CHECK(square(numbers) == std::tuple{ 4, 9u });
    }
    SECTION("index")
    {
        int offset = 100;
        auto myFunc = mk::tuple_foreach([offset](auto& result, auto lhs, auto rhs, std::size_t index)
        {
            result = lhs + rhs + int(index*offset);
        });
        auto result_tuple = std::tuple{ 0, 0, 0 };
        auto lhs_tuple = std::tuple{ 10, 20, 30 };
        auto rhs_scalar = 1;
        myFunc(result_tuple, lhs_tuple, rhs_scalar, mk::tuple_index);
        CHECK(result_tuple == std::tuple{ 11, 121, 231 });
    }
    SECTION("overloads")
    {
        auto typenamesOf = mk::tuple_map(
            mk::overload(
                [](int) { return "int"s; },
                [](float) { return "float"s; }
            )
        );
        auto someTuple = std::tuple{ 42, 13.37f, 0 };
        auto joinStrings = mk::tuple_fold(
            [](const std::string& lhs, const std::string& rhs)
            {
                return !lhs.empty()
                    ? lhs + ", " + rhs
                    : rhs;
            });
        auto typenames = typenamesOf(someTuple);
        CHECK(joinStrings("", typenames) == "int, float, int");
    }
    SECTION("function-variants")
    {
        auto moreNumbers = std::tuple{ 0, 1u, 2 };
        mk::tuple_foreach(moreNumbers, [](auto& val)
        {
            ++val;
        });
        CHECK(moreNumbers == std::tuple{ 1, 2u, 3 });
        auto squares = mk::tuple_map(moreNumbers, [](auto x) { return x*x; });
        CHECK(squares == std::tuple{ 1, 4u, 9 });
        auto sumOfSquares = mk::tuple_fold(squares, 0, std::plus<>());
        CHECK(sumOfSquares == 14);
    }
}
