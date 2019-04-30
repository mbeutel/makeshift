
#include <tuple>
#include <type_traits> // for integral_constant<>, is_same<>
#include <functional>  // for plus<>

#include <makeshift/array2.hpp>   // for array<>
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

template <typename T>
    void expect_type_tag(mk::type<T>)
{
}

template <typename... Ts>
    void expect_type_sequence_tag(mk::type_sequence<Ts...>)
{
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
    expect_array_constval_normalization<int, 4, 2>(cA);

    auto cAA = mk::make_constval([]{ return std::array{ std::array{ 4 }, std::array{ 2 } }; });
    expect_nested_array_constval_normalization<mk::array<int[1]>, mk::array_constant<int, 4>, mk::array_constant<int, 2>>(cAA);

    auto cTA = mk::make_constval([]{ return std::tuple{ std::array{ 3 }, std::array{ 1, 4 } }; });
    expect_array_tuple_constval_normalization<mk::array_constant<int, 3>, mk::array_constant<int, 1, 4>>(cTA);

    auto cT1 = mk::make_constval([]{ return mk::type_v<int>; });
    expect_type_tag<int>(cT1);

    auto cT2 = mk::type_v<float>;
    expect_type_tag<float>(cT2);

    auto cTS1 = mk::make_constval([]{ return mk::type_sequence_v<int, float>; });
    expect_type_sequence_tag<int, float>(cTS1);

    auto cTS2 = mk::type_sequence_v<float, int>;
    expect_type_sequence_tag<float, int>(cTS2);
}
