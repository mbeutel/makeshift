
#include <string>
#include <iostream>
#include <functional> // for plus<>

#include "tuple.hpp"


using namespace std::literals;

namespace ac = asc::cptools;


    // poor man's substitute for a proper unit test framework
[[noreturn]] int fail(void)
{
    throw std::logic_error("runtime check failed");
}
#define CHECK(...) ((void) ((__VA_ARGS__) ? 0 : fail()))


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

void testIndex(void)
{
    int offset = 100;
    auto myFunc = ac::tuple_foreach([offset](auto& result, auto lhs, auto rhs, std::size_t index)
    {
        result = lhs + rhs + int(index*offset);
    });
    auto result_tuple = std::make_tuple(0, 0, 0);
    auto lhs_tuple = std::make_tuple(10, 20, 30);
    auto rhs_scalar = 1;
    myFunc(result_tuple, lhs_tuple, rhs_scalar, ac::tuple_index);
    CHECK(result_tuple == std::make_tuple(11, 121, 231));
}

void testOverloads(void)
{
    auto typenamesOf = ac::tuple_fmap(
        ac::overload(
            [](int) { return "int"s; },
            [](float) { return "float"s; }
        )
    );
    auto someTuple = std::make_tuple(42, 13.37f, 0);
    auto joinStrings = ac::tuple_freduce(
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

void run(void)
{
    testTupleReduce();
    testTupleForeach();
    testTupleMap();
    testIndex();
    testOverloads();
}

int main(void)
{
    try
    {
        run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
