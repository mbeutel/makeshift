
#include <makeshift/types.hpp>
#include <makeshift/metadata.hpp>
#include <makeshift/serialize.hpp>

#include <iostream>


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

#if 0
struct MySetValues : mk::define_flags<MySetValues>
{
    static constexpr flag foo { 1 };
    static constexpr flag bar { 2 };
};
using MySet = MySetValues::flags;
static constexpr auto reflect(MySet*, mk::tag<>)
{
    return mk::type<MySet>("MySet",
        mk::flags,
        mk::description("this is a bitset"),
        mk::value<MySetValues::foo>("foo"),
        mk::value<MySetValues::bar>("bar")
    );
}

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

int main(void)
{
    std::cout << mk::as_string(MyEnum::foo) << '\n';
    MyEnum myEnum;
    std::cin >> mk::as_string(myEnum);
}
