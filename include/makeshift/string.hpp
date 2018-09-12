
#ifndef INCLUDED_MAKESHIFT_STRING_HPP_
#define INCLUDED_MAKESHIFT_STRING_HPP_


#include <string>
#include <string_view>
#include <type_traits> // for decay<>, enable_if<>

#include <makeshift/type_traits.hpp> // for tag<>, is_serializer<>

#include <makeshift/detail/string_compare.hpp>


namespace makeshift
{

inline namespace serialize
{


    // To customize string serialization for arbitrary types, define your own serializer type along with `to_string_impl()`, `from_string_impl()`
    // overloads discoverable by ADL.
    //
    // To override parts of the behavior of an existing serializer, define a new serializer that handles only the types you want to customize, and
    // chain it with the existing serializer using `chain_serializers()`.


    //ᅟ
    // Serializes the given value as string using the provided serializer.
    //ᅟ
    //ᅟ    std::string s = to_string(42, string_serializer); // returns "42"s
    //
template <typename T, typename SerializerT>
    std::string to_string(const T& value, SerializerT& serializer)
{
    return to_string_impl(value, serializer, serializer);
}


    //ᅟ
    // Deserializes the given value from a string using the provided serializer.
    //ᅟ
    //ᅟ    int i = from_string<int>("42", string_serializer); // returns 42
    //
template <typename T, typename SerializerT,
          typename = std::enable_if_t<is_serializer_v<std::decay_t<SerializerT>>>>
    T from_string(std::string_view string, SerializerT& serializer, tag<T> = { })
{
    return from_string_impl(tag_v<T>, string, serializer, serializer);
}


} // inline namespace serialize

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_STRING_HPP_
