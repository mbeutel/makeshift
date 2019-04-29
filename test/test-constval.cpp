
#include <tuple>
#include <type_traits> // for integral_constant<>, is_same<>
#include <functional>  // for plus<>

#include <makeshift/utility2.hpp>
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

template <typename T, T V>
    void expect_constval_normalization(std::integral_constant<T, V>)
{
}

template <typename T, T... Vs>
    void expect_array_constval_normalization(mk::array_constant<T, Vs...>)
{
}

template <typename T, typename... Vs>
    void expect_nested_array_constval_normalization(mk::constval_array<T, Vs...>)
{
    (expect_array_constval_normalization(Vs{ }), ...);
}

template <typename... Vs>
    void expect_array_tuple_constval_normalization(mk::constval_tuple<Vs...>)
{
    (expect_array_constval_normalization(Vs{ }), ...);
}


TEST_CASE("constexpr")
{
    auto c1 = std::integral_constant<int, 1>{ };
    auto c5 = mk::make_constval([]{ return 5; });
    expect_constval_normalization(c5);
    auto c42 = 42;

    /*auto c6R = mk::constval_transform(std::plus<>{ }, c1, c5);
    constexpr int c6 = c6R();
    static_assert(c6 == 6);

    auto c2R = mk::constval_extend(getTupleElement, c1);
    constexpr int c2 = c2R();
    static_assert(c2 == 2);*/

    auto c42R = mk::constval_transform(std::plus<>{ }, c1, c42);
    static_assert(std::is_same<decltype(c42R), int>::value, "wrong type");
    CHECK(c42R == 43);

    auto cA = mk::make_constval([]{ return std::array{ 4, 2 }; });
    expect_array_constval_normalization(cA);

    auto cAA = mk::make_constval([]{ return std::array{ std::array{ 4 }, std::array{ 2 } }; });
    expect_nested_array_constval_normalization(cAA);

    auto cTA = mk::make_constval([]{ return std::tuple{ std::array{ 3 }, std::array{ 1, 4 } }; });
    expect_array_tuple_constval_normalization(cTA);
}
