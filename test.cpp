
#include "include/makeshift/types.hpp"



using namespace makeshift::literals;
namespace mk = makeshift;

struct Vegetable : mk::define_flags<Vegetable>
{
    static constexpr flag tomato { 1 };
    static constexpr flag onion { 2 };
    static constexpr flag eggplant { 4 };
    static constexpr flag garlic { 8 };
};
using Vegetables = Vegetable::flags;

static constexpr auto foo = Vegetables::none;
static constexpr auto bar = Vegetable::tomato | Vegetable::onion;
static constexpr bool anyOf = hasAnyOf(foo, bar);

using NamedInt = mk::named<int, "width"_kn>;
NamedInt val1 { 42 };
NamedInt val2 = { mk::name<"width"_kn> = 42 };


int main(void)
{

}
