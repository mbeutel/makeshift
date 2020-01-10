
#if (!defined(__apple_build_version__) || __apple_build_version__ >= 10010046) && (!defined(__GNUC__) || defined(__clang__) || __GNUC__ >= 7) // GCC <=6 and AppleClang <=10.0.0 don't have <variant>
#include <makeshift/variant.hpp>
#include <makeshift/experimental/variant.hpp>

#include <variant>
#include <type_traits> // for is_same<>

#include <gsl-lite/gsl-lite.hpp>

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


using V1 = std::variant<int, float>;
using V2 = std::variant<char const*, int>;
using V3 = std::variant<char const*, float>;

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
    using VT12 = std::variant<int, char const*>;
    using VTM12 = std::variant<float, char const*, int>;

    auto vt12 = mk::variant_transform(VarTransformer{ }, V1{ 42 }, V2{ "there" });
    auto vtm12 = mk::variant_transform_many(NestedVarTransformer{ }, V1{ 42 }, V2{ "there" });
#if !defined(__INTELLISENSE__)
    static_assert(IsSame<decltype(vt12), VT12>::value);
    static_assert(IsSame<decltype(vtm12), VTM12>::value);
#endif // !defined(__INTELLISENSE__)
}


} // anonymous namespace
#endif // (!defined(__apple_build_version__) || __apple_build_version__ >= 10010046) && (!defined(__GNUC__) || defined(__clang__) || __GNUC__ >= 7)
