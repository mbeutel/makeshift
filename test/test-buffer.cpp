
#include <type_traits> // for integral_constant<>

#include <makeshift/buffer.hpp>
#include <makeshift/constval.hpp> // for make_constval()

#include <catch2/catch.hpp>


namespace mk = makeshift;


TEST_CASE("buffer")
{
    auto c1 = std::integral_constant<int, 1>{ };
    auto c5 = mk::make_constval([]{ return 5; });

    auto buf1 = mk::make_buffer<int>(c1);
    auto buf5 = mk::make_buffer<int, 1>(c5);
}
