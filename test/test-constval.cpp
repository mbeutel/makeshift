
#include <tuple>
#include <type_traits> // for integral_constant<>
#include <functional>  // for plus<>

#include <makeshift/constval.hpp>

#include <catch2/catch.hpp>


namespace mk = makeshift;


constexpr std::tuple<int, int> getTuple(void)
{
    return { 4, 2 };
}

static auto getTupleElement = [](auto indexR)
{
    return std::get<indexR()>(getTuple());
};


TEST_CASE("constexpr")
{
    auto c1 = std::integral_constant<int, 1>{ };
    auto c5 = mk::make_constval([]{ return 5; });

    auto c6R = mk::constval_transform(std::plus<>{ }, c1, c5);
    constexpr int c6 = c6R();
    static_assert(c6 == 6);

    auto c2R = mk::constval_extend(getTupleElement, c1);
    constexpr int c2 = c2R();
    static_assert(c2 == 2);
}
