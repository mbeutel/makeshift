
#ifndef INCLUDED_MAKESHIFT_HINT_HPP_
#define INCLUDED_MAKESHIFT_HINT_HPP_


#include <string>
#include <type_traits> // for enable_if<>

#include <makeshift/type_traits.hpp> // for tag<>, is_serializer<>


namespace makeshift
{

inline namespace serialize
{


    //ᅟ
    // Options for formatting hints for enum types.
    //
struct enum_hint_options
{
        //ᅟ
        // Determines separator for enumeration values.
        //
    std::string_view option_separator = "|";

        //ᅟ
        // Determines separator for enum flags.
        //
    std::string_view flags_separator = ",";

    constexpr enum_hint_options(void) = default;
};


    //ᅟ
    // Options for formatting hints for compound types.
    //
struct compound_hint_options
{
        //ᅟ
        // Determines opening compound delimiter.
        //
    std::string_view opening_delimiter = "{ ";

        //ᅟ
        // Determines closing compound delimiter.
        //
    std::string_view closing_delimiter = " }";

        //ᅟ
        // Determines the element delimiter.
        //
    std::string_view element_delimiter = ", ";

        //ᅟ
        // Determines the placeholder to be used for members without name.
        //
    std::string_view unnamed_member_placeholder = "val";
};

    //ᅟ
    // Options for formatting hints for compound types and compound value types.
    //
struct any_compound_hint_options
{
    compound_hint_options compound = { "{", "}", ",", "val" };
    compound_hint_options compound_value = { "(", ")", ",", "val" };
};


    //ᅟ
    // Retrieves a hint for the given type.
    //
template <typename T, typename SerializerT,
          typename = std::enable_if_t<is_serializer_v<std::decay_t<SerializerT>>>>
    std::string get_hint(SerializerT&& serializer, tag<T> = { })
{
    return get_hint_impl(tag_v<T>, serializer, serializer);
}


} // inline namespace serialize

} // namespace makeshift



#endif // INCLUDED_MAKESHIFT_HINT_HPP_
