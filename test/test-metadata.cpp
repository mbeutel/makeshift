
#include <makeshift/utility.hpp>     // for define_flags<>
#include <makeshift/metadata.hpp>
#include <makeshift/string.hpp>
#include <makeshift/streamable.hpp>
#include <makeshift/reflect.hpp>

#include <makeshift/serializers/string.hpp>
#include <makeshift/serializers/stream.hpp>
#include <makeshift/serializers/hint.hpp>

#include <sstream>
#include <string>

#include <catch2/catch.hpp>

namespace mk = makeshift;

namespace
{

enum class MyEnum
{
    foo,
    bar
};
constexpr inline auto reflect(mk::tag<MyEnum>, mk::any_tag_of<mk::reflection_tag, mk::serialization_tag>)
{
    using namespace makeshift::metadata;
    return type1<MyEnum>(
        "MyEnum", // optional for serialization (may yield better error messages)
        value1<MyEnum::foo>("foo"),
        value1<MyEnum::bar>("bar")
    );
}

struct Vegetables : mk::define_flags<Vegetables>
{
    static constexpr auto potato = flag(1);
    static constexpr auto tomato = flag(2);

    static constexpr auto garlic = flag(4);
    static constexpr auto onion = flag(8);
    static constexpr auto chili = flag(16);

    static constexpr auto bean = flag(32);
    static constexpr auto lentil = flag(64);

    static constexpr flags legume = bean | lentil;
    static constexpr flags spicy = garlic | chili;
    static constexpr flags nightshade = chili | potato | tomato;
    static constexpr flags something_weird = chili | flag(128);
};
using Ratatouille = Vegetables::flags;
constexpr inline auto reflect(mk::tag<Vegetables>, mk::any_tag_of<mk::reflection_tag, mk::serialization_tag>) // note: we reflect on Vegetables which is our own type, not on Ratatouille which is just an alias!
{
    using namespace makeshift::metadata;
    return type1<Vegetables>(
        "Vegetables", // not required for serialization
        flags( // not required for serialization
            type1<Ratatouille>("Ratatouille") // optional for serialization (may yield better error messages)
        ),
        caption("foo"), // optional for serialization (may yield even better error messages)
        value1<Vegetables::potato>("potato"),
        value1<Vegetables::tomato>("tomato"),
        value1<Vegetables::garlic>("garlic"),
        value1<Vegetables::onion>("onion"),
        value1<Vegetables::chili>("chili", "chili pepper"),
        value1<Vegetables::bean>("bean"),
        value1<Vegetables::lentil>("lentil"),
        value1<Vegetables::legume>("legume"),
        value1<Vegetables::spicy>("spicy"),
        value1<Vegetables::nightshade>("nightshade"),
        value1<Vegetables::something_weird>("something-weird")
    );
}

struct COOIndex
{
    int i;
    int j;
};
constexpr inline auto reflect(mk::tag<COOIndex>, mk::any_tag_of<mk::reflection_tag, mk::serialization_tag>)
{
    using namespace makeshift::metadata;
    return type1<COOIndex>(
        constant<type_flag::value | type_flag::compound>{ },
        member1<&COOIndex::i>("i"),
        member1<&COOIndex::j>("j")
    );
}

struct COOValue
{
    COOIndex index;
    double value;
};
constexpr inline auto reflect(mk::tag<COOValue>, mk::any_tag_of<mk::reflection_tag, mk::serialization_tag>)
{
    using namespace makeshift::metadata;
    return type1<COOValue>(
        constant<type_flag::compound>{ },
        member1<&COOValue::index>("index"),
        member1<&COOValue::value>("the value")
    );
}


} // anonymous namespace


TEST_CASE("reflect", "[reflect]")
{
    SECTION("enum")
    {
        // TODO
    }
    SECTION("constrained integer")
    {
        // TODO
    }
    SECTION("struct")
    {
        COOIndex i1 { 1, 2 };
        COOIndex i2 { 2, 1 };
        COOIndex i3 { 2, 2 };
        
        CHECK(mk::less<>{ }(i1, i2));
        CHECK(mk::less<>{ }(i2, i3));
        CHECK_FALSE(mk::less<>{ }(i2, i1));
        CHECK_FALSE(mk::less<>{ }(i3, i2));
        CHECK_FALSE(mk::less<>{ }(i1, i1));

        CHECK(mk::equal_to<>{ }(i1, i1));
        CHECK(mk::equal_to<>{ }(i2, i2));
        CHECK(mk::equal_to<>{ }(i3, i3));
        CHECK_FALSE(mk::equal_to<>{ }(i1, i2));
        CHECK_FALSE(mk::equal_to<>{ }(i2, i3));
        CHECK_FALSE(mk::equal_to<>{ }(i1, i3));
        
        auto i2hash = mk::hash<>{ }(i2);
        auto i3hash = mk::hash<>{ }(i3);
        CHECK(i2hash != i3hash); // this is not strictly necessary, but it typically holds unless std::hash<int> is terrible
        
        COOValue v1a { i1, 0.0 };
        COOValue v1b { i1, 1.0 };
        COOValue v2 { i2, 42.0 };

        CHECK(mk::less<>{ }(v1a, v1b));
        CHECK(mk::less<>{ }(v1a, v2));
        CHECK_FALSE(mk::less<>{ }(v1a, v1a));
        CHECK_FALSE(mk::less<>{ }(v2, v1a));

        CHECK(mk::equal_to<>{ }(v1a, v1a));
        CHECK_FALSE(mk::equal_to<>{ }(v1a, v1b));
        CHECK_FALSE(mk::equal_to<>{ }(v1a, v2));

        auto v1ahash = mk::hash<>{ }(v1a);
        auto v1bhash = mk::hash<>{ }(v1b);
        CHECK(v1ahash != v1bhash); // this is not strictly necessary, but it typically holds unless std::hash<double> is terrible
    }
}


