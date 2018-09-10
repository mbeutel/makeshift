
#include <makeshift/utility.hpp>     // for define_flags<>
#include <makeshift/metadata.hpp>
#include <makeshift/string.hpp>
#include <makeshift/streamable.hpp>

#include <sstream>
#include <string>

#include <catch.hpp>

namespace mk = makeshift;

namespace
{

enum class MyEnum
{
    foo,
    bar
};
static constexpr auto reflect(MyEnum*, mk::any_tag_of<mk::reflection_metadata_tag, mk::serialization_metadata_tag>)
{
    using namespace makeshift::metadata;
    return type<MyEnum>(
        "MyEnum", // optional for serialization (may yield better error messages)
        value<MyEnum::foo>("foo"),
        value<MyEnum::bar>("bar")
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
static constexpr auto reflect(Vegetables*, mk::any_tag_of<mk::reflection_metadata_tag, mk::serialization_metadata_tag>) // note: we reflect on Vegetables which is our own type, not on Ratatouille which is just an alias!
{
    using namespace makeshift::metadata;
    return type<Vegetables>(
        "Vegetables", // not required for serialization
        flags( // not required for serialization
            type<Ratatouille>("Ratatouille") // optional for serialization (may yield better error messages)
        ),
        caption("foo"), // optional for serialization (may yield even better error messages)
        value<Vegetables::potato>("potato"),
        value<Vegetables::tomato>("tomato"),
        value<Vegetables::garlic>("garlic"),
        value<Vegetables::onion>("onion"),
        value<Vegetables::chili>("chili", "chili pepper"),
        value<Vegetables::bean>("bean"),
        value<Vegetables::lentil>("lentil"),
        value<Vegetables::legume>("legume"),
        value<Vegetables::spicy>("spicy"),
        value<Vegetables::nightshade>("nightshade"),
        value<Vegetables::something_weird>("something-weird")
    );
}

} // anonymous namespace


TEST_CASE("serialize", "[serialize]")
{
    SECTION("enum")
    {
        CHECK(mk::to_string(MyEnum::foo) == "foo");
        CHECK(mk::to_string(MyEnum::bar) == "bar");
        CHECK(mk::from_string(mk::tag_v<MyEnum>, "bar") == MyEnum::bar);
        CHECK_THROWS(mk::from_string(mk::tag_v<MyEnum>, "baz"));
        std::stringstream sstr;
        sstr << mk::streamable(MyEnum::bar);
        MyEnum val = MyEnum::foo;
        sstr >> mk::streamable(val);
        CHECK(val == MyEnum::bar);
    }
    SECTION("flags-enum")
    {
        CHECK(mk::to_string(Vegetables::tomato) == "tomato");
        CHECK(mk::to_string(Vegetables::tomato | Vegetables::potato) == "potato, tomato");
        CHECK(mk::to_string(Vegetables::legume | Vegetables::potato) == "legume, potato"); // combined flags
        //CHECK(mk::to_string(Vegetables::spicy | Vegetables::nightshade) == "legume, potato"); // combined flags
        // TODO: not quite sure what we actually want here
    }
}
