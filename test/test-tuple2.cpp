
#include <makeshift/tuple2.hpp>

#include <catch2/catch.hpp>


namespace mk = makeshift;


TEST_CASE("tuple2", "[flags]")
{
    constexpr auto numbers = std::tuple{ 2, 3u };
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
        auto square = mk::array_transform2([](auto x) { return int(x*x); }, numbers);
        CHECK(square == std::array{ 4, 9 });
    }
    SECTION("type-seq-transform")
    {
        constexpr auto moreNumbers = std::tuple{ 4.0, 9 };
        constexpr auto sum = mk::type_sequence_transform2(
            [](auto x, auto y) { return x + y; },
            numbers, moreNumbers);
        CHECK(std::is_same<std::remove_const_t<decltype(sum)>, mk::type_sequence2<double, unsigned>>::value);
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
    SECTION("array")
    {
        auto moreNumbers = std::array{ 6, 8 };
        auto allNumbers = mk::array_cat<int>(numbers, moreNumbers);
        CHECK(allNumbers == std::array{ 2, 3, 6, 8 });
    }
}
