
#include <type_traits> // for integral_constant<>

#include <makeshift/buffer.hpp>

#include <catch2/catch.hpp>


namespace mk = makeshift;


TEST_CASE("buffer")
{
    auto c1 = std::integral_constant<int, 1>{ };
    auto c5 = std::integral_constant<int, 5>{ };

    auto buf1 = mk::make_buffer<int>(c1);
    buf1 = { 1 };
    auto buf5 = mk::make_buffer<int, 1>(c5);
    buf5 = { 1, 4, 1, 4, 2 };
    auto buf3 = mk::make_buffer<int, 4>(3);
    buf3 = { 1, 4, 1 };
    auto buf7 = mk::make_buffer<int, 4>(7);
    buf7 = { 1, 4, 1, 4, 2, 1, 3 };
}
