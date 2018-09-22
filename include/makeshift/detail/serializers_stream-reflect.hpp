
#ifndef INCLUDED_MAKESHIFT_DETAIL_SERIALIZERS_STREAM_REFLECT_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_SERIALIZERS_STREAM_REFLECT_HPP_


#include <iostream>
#include <sstream>
#include <utility>  // for move(), forward<>()

#include <makeshift/serialize.hpp> // for metadata_tag_of_serializer<>, parse_error
#include <makeshift/reflect.hpp>   // for member_accessor()
#include <makeshift/tuple.hpp>     // for tuple_foreach()

#include <makeshift/detail/cfg.hpp>            // for MAKESHIFT_DLLFUNC
#include <makeshift/detail/metadata.hpp>       // for type_flags_of<>
#include <makeshift/detail/string_compare.hpp>


namespace makeshift
{

namespace detail
{


template <typename T, typename SerializerT>
    void compound_to_stream(std::ostream& stream, const T& value, SerializerT&& serializer, const any_compound_serialization_options& compoundOptions)
{
    using MetadataTag = metadata_tag_of_serializer_t<std::decay_t<SerializerT>>;
    constexpr auto members = get_members<T, MetadataTag>();
    constexpr bool isCompoundValue = (type_flags_of<T, MetadataTag> & type_flag::value) != type_flags::none;
    const auto& options = isCompoundValue ? compoundOptions.compound_value : compoundOptions.compound;

    stream << options.opening_delimiter;
    bool first = true;
    tuple_foreach(members, [&](auto&& member)
    {
        if (first)
            first = false;
        else
            stream << options.element_delimiter;

        auto accessor = member_accessor(member);
        auto theName = get_or_default<std::string_view>(member.attributes);
        if (options.with_member_names && !theName.empty())
        {
            string_to_stream(stream, theName);
            stream << options.name_value_separator;
        }
        stream << streamable(accessor(value), serializer);
    });
    stream << options.closing_delimiter;
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
    void compound_member_by_name_from_stream(std::istream& stream, T& value, std::string_view name, SerializerT&& serializer, string_comparison memberNameComparison)
{
    using MetadataTag = metadata_tag_of_serializer_t<std::decay_t<SerializerT>>;
    constexpr auto members = get_members<T, MetadataTag>();

    auto comparer = string_comparer(string_comparer_options{ memberNameComparison });
    bool any = tuple_any(members, [&](auto&& member)
    {
        auto memberName = get_or_default<std::string_view>(member.attributes);
        if (!memberName.empty() && equal_to(comparer)(memberName, name))
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
MAKESHIFT_DLLFUNC char trim_delim_char(std::string_view s); // defined in serialize_enum.cpp
template <typename T, typename SerializerT>
    void compound_from_stream(std::istream& stream, T& value, SerializerT&& serializer, const any_compound_serialization_options& compoundOptions)
{
    using MetadataTag = metadata_tag_of_serializer_t<std::decay_t<SerializerT>>;
    constexpr bool isCompoundValue = (type_flags_of<T, MetadataTag> & type_flag::value) != type_flags::none;
    const auto& options = isCompoundValue ? compoundOptions.compound_value : compoundOptions.compound;

    char leftDelim = trim_delim_char(options.opening_delimiter);
    char rightDelim = trim_delim_char(options.closing_delimiter);
    char nameValueSep = trim_delim_char(options.name_value_separator);
    char elementDelim = trim_delim_char(options.element_delimiter);

    stream >> std::ws;
    char ch;
    if (!stream.get(ch) || ch != leftDelim)
        throw parse_error(std::string("error parsing compound argument: expected '") + leftDelim + "'");

    bool hadNamedArgs = false;
    std::size_t pos = 0;

    stream >> std::ws;
    if (!stream.get(ch))
        throw parse_error(std::string("error parsing compound argument: expected name, value, or '") + rightDelim + "'");
    while (ch != rightDelim)
    {
        stream.unget();
        if (ch == '"') // either a named argument or a positional string value
        {
            std::string str;

            string_from_stream(stream, str);
            stream >> std::ws;
            if (!stream.get(ch) || (ch != nameValueSep && ch != elementDelim && ch != rightDelim))
                throw parse_error(std::string("error parsing compound argument: expected '") + nameValueSep + "', '" + elementDelim + "', or '" + rightDelim + "'");
            if (ch == nameValueSep)
            {
                    // named argument; look up and deserialize
                hadNamedArgs = true;
                compound_member_by_name_from_stream(stream, value, str, serializer, options.member_name_comparison_mode);
                stream >> std::ws;
            }
            else if (ch == rightDelim)
                break;
            else // ch == elementDelim
            {
                if (hadNamedArgs)
                    throw parse_error("error parsing compound argument: positional members cannot appear after named members");

                    // this is awkward: we have to put the value back in a stream and deserialize it using the serializer and the proper type
                std::stringstream sstr;
                string_to_stream(sstr, str);
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
        if (!stream.get(ch) || (ch != elementDelim && ch != rightDelim))
            throw parse_error(std::string("error parsing compound argument: expected '") + elementDelim + "' or '" + rightDelim + "'");
        if (ch == rightDelim)
            break;
        stream >> std::ws;
        if (!stream.get(ch))
            throw parse_error(std::string("error parsing compound argument: expected name, value, or '") + rightDelim + "'");
    }
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_SERIALIZERS_STREAM_REFLECT_HPP_
