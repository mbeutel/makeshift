#include <utility>
#include <iostream>

#include <makeshift/types.hpp>

namespace mk = makeshift;

template <typename... Fs>
    struct overload : Fs...
{
    constexpr overload(Fs&&... fs)
        : Fs(std::move(fs))...
    {
    }
    using Fs::operator()...;
};

int main(void)
{
    auto f = mk::overload(
        [](int i) { return 0; },
        [](float f) { return 1; },
        [](const char* s) { return 2; },
        mk::otherwise([](auto a) { return -1; })
    );
    std::cout << f(42) << '\n';
    std::cout << f(13.37f) << '\n';
    std::cout << f("hi") << '\n';
    //std::cout << f(13.37) << '\n';
    //std::cout << f(42u) << '\n';
    std::cout << f((int*) nullptr) << '\n';
}

#if 0
#include <tuple>
#include <utility>
#include <string_view>

struct S
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
}
int main(void)
{
    h();
}
#endif