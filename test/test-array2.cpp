
#include <array>
#include <tuple>

#include <makeshift/array2.hpp>
#include <makeshift/utility2.hpp> // for index

#include <catch2/catch.hpp>


namespace mk = makeshift;


TEST_CASE("array2", "[flags]")
{
    constexpr auto numbers = std::tuple{ 2, 3u };

    SECTION("array-transform")
    {
        auto square = mk::array_transform([](auto x) { return int(x*x); }, numbers);
        CHECK(square == std::array{ 4, 9 });

        constexpr auto homogeneousNumbers = std::tuple{ 2, 3 };
        auto moreNumbers = std::array{ 6, 8 };
        auto allNumbers = mk::array_cat<int>(homogeneousNumbers, moreNumbers);
        CHECK(allNumbers == std::array{ 2, 3, 6, 8 });

        mk::array<double, 2> intSquares = mk::array_transform<double>(
            [](auto x) { return x*x; },
            std::make_tuple(2.0, 3.0f));
        CHECK(intSquares == std::array{ 4.0, 9.0 });

        mk::array<double, 3> gridCoords = mk::array_transform<double, 3>(
            [dx=1.0](mk::index i) { return i*dx; },
            mk::array_index);
        CHECK(gridCoords == std::array{ 0.0, 1.0, 2.0 });
    }
}
