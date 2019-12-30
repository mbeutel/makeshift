
#include <makeshift/tuple.hpp>

#include <tuple>

#include <gsl-lite/gsl-lite.hpp> // for index

#include <catch2/catch.hpp>


namespace {


namespace mk = ::makeshift;
namespace gsl = ::gsl_lite;


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
            constexpr gsl::index index = indexC();
            result = lhs + rhs + int(index*offset);
        }, result_tuple, lhs_tuple, mk::tuple_index);
        CHECK(result_tuple == std::make_tuple(11, 121, 231));
    }
    SECTION("reduce")
    {
        int sum = mk::template_reduce(0, std::plus<int>{ }, numbers);
        CHECK(sum == 5);
    }
    SECTION("all/any/none")
    {
        bool allGreaterThan0 = mk::template_all_of([](auto x) { return x > 0; }, numbers);
        CHECK(allGreaterThan0);
        bool allGreaterThan2 = mk::template_all_of([](auto x) { return x > 2; }, numbers);
        CHECK_FALSE(allGreaterThan2);
        bool anyGreaterThan2 = mk::template_any_of([](auto x) { return x > 2; }, numbers);
        CHECK(anyGreaterThan2);
        bool anyGreaterThan4 = mk::template_any_of([](auto x) { return x > 4; }, numbers);
        CHECK_FALSE(anyGreaterThan4);
        bool noneGreaterThan2 = mk::template_none_of([](auto x) { return x > 2; }, numbers);
        CHECK_FALSE(noneGreaterThan2);
        bool noneGreaterThan4 = mk::template_none_of([](auto x) { return x > 4; }, numbers);
        CHECK(noneGreaterThan4);
    }
}


} // anonymous namespace
