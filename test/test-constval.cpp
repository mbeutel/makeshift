﻿
#include <makeshift/constval.hpp>

#include <array>
#include <tuple>
#include <type_traits> // for integral_constant<>, is_same<>
#include <functional>  // for plus<>

#include <gsl-lite/gsl-lite.hpp> // for type_identity<>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

#include <makeshift/array.hpp>   // for array<>
#include <makeshift/utility.hpp>


#if defined(__NVCC__) || gsl_BETWEEN(gsl_COMPILER_GNUC_VERSION, 1, 700) || gsl_BETWEEN(gsl_COMPILER_APPLECLANG_VERSION, 1, 900) || defined(__EDG__)
// These compilers wrongly attempt to deduce a dependent type argument in some cases.
# define ERRONEOUS_DEPENDENT_TYPE_DEDUCTION
#endif // defined(__NVCC__) || gsl_BETWEEN(gsl_COMPILER_GNUC_VERSION, 1, 700) || gsl_BETWEEN(gsl_COMPILER_APPLECLANG_VERSION, 1, 900) || defined(__EDG__)

#if defined(__NVCC__)
// These compilers cannot handle reference-type template arguments correctly in some cases.
# define ERRONEOUS_REF_TEMPLATE_ARGS
#endif // defined(__NVCC__)


namespace {

namespace mk = ::makeshift;
namespace gsl = ::gsl_lite;


template <typename... Ts>
void discard_args(Ts...)
{
}

template <typename T, T V>
void expect_constval_normalization(std::integral_constant<T, V>)
{
}

template <typename T, gsl::type_identity_t<T>... Vs>
void expect_array_constval_normalization(mk::array_constant<T, Vs...>)
{
}

template <typename T, gsl::type_identity_t<T>... Vs>
void expect_nested_array_constval_normalization(mk::array_constant<T, Vs...>)
{
    (expect_array_constval_normalization(mk::c<T, Vs>), ...);
}

template <typename... Cs>
void expect_tuple_constval_normalization(mk::tuple_constant<Cs...>)
{
}

template <typename... Cs>
void expect_array_tuple_constval_normalization(mk::tuple_constant<Cs...>)
{
    (expect_array_constval_normalization(Cs{ }), ...);
}

template <typename T>
void expect_type_tag(gsl::type_identity<T>)
{
}

template <typename... Ts>
void expect_type_sequence_tag(mk::type_sequence<Ts...>)
{
}

template <typename C>
void expect_tuple_like(C c)
{
    discard_args(c);

    if constexpr (std::tuple_size_v<C> > 0)
    {
        using std::get;
        std::tuple_element_t<0, C> c0 = get<0>(c);
        discard_args(c0);
    }
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
    discard_args(ncA1);

#ifndef ERRONEOUS_REF_TEMPLATE_ARGS
    auto cAA1 = MAKESHIFT_CONSTVAL(std::array<std::array<int, 1>, 2>{ std::array<int, 1>{ 4 }, std::array<int, 1>{ 2 } });
# ifndef ERRONEOUS_DEPENDENT_TYPE_DEDUCTION
    expect_nested_array_constval_normalization(cAA1);
# endif // ERRONEOUS_DEPENDENT_TYPE_DEDUCTION
    mk::mdarray<int, 2, 1> ncAA1 = cAA1;
    discard_args(ncAA1);
#endif // ERRONEOUS_REF_TEMPLATE_ARGS

    auto cTA1 = MAKESHIFT_CONSTVAL(std::make_tuple(std::array<int, 1>{ 3 }, std::array<int, 2>{ 1, 4 }));
    expect_array_tuple_constval_normalization(cTA1);
    std::tuple<std::array<int, 1>, std::array<int, 2>> ncTA1 = cTA1;
    discard_args(ncTA1);

    auto cT1 = MAKESHIFT_CONSTVAL(mk::type_identity_c<int>);
    expect_type_tag<int>(cT1);

    auto cT3 = mk::type_identity_c<float>;
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
#if !gsl_COMPILER_NVCC_VERSION  // NVCC somehow screws up here.
    auto cCTA = mk::constval_transform(ToArrayTransform{ }, cCT);
    expect_tuple_like(cCTA);
# ifndef ERRONEOUS_DEPENDENT_TYPE_DEDUCTION
    expect_array_constval_normalization(cCTA);
# endif // ERRONEOUS_DEPENDENT_TYPE_DEDUCTION
#endif // !gsl_COMPILER_NVCC_VERSION

    auto cCTV = mk::constval_transform(ToTupleTransform{ }, cCT);
    expect_tuple_like(cCTV);
    expect_tuple_constval_normalization(cCTV);

    auto cCT1 = mk::c<CustomType const&, SomeClass::ct>;
    static constexpr CustomType c2 = cCT1();
    discard_args(c2);
    auto cA3 = mk::c<std::array<int, 2> const&, SomeClass::ca>;
    expect_array_constval_normalization(cA3);
    auto cCT2 = mk::ref_c<SomeClass::ct>;
    static constexpr CustomType c3 = cCT2();
    discard_args(c3);
    auto cA4 = mk::ref_c<SomeClass::ca>;
    expect_array_constval_normalization(cA4);

    //auto iCT = mk::ref_c<SomeClass::ct.i>; // this doesn't work because arg doesn't have linkage
    //auto cCT3 = mk::ref_c<c2>; // this doesn't work either, for the same reason
    auto iCT = MAKESHIFT_CONSTVAL(SomeClass::ct.i);
    expect_constval_normalization<int>(iCT);
}

TEST_CASE("constval as comonad")
{
    auto aC = MAKESHIFT_CONSTVAL(2);

        // extract
    int a = aC();
    CHECK(a == 2);

# if !gsl_BETWEEN(gsl_COMPILER_MSVC_VERSION, 1, 143)  // ICE in VC++ 2019
        // extend
    static constexpr auto lookupTable = std::tuple{ "hi", 42, 3.14f };
    auto f = [](auto xC) { return std::get<xC()>(lookupTable); };
    auto b = f(aC);
    CHECK(b == 3.14f);
    //auto bC = MAKESHIFT_CONSTVAL(f(aC));  // doesn't work due to capture
    auto bC = makeshift::constval_extend(  // works in C++20
        [](auto f, auto xC)
        {
            return f(xC);
        },
        f, aC);
    CHECK(bC() == b);
    auto bC2 = makeshift::constval_extend(f, aC);  // works in C++20
    CHECK(bC2() == b);
# endif  // !gsl_BETWEEN(gsl_COMPILER_MSVC_VERSION, 1, 143)
}

constexpr std::vector<int>
computeFibonacciSeqUpTo(int max)
{
    gsl_Expects(max >= 1);

    int c0 = 1;
    int c1 = 1;
    auto result = std::vector<int>{ c0, c1 };
    for (;;)
    {
        int c2 = c0 + c1;
        if (c2 > max) return result;
        result.push_back(c2);
        c0 = c1;
        c1 = c2;
    }
}

TEST_CASE("constval_range_to_array")
{
    CHECK_THAT(computeFibonacciSeqUpTo(50), Catch::Matchers::RangeEquals(std::array{ 1, 1, 2, 3, 5, 8, 13, 21, 34 }));

    constexpr std::array fibonacciSeqUpTo50 = makeshift::constval_range_to_array([] { return computeFibonacciSeqUpTo(50); });
    CHECK(fibonacciSeqUpTo50 == std::array{ 1, 1, 2, 3, 5, 8, 13, 21, 34 });
}


} // anonymous namespace
