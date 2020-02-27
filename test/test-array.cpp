
#include <makeshift/array.hpp>

#include <array>
#include <tuple>

#include <gsl-lite/gsl-lite.hpp> // for index

#include <catch2/catch.hpp>


namespace {

namespace mk = ::makeshift;
namespace gsl = ::gsl_lite;


TEST_CASE("array", "[flags]")
{
    constexpr auto numbers = std::make_tuple(2, 3u);

    SECTION("array-transform")
    {
        auto square = mk::array_transform([](auto x) { return int(x*x); }, numbers);
        CHECK(square == std::array<int, 2>{ 4, 9 });

        constexpr auto homogeneousNumbers = std::make_tuple(2, 3);
        auto moreNumbers = std::array<int, 2>{ 6, 8 };
        auto allNumbers = mk::array_cat<int>(homogeneousNumbers, moreNumbers);
        CHECK(allNumbers == std::array<int, 4>{ 2, 3, 6, 8 });

        mk::mdarray<double, 2> intSquares = mk::array_transform<double>(
            [](auto x) { return x*x; },
            std::make_tuple(2.0, 3.0f));
        CHECK(intSquares == std::array<double, 2>{ 4.0, 9.0 });

        mk::mdarray<double, 3> gridCoords = mk::array_transform<3, double>(
            [dx=1.0](gsl::index i) { return i*dx; },
            mk::range_index);
        CHECK(gridCoords == std::array<double, 3>{ 0.0, 1.0, 2.0 });
    }
}


} // anonymous namespace