TEST_CASE("serialize", "[serialize]")
{
    SECTION("enum")
    {
        CHECK(mk::get_hint<MyEnum>() == "foo|bar");

        CHECK(mk::to_string(MyEnum::foo) == "foo");
        CHECK(mk::to_string(MyEnum::bar) == "bar");
        CHECK(mk::from_string<MyEnum>("bar") == MyEnum::bar);
        CHECK_THROWS(mk::from_string<MyEnum>("baz"));
        std::stringstream sstr;
        sstr << mk::streamable(MyEnum::bar);
        MyEnum val = MyEnum::foo;
        sstr >> mk::streamable(val);
        CHECK(val == MyEnum::bar);
    }
    SECTION("flags-enum")
    {
        CHECK(mk::get_hint<Ratatouille>() == "potato,tomato,garlic,onion,chili,chili pepper,bean,lentil,legume,spicy,nightshade,something-weird");

        CHECK(mk::to_string(Vegetables::tomato) == "tomato");
        CHECK(mk::to_string(Vegetables::tomato | Vegetables::potato) == "potato, tomato");
        CHECK(mk::to_string(Vegetables::legume | Vegetables::potato) == "legume, potato"); // combined flags
        //CHECK(mk::to_string(Vegetables::spicy | Vegetables::nightshade) == "legume, potato"); // combined flags
        // TODO: not quite sure what we actually want here
    }
    SECTION("struct")
    {
        CHECK(mk::get_hint<COOIndex>() == "(i,j)");
        CHECK(mk::get_hint<COOValue>() == "{index,\"the value\"}");

        COOIndex i2 { 2, 1 };
        COOValue v2 { i2, 42.0 };

        {
            std::stringstream sstr;
            sstr << mk::streamable(i2);
            CHECK(sstr.str() == "(2, 1)");
            COOIndex i2r { };
            sstr >> mk::streamable(i2r);
            CHECK(mk::equal_to{ }(i2, i2r));
            //CHECK(mk::equal_to<>{ }(i2, i2r));
        }

        {
            std::stringstream sstrExpected, sstrActual;
            sstrExpected << "{ .index=(2, 1), \"the value\"=" << v2.value << " }";
            sstrActual << mk::streamable(v2);
            CHECK(sstrActual.str() == sstrExpected.str());
            COOValue v2r { };
            sstrActual >> mk::streamable(v2r);
            CHECK(mk::equal_to{ }(v2, v2r));
        }

        {
            std::stringstream sstr;
            sstr.str("{ \"the value\"=42.0, .index=(2, 1) }");
            COOValue v2r{ };
            sstr >> mk::streamable(v2r);
            CHECK(mk::equal_to{ }(v2, v2r));
        }

        {
            std::stringstream sstr;
            sstr.str("{ (2, 1), \"the value\"=42.0 }");
            COOValue v2r{ };
            sstr >> mk::streamable(v2r);
            CHECK(mk::equal_to{ }(v2, v2r));
        }

        {
            std::stringstream sstr;
            sstr.str("{ \"the value\"=42.0, (2, 1) }");
            COOValue v2r{ };
            CHECK_THROWS_AS(sstr >> mk::streamable(v2r), mk::parse_error);
        }

        {
            std::stringstream sstr;
            sstr.str("{ .index=(2, 1), 42.0 }");
            COOValue v2r{ };
            sstr >> mk::streamable(v2r);
            CHECK(mk::equal_to{ }(v2, v2r));
        }

        {
            std::stringstream sstr;
            sstr.str("{ .index=(2, 1) }");
            COOValue v{ { 2, 1 }, 42.0 };
            COOValue vr{ { }, 42.0 };
            sstr >> mk::streamable(vr);
            CHECK(mk::equal_to{ }(v, vr));
        }
    }
}
