
#include <makeshift/range.hpp>
#include <makeshift/array.hpp>

#include <array>
#include <tuple>

#include <makeshift/stdint.hpp> // for index

#include <catch2/catch.hpp>


namespace mk = makeshift;




template <int> struct True : std::true_type { };
template <int> struct False : std::false_type { };

template <template <typename...> class TraitT, typename... Ts>
struct Check
{
    constexpr operator bool(void) const noexcept
    {
        static_assert(TraitT<Ts...>::value, "static assertion failed");
        return true;
    }
};

static_assert(Check<std::is_base_of, std::false_type, mk::disjunction<>>{ }, "static assertion failed");
static_assert(Check<std::is_base_of, True<0>, mk::disjunction<True<0>>>{ }, "static assertion failed");
static_assert(Check<std::is_base_of, False<0>, mk::disjunction<False<0>>>{ }, "static assertion failed");
static_assert(Check<std::is_base_of, True<0>, mk::disjunction<True<0>, True<1>>>{ }, "static assertion failed");
static_assert(Check<std::is_base_of, True<0>, mk::disjunction<True<0>, True<1>, True<2>>>{ }, "static assertion failed");
static_assert(Check<std::is_base_of, True<1>, mk::disjunction<False<0>, True<1>, True<2>>>{ }, "static assertion failed");
static_assert(Check<std::is_base_of, False<2>, mk::disjunction<False<0>, False<1>, False<2>>>{ }, "static assertion failed");
static_assert(Check<std::is_base_of, std::true_type, mk::conjunction<>>{ }, "static assertion failed");
static_assert(Check<std::is_base_of, True<0>, mk::conjunction<True<0>>>{ }, "static assertion failed");
static_assert(Check<std::is_base_of, False<0>, mk::conjunction<False<0>>>{ }, "static assertion failed");
static_assert(Check<std::is_base_of, True<1>, mk::conjunction<True<0>, True<1>>>{ }, "static assertion failed");
static_assert(Check<std::is_base_of, True<2>, mk::conjunction<True<0>, True<1>, True<2>>>{ }, "static assertion failed");
static_assert(Check<std::is_base_of, False<1>, mk::conjunction<True<0>, False<1>, False<2>>>{ }, "static assertion failed");
static_assert(Check<std::is_base_of, False<0>, mk::conjunction<False<0>, False<1>, False<2>>>{ }, "static assertion failed");


TEST_CASE("array2", "[flags]")
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

        mk::mdarray<double, 3> gridCoords = mk::array_transform<double, 3>(
            [dx=1.0](mk::index i) { return i*dx; },
            mk::range_index);
        CHECK(gridCoords == std::array<double, 3>{ 0.0, 1.0, 2.0 });
    }
}
