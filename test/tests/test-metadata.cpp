
#include <makeshift/type_traits.hpp>   // for metadata_tag
#include <makeshift/utility.hpp>   // for define_flags<>
#include <makeshift/metadata.hpp>
#include <makeshift/serialize.hpp>
#include <makeshift/property.hpp>

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
static constexpr auto reflect(MyEnum*, mk::default_metadata_tag)
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
static constexpr auto reflect(Vegetables*, mk::default_metadata_tag) // note: we reflect on Vegetables which is our own type, not on Ratatouille which is just an alias!
{
    using namespace makeshift::metadata;
    return type<Vegetables>(
        "Vegetables", // not required for serialization
        flags( // not required for serialization
            type<Ratatouille>("Ratatouille") // optional for serialization (may yield better error messages)
        ),
        description("foo"), // optional for serialization (may yield even better error messages)
        value<Vegetables::potato>("potato"),
        value<Vegetables::tomato>("tomato"),
        value<Vegetables::garlic>("garlic"),
        value<Vegetables::onion>("onion"),
        value<Vegetables::chili>("chili"),
        value<Vegetables::bean>("bean"),
        value<Vegetables::lentil>("lentil"),
        value<Vegetables::legume>("legume"),
        value<Vegetables::spicy>("spicy"),
        value<Vegetables::nightshade>("nightshade"),
        value<Vegetables::something_weird>("something-weird")
    );
}

struct MyStruct
{
    int foo;
    float bar(void) const { }
    void setBar(float) const { }
    double baz;
};
static constexpr auto reflect(MyStruct*, mk::default_metadata_tag)
{
    using namespace makeshift::metadata;
    return type<MyStruct>("MyStruct",
        property<&MyStruct::foo>("foo"),
        property<&MyStruct::bar, &MyStruct::setBar>("bar"),
        property([](const MyStruct& obj) { return obj.baz; }, [](MyStruct& obj, double val) { obj.baz = val; }, "baz")
    );
}

template <typename T>
    constexpr std::string_view type_name(void)
{
    return std::get<std::string_view>(mk::metadata_of<T>.attributes);
}

//template <typename T>
//    struct descriptor

struct string_join_t
{
private:
    std::string separator_;

public:
    string_join_t(std::string _separator) noexcept : separator_(std::move(_separator)) { }
    template <typename TupleT,
              typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
        std::string operator ()(TupleT&& tuple) const
    {
        auto reducedValue = tuple
            | mk::tuple_reduce(mk::none,
                  mk::overload(
                      [](mk::none_t lhs, auto&& rhs) { return std::forward<decltype(rhs)>(rhs); },
                      [&](auto&& lhs, auto&& rhs) { return std::string(std::forward<decltype(lhs)>(lhs)) + separator_ + std::string(std::forward<decltype(rhs)>(rhs)); }
                  )
              );
        if constexpr (std::is_same<decltype(reducedValue), mk::none_t>::value)
            return std::string{ };
        else
            return reducedValue;
    }
};
string_join_t string_join(std::string separator)
{
    return { separator };
}

struct serialize_as_table_t
{
    std::string separator = "\t";
};
serialize_as_table_t serialize_as_table(std::string separator = "\t")
{
    return { std::move(separator) };
}

template <typename T> using is_property_metadata = mk::is_same_template<T, mk::property_metadata>;
template <typename T> using is_not_none = std::negation<std::is_same<T, mk::none_t>>;

template <typename T, typename AttributesT>
    std::string to_string(const mk::type_metadata<T, AttributesT>& typeMetadata, serialize_as_table_t)
{
    return typeMetadata.attributes
        | mk::tuple_filter<is_property_metadata>()
        | mk::tuple_map(columns, [&](const auto& propMetadata)
          {
              constexpr auto accessor = mk::get_property_accessor(typeMetadata, propMetadata);
              using Accessor = decltype(accessor);
              if constexpr (mk::has_getter_v<Accessor>)
                  return get<std::string_view>(propMetadata.attributes);
              else
                  return none;
          })
        | mk::tuple_filter<is_not_none>()
        | string_join("\t");
}

template <typename T>
    void to_stream(std::ostream& stream, const T& value, const serialize_as_table_t& sctx)
{
    auto readableProperties = metadata_of<T>.attributes
        | mk::tuple_filter<is_property_metadata>()
        | mk::tuple_map(columns, [&](const auto& propMetadata)
          {
              constexpr auto accessor = mk::get_property_accessor(typeMetadata, propMetadata);
              using Accessor = decltype(accessor);
              if constexpr (mk::has_getter_v<Accessor>)
                  return get<std::string_view>(propMetadata.attributes);
              else
                  return none;
          })
        | mk::tuple_filter<is_not_none>();
    bool haveFirst = false;
    mk::tuple_foreach(readableProperties, [&haveFirst](const auto& value)
    {
        if (!haveFirst)
            haveFirst = true;
        else
            stream << "\t";
        stream << value;
    });
}

} // anonymous namespace


TEST_CASE("serialize", "[serialize]")
{
    SECTION("enum")
    {
        CHECK(mk::to_string(MyEnum::foo) == "foo");
        CHECK(mk::to_string(MyEnum::bar) == "bar");
        CHECK(mk::from_string(mk::tag<MyEnum>, "bar") == MyEnum::bar);
        CHECK_THROWS(mk::from_string(mk::tag<MyEnum>, "baz"));
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


#if 0
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
