
#include <tuple>

#include <makeshift/tuple.hpp>
#include <makeshift/stdint.hpp> // for index

#include <catch2/catch.hpp>


namespace mk = makeshift;


TEST_CASE("tuple", "[flags]")
{
    constexpr auto numbers = std::make_tuple(2, 3u);
    SECTION("foreach")
    {
        int sum = 0;
        mk::template_for([&](auto x) { sum += int(x); }, numbers);
        CHECK(sum == 5);
    }
    SECTION("transform")
    {
        auto square = mk::tuple_transform([](auto x) { return x*x; }, numbers);
        CHECK(square == std::make_tuple(4, 9u));
    }
    SECTION("index")
    {
        int offset = 100;
        auto result_tuple = std::make_tuple(0, 0, 0);
        auto lhs_tuple = std::make_tuple(10, 20, 30);
        auto rhs_scalar = 1;
        mk::template_for([offset, rhs = rhs_scalar](auto& result, auto lhs, auto indexC)
        {
            constexpr mk::index index = indexC();
            result = lhs + rhs + int(index*offset);
        }, result_tuple, lhs_tuple, mk::tuple_index);
        CHECK(result_tuple == std::make_tuple(11, 121, 231));
    }
    SECTION("fold")
    {
        int sum = mk::tuple_reduce(numbers, 0, std::plus<int>{ });
        CHECK(sum == 5);
    }
    SECTION("all/any/none")
    {
        bool allGreaterThan0 = mk::tuple_all_of(numbers, [](auto x) { return x > 0; });
        CHECK(allGreaterThan0);
        bool allGreaterThan2 = mk::tuple_all_of(numbers, [](auto x) { return x > 2; });
        CHECK_FALSE(allGreaterThan2);
        bool anyGreaterThan2 = mk::tuple_any_of(numbers, [](auto x) { return x > 2; });
        CHECK(anyGreaterThan2);
        bool anyGreaterThan4 = mk::tuple_any_of(numbers, [](auto x) { return x > 4; });
        CHECK_FALSE(anyGreaterThan4);
        bool noneGreaterThan2 = mk::tuple_none_of(numbers, [](auto x) { return x > 2; });
        CHECK_FALSE(noneGreaterThan2);
        bool noneGreaterThan4 = mk::tuple_none_of(numbers, [](auto x) { return x > 4; });
        CHECK(noneGreaterThan4);
    }
}
