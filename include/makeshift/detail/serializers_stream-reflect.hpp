
#ifndef INCLUDED_MAKESHIFT_DETAIL_SERIALIZERS_STREAM_REFLECT_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_SERIALIZERS_STREAM_REFLECT_HPP_


#include <iostream>
#include <sstream>
#include <utility>  // for move(), forward<>()

#include <makeshift/serialize.hpp> // for metadata_tag_of_serializer<>, parse_error
#include <makeshift/reflect.hpp>   // for member_accessor()
#include <makeshift/tuple.hpp>     // for tuple_foreach()


namespace makeshift
{

namespace detail
{


template <typename T, typename SerializerT>
    void compound_to_stream(std::ostream& stream, const T& value, SerializerT&& serializer)
{
    using MetadataTag = metadata_tag_of_serializer_t<std::decay_t<SerializerT>>;
    constexpr auto members = get_members<T, MetadataTag>();

    stream << "{ ";
    bool first = true;
    tuple_foreach(members, [&](auto&& member)
    {
        if (first)
            first = false;
        else
            stream << ", ";

        auto accessor = member_accessor(member);
        auto theName = get_or_default<std::string_view>(member.attributes);
        if (!theName.empty())
        {
            string_to_stream(stream, theName);
            stream << ": ";
        }
        stream << streamable(accessor(value), serializer);
    });
    stream << " }";
}
template <typename T, typename SerializerT>
    void compound_member_by_index_from_stream(std::istream& stream, T& value, std::size_t index, SerializerT&& serializer)
{
    using MetadataTag = metadata_tag_of_serializer_t<std::decay_t<SerializerT>>;
    constexpr auto members = get_members<T, MetadataTag>();

    std::size_t i = 0;
    bool any = tuple_any(members, [&](auto&& member)
    {
        if (i == index)
        {
            auto accessor = member_accessor(member);
            using Member = typename std::decay_t<decltype(accessor)>::value_type;
            Member memberValue{ accessor(value) };
            stream >> streamable(memberValue, serializer);
            accessor(value, std::move(memberValue));
            return true;
        }
        ++i;
        return false;
    });
    if (!any)
        throw parse_error("error parsing compound argument: too many members");
}
template <typename T, typename SerializerT>
    void compound_member_by_name_from_stream(std::istream& stream, T& value, std::string_view name, SerializerT&& serializer)
{
    using MetadataTag = metadata_tag_of_serializer_t<std::decay_t<SerializerT>>;
    constexpr auto members = get_members<T, MetadataTag>();

    bool any = tuple_any(members, [&](auto&& member)
    {
        auto memberName = get_or_default<std::string_view>(member.attributes);
        if (!memberName.empty() && memberName == name)
        {
            auto accessor = member_accessor(member);
            using Member = typename std::decay_t<decltype(accessor)>::value_type;
            Member memberValue{ accessor(value) };
            stream >> streamable(memberValue, serializer);
            accessor(value, std::move(memberValue));
            return true;
        }
        return false;
    });
    if (!any)
        throw parse_error("error parsing compound argument: member '" + std::string(name) + "' not found");
}
template <typename T, typename SerializerT>
    void compound_from_stream(std::istream& stream, T& value, SerializerT&& serializer)
{
    stream >> std::ws;
    char ch;
    if (!stream.get(ch) || ch != '{')
        throw parse_error("error parsing compound argument: expected '{'");

    bool hadNamedArgs = false;
    std::size_t pos = 0;

    stream >> std::ws;
    if (!stream.get(ch))
        throw parse_error("error parsing compound argument: expected name, value, or '}'");
    while (ch != '}')
    {
        stream.unget();
        if (ch == '"') // either a named argument or a positional string value
        {
            std::string str;
            if (!(stream >> std::quoted(str)))
                throw parse_error("error parsing compound argument: expected name or value");
#if defined(_MSC_VER) && _MSC_VER == 1915
            stream.get(ch); // workaround for VS 2017 15.8 bug https://developercommunity.visualstudio.com/content/problem/314993/stdquoted-broken-in-158-fails-to-properly-extract.html
#endif // defined(_MSC_VER) && _MSC_VER == 1915
            stream >> std::ws;
            if (!stream.get(ch) || (ch != ':' && ch != ',' && ch != '}'))
                throw parse_error("error parsing compound argument: expected ':', ',', or '}'");
            if (ch == ':')
            {
                    // named argument; look up and deserialize
                hadNamedArgs = true;
                compound_member_by_name_from_stream(stream, value, str, serializer);
                stream >> std::ws;
            }
            else if (ch == '}')
                break;
            else // ch == ','
            {
                if (hadNamedArgs)
                    throw parse_error("error parsing compound argument: positional members cannot appear after named members");

                    // this is awkward: we have to put the value back in a stream and deserialize it using the serializer and the proper type
                std::stringstream sstr;
                sstr << std::quoted(str);
                compound_member_by_index_from_stream(sstr, value, pos, serializer);
                ++pos;
            }
        }
        else
        {
                // positional string value
            if (hadNamedArgs)
                throw parse_error("error parsing compound argument: positional members cannot appear after named members");
            compound_member_by_index_from_stream(stream, value, pos, serializer);
            ++pos;
        }
        stream >> std::ws;
        if (!stream.get(ch) || (ch != ',' && ch != '}'))
            throw parse_error("error parsing compound argument: expected ',' or '}'");
        if (ch == '}')
            break;
        stream >> std::ws;
        if (!stream.get(ch))
            throw parse_error("error parsing compound argument: expected name, value, or '}'");
    }
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_SERIALIZERS_STREAM_REFLECT_HPP_
