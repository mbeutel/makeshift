
#include <array>
#include <tuple>
#include <type_traits> // for integral_constant<>, is_same<>
#include <functional>  // for plus<>

#include <makeshift/array.hpp>   // for array<>
#include <makeshift/utility.hpp>
#include <makeshift/constval.hpp>

#include <catch2/catch.hpp>


namespace mk = makeshift;


constexpr std::tuple<int, int> getTuple(void)
{
    return { 4, 2 };
}

/*static auto getTupleElement = [](auto indexR)
{
    return std::get<indexR()>(getTuple());
};*/

template <typename... Ts>
    void discard_args(Ts...)
{
}

template <typename T, T V>
    void expect_constval_normalization(std::integral_constant<T, V>)
{
}

template <typename T, mk::as_dependent_type<T>... Vs>
    void expect_array_constval_normalization(mk::array_constant<T, Vs...>)
{
}

template <typename T, mk::as_dependent_type<T>... Vs>
    void expect_nested_array_constval_normalization(mk::array_constant<T, Vs...>)
{
#if MAKESHIFT_CXX >= 17
    (expect_array_constval_normalization(mk::c<T, Vs>), ...);
#else // MAKESHIFT_CXX >= 17
    discard_args((expect_array_constval_normalization(mk::c<T, Vs>), 1)...);
#endif // MAKESHIFT_CXX >= 17
}

template <typename... Cs>
    void expect_tuple_constval_normalization(mk::tuple_constant<Cs...>)
{
}

template <typename... Cs>
    void expect_array_tuple_constval_normalization(mk::tuple_constant<Cs...>)
{
#if MAKESHIFT_CXX >= 17
    (expect_array_constval_normalization(Cs{ }), ...);
#else // MAKESHIFT_CXX >= 17
    discard_args((expect_array_constval_normalization(Cs{ }), 1)...);
#endif // MAKESHIFT_CXX >= 17
}

template <typename T>
    void expect_type_tag(mk::type<T>)
{
}

template <typename... Ts>
    void expect_type_sequence_tag(mk::type_sequence<Ts...>)
{
}

template <typename C>
    void expect_tuple_like(C c)
{
    (void) c;

#if MAKESHIFT_CXX >= 17
    if constexpr (std::tuple_size_v<C> > 0)
    {
        using std::get;
        std::tuple_element_t<0, C> c0 = get<0>(c);
        (void) c0;
    }
#endif // MAKESHIFT_CXX >= 17
}


struct CustomType
{
    int i;
    float f;
    std::array<int, 2> a;
};

struct SomeClass
{
    static constexpr CustomType ct = { 4, 1.41421f, { 1, 3 } };
    static constexpr std::array<int, 2> ca = { 2, 4 };
};
constexpr CustomType SomeClass::ct;

enum class Color { red, green, blue };


struct ToArrayTransform
{
    template <typename T>
        constexpr auto operator ()(T customTypeObj) const
    {
        return std::array<T, 2>{ customTypeObj, customTypeObj };
    }
};
struct ToTupleTransform
{
    template <typename T>
        constexpr auto operator ()(T customTypeObj) const
    {
        return std::tuple<T, T>{ customTypeObj, customTypeObj };
    }
};

TEST_CASE("constval")
{
    auto c1 = std::integral_constant<int, 1>{ };
    auto c51 = MAKESHIFT_CONSTVAL(5);
    expect_constval_normalization(c51);
    auto c42 = 42;

    auto cClr1 = MAKESHIFT_CONSTVAL(Color::red);
    expect_constval_normalization(cClr1);

    auto c42R = mk::constval_transform(std::plus<>{ }, c1, c42);
    static_assert(std::is_same<decltype(c42R), int>::value, "wrong type");
    CHECK(c42R == 43);

    auto cA1 = MAKESHIFT_CONSTVAL(std::array<int, 2>{ 4, 2 });
    expect_array_constval_normalization<int, 4, 2>(cA1);
    mk::mdarray<int, 2> ncA1 = cA1;
    (void) ncA1;

    auto cAA1 = MAKESHIFT_CONSTVAL(std::array<std::array<int, 1>, 2>{ std::array<int, 1>{ 4 }, std::array<int, 1>{ 2 } });
    expect_nested_array_constval_normalization(cAA1);
    mk::mdarray<int, 2, 1> ncAA1 = cAA1;
    (void) ncAA1;

    auto cTA1 = MAKESHIFT_CONSTVAL(std::make_tuple(std::array<int, 1>{ 3 }, std::array<int, 2>{ 1, 4 }));
    expect_array_tuple_constval_normalization(cTA1);
    std::tuple<std::array<int, 1>, std::array<int, 2>> ncTA1 = cTA1;
    (void) ncTA1;

    auto cT1 = MAKESHIFT_CONSTVAL(mk::type_c<int>);
    expect_type_tag<int>(cT1);

    auto cT3 = mk::type_c<float>;
    expect_type_tag<float>(cT3);

    auto cTS1 = MAKESHIFT_CONSTVAL(mk::type_sequence<int, float>{ });
    expect_type_sequence_tag<int, float>(cTS1);

    auto cTS3 = mk::type_sequence<float, int>{ };
    expect_type_sequence_tag<float, int>(cTS3);

    auto cCT = MAKESHIFT_CONSTVAL(
        CustomType{
            42,
            13.37f,
            { 4, 2 }
        });
    auto cCTA = mk::constval_transform(ToArrayTransform{ }, cCT);
    expect_tuple_like(cCTA);
    expect_array_constval_normalization(cCTA);

    auto cCTV = mk::constval_transform(ToTupleTransform{ }, cCT);
    expect_tuple_like(cCTV);
    expect_tuple_constval_normalization(cCTV);

    auto cCT1 = mk::c<CustomType const&, SomeClass::ct>;
    static constexpr CustomType c2 = cCT1();
    (void) c2;
    auto cA3 = mk::c<std::array<int, 2> const&, SomeClass::ca>;
    expect_array_constval_normalization(cA3);
#if MAKESHIFT_CXX >= 17
    auto cCT2 = mk::ref_c<SomeClass::ct>;
    static constexpr CustomType c3 = cCT2();
    (void) c3;
    auto cA4 = mk::ref_c<SomeClass::ca>;
    expect_array_constval_normalization(cA4);
#endif // MAKESHIFT_CXX >= 17

#if MAKESHIFT_CXX >= 17
    //auto iCT = mk::ref_c<SomeClass::ct.i>; // this doesn't work because arg doesn't have linkage
    //auto cCT3 = mk::ref_c<c2>; // this doesn't work either, for the same reason
    auto iCT = MAKESHIFT_CONSTVAL(SomeClass::ct.i);
    expect_constval_normalization<int>(iCT);
#endif // MAKESHIFT_CXX >= 17
}
