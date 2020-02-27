
#include <makeshift/mpark/variant.hpp>
#include <makeshift/experimental/mpark/variant.hpp>

#include <type_traits> // for is_same<>

#include <gsl-lite/gsl-lite.hpp>

#include <mpark/variant.hpp>

#include <catch2/catch.hpp>


namespace {


namespace mk = ::makeshift;
namespace gsl = ::gsl_lite;


template <typename A, typename B>
struct IsSame
{
    static constexpr bool value = std::is_same<A, B>::value;
    static_assert(value, "static assertion failed");
};


using V1 = mpark::variant<int, float>;
using V2 = mpark::variant<char const*, int>;
using V3 = mpark::variant<char const*, float>;

struct VarTransformer
{
    char const* operator ()(float, int) const { return "hi"; }
    int operator ()(float f, char const*) const { return int(f); }
    int operator ()(int i, int) const { return i; }
    int operator ()(int i, char const*) const { return i; }
};
struct NestedVarTransformer
{
    V2 operator ()(float, int) const { return "hi"; }
    V3 operator ()(float f, char const*) const { return f; }
    V1 operator ()(int i, int) const { return i; }
    V1 operator ()(int i, char const*) const { return i; }
};

TEST_CASE("variant")
{
    using VT12 = mpark::variant<int, char const*>;
    using VTM12 = mpark::variant<float, char const*, int>;

    auto vt12 = mk::mpark::variant_transform(VarTransformer{ }, V1{ 42 }, V2{ "there" });
    static_assert(IsSame<decltype(vt12), VT12>::value, "static assertion failed");
    auto vtm12 = mk::mpark::variant_transform_many(NestedVarTransformer{ }, V1{ 42 }, V2{ "there" });
    static_assert(IsSame<decltype(vtm12), VTM12>::value, "static assertion failed");
}


} // anonymous namespace
