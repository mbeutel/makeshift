
#include <makeshift/metadata.hpp>
#include <makeshift/serialize.hpp>
#include <makeshift/functional.hpp>
#include <makeshift/experimental/property.hpp>

#include <sstream>
#include <string>

#include <catch.hpp>

namespace mk = makeshift;

namespace
{

struct MyStruct
{
    int foo;
    float bar(void) const { }
    void setBar(float) const { }
    double baz;
};
static constexpr auto reflect(mk::tag<MyStruct>, mk::any_tag_of<mk::reflection_tag, mk::serialization_tag>)
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
    return std::get<std::string_view>(mk::metadata_of<T, mk::serialization_tag>.attributes);
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
              typename = std::enable_if_t<mk::is_tuple_like_v<std::decay_t<TupleT>>>>
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

template <typename T> using is_property_metadata = mk::is_instantiation_of<T, mk::property_metadata>;
template <typename T> using is_not_none = std::negation<std::is_same<T, mk::none_t>>;

template <typename T, typename AttributesT>
    std::string to_string(const mk::type_metadata<T, AttributesT>& typeMetadata, serialize_as_table_t)
{
    return typeMetadata.attributes
        | mk::tuple_filter(trait_v<is_property_metadata>)
        | mk::tuple_map([&](const auto& propMetadata)
          {
              constexpr auto accessor = mk::get_property_accessor(typeMetadata, propMetadata);
              using Accessor = decltype(accessor);
              if constexpr (mk::has_getter_v<Accessor>)
                  return get<std::string_view>(propMetadata.attributes);
              else
                  return none;
          })
        | mk::tuple_filter(trait_v<is_not_none>)
        | string_join("\t");
}

template <typename T>
    void to_stream(std::ostream& stream, const T& value, const serialize_as_table_t& sctx)
{
    auto readableProperties = metadata_of<T, mk::serialization_tag>.attributes
        | mk::tuple_filter<is_property_metadata>()
        | mk::tuple_map([&](const auto& propMetadata)
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
