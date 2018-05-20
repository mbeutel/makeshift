
#include <makeshift/types.hpp>
#include <makeshift/metadata.hpp>
#include <makeshift/serialize.hpp>

#include <sstream>
#include <string>

#include <catch.hpp>

namespace mk = makeshift;

enum class MyEnum
{
    foo,
    bar
};
static constexpr auto reflect(MyEnum*, mk::tag<>)
{
    return mk::type<MyEnum>("MyEnum",
        mk::value<MyEnum::foo>("foo"),
        mk::value<MyEnum::bar>("bar")
    );
}

struct Vegetables : mk::define_flags<Vegetables>
{
    static constexpr flag potato { 1 };
    static constexpr flag tomato { 2 };

    static constexpr flag garlic { 4 };
    static constexpr flag onion { 8 };
    static constexpr flag chili { 16 };

    static constexpr flag bean { 32 };
    static constexpr flag lentil { 64 };

    static constexpr flags legume = bean | lentil;
    static constexpr flags spicy = garlic | chili;
    static constexpr flags nightshade = chili | potato | tomato;
    static constexpr flags something_weird = chili | flag(128);
};
using Ratatouille = Vegetables::flags;
static constexpr auto reflect(Ratatouille*, mk::tag<>)
{
    return mk::type<Ratatouille>("Ratatouille",
        mk::flags,
        mk::description("foo"),
        mk::value<Vegetables::potato>("potato"),
        mk::value<Vegetables::tomato>("tomato"),
        mk::value<Vegetables::garlic>("garlic"),
        mk::value<Vegetables::onion>("onion"),
        mk::value<Vegetables::chili>("chili"),
        mk::value<Vegetables::bean>("bean"),
        mk::value<Vegetables::lentil>("lentil"),
        mk::value<Vegetables::legume>("legume"),
        mk::value<Vegetables::spicy>("spicy"),
        mk::value<Vegetables::nightshade>("nightshade"),
        mk::value<Vegetables::something_weird>("something-weird")
    );
}

TEST_CASE("serialize", "[serialize]")
{
    SECTION("enum")
    {
        CHECK(mk::to_string(MyEnum::foo) == "foo");
        CHECK(mk::to_string(MyEnum::bar) == "bar");
        CHECK(mk::from_string<MyEnum>("bar") == MyEnum::bar);
        CHECK_THROWS(mk::from_string<MyEnum>("baz"));
        std::stringstream sstr;
        sstr << mk::as_string(MyEnum::bar);
        MyEnum val = MyEnum::foo;
        sstr >> mk::as_string(val);
        CHECK(val == MyEnum::bar);
    }
    SECTION("flags-enum")
    {
        CHECK(mk::to_string(Vegetables::tomato) == "tomato");
        CHECK(mk::to_string(Vegetables::tomato | Vegetables::potato) == "potato tomato");
        CHECK(mk::to_string(Vegetables::legume | Vegetables::potato) == "legume potato"); // combined flags
        //CHECK(mk::to_string(Vegetables::spicy | Vegetables::nightshade) == "legume potato"); // combined flags
        // TODO: not quite sure what we actually want here
    }
}

#if 0
struct MyStruct
{
    int foo;
    float bar(void) const { }
    void setBar(float) const { }
    double baz;
};
static constexpr auto reflect(MyStruct*, mk::tag<>)
{
    return mk::type<MyStruct>("MyStruct",
        mk::property<&MyStruct::foo>("foo"),
        mk::property</*&MyStruct::bar,*/ &MyStruct::setBar>("bar"),
        mk::property([](const MyStruct& obj) { return obj.baz; }, [](MyStruct& obj, double val) { obj.baz = val; }, "baz")
    );
}

/*struct S
{
    void f(void) { }
};
template <auto..., typename... Ts>
    auto g(Ts&&... args)
{
    return std::make_tuple(std::forward<Ts>(args)...);
}
auto h(void)
{
    return std::make_tuple(
        g<&S::f, 42>(std::string_view{"foo"}),
        g(42)
    );
}*/
#endif

/*int main(void)
{
    std::cout << mk::as_string(MyEnum::foo) << '\n';
    MyEnum myEnum;
    std::cin >> mk::as_string(myEnum);
}*/
