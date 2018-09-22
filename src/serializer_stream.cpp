
#include <string>
#include <string_view>
#include <iostream>
#include <sstream>
#include <iomanip>

#include <makeshift/serializers/stream.hpp>

#include <makeshift/detail/serializers_stream-reflect.hpp>


namespace makeshift
{

namespace detail
{


[[noreturn]] void raise_ostream_error(std::ostream& stream)
{
    if (stream.bad())
        throw std::runtime_error("error writing to stream");
    if (stream.fail())
        throw std::runtime_error("failed to serialize object");
    throw std::runtime_error("unknown error");
}
[[noreturn]] void raise_istream_error(std::istream& stream)
{
    if (stream.bad())
        throw std::runtime_error("error reading from stream");
    if (stream.eof() && stream.fail())
        throw parse_error("failed to deserialize object: no data");
    if (stream.fail())
        throw parse_error("failed to deserialize object");
    throw std::runtime_error("unknown error");
}


void string_to_stream(std::ostream& stream, std::string_view string)
{
        // workaround for missing string_view overload in libstdc++
    //if (!(stream << std::quoted(string)))
    if (!(stream << std::quoted(std::string(string))))
        raise_ostream_error(stream);
}
void string_from_stream(std::istream& stream, std::string& string)
{
    if (!(stream >> std::quoted(string)))
        raise_istream_error(stream);
#if defined(_MSC_VER) && _MSC_VER == 1915
    char ch;
    stream.get(ch); // workaround for VS 2017 15.8 bug https://developercommunity.visualstudio.com/content/problem/314993/stdquoted-broken-in-158-fails-to-properly-extract.html
#endif // defined(_MSC_VER) && _MSC_VER == 1915
}


char trim_delim_char(std::string_view s); // defined in serialize_enum.cpp

static void compound_from_stream_impl(std::istream& stream, stream_compound_member_deserializer_base& memberDeserializer, const compound_serialization_options& options)
{
    std::size_t offset = 0;

    auto member_by_name = [&](std::istream& istream, std::string_view name)
    {
        bool found = true;
        try
        {
            found = memberDeserializer.by_name(istream, name);
        }
        catch (const std::runtime_error&)
        {
            std::throw_with_nested(parse_error("parsing compound:", offset));
        }
        if (!found)
            throw parse_error("parsing compound: member '" + std::string(name) + "' not found", offset);
    };
    auto member_by_index = [&](std::istream& istream, std::size_t index)
    {
        bool found = true;
        try
        {
            found = memberDeserializer.by_index(istream, index);
        }
        catch (const std::runtime_error&)
        {
            std::throw_with_nested(parse_error("parsing compound:", offset));
        }
        if (!found)
            throw parse_error("parsing compound: too many members", offset);
    };

    char leftDelim = trim_delim_char(options.opening_delimiter);
    char rightDelim = trim_delim_char(options.closing_delimiter);
    char nameValueSep = trim_delim_char(options.name_value_separator);
    char elementDelim = trim_delim_char(options.element_delimiter);

    std::streampos startPos = stream.tellg();

    stream >> std::ws;
    char ch;
    offset = std::size_t(stream.tellg() - startPos);
    if (!stream.get(ch) || ch != leftDelim)
        throw parse_error(std::string("parsing compound: expected '") + leftDelim + "'", offset);

    bool hadNamedArgs = false;
    std::size_t pos = 0;

    stream >> std::ws;
    offset = std::size_t(stream.tellg() - startPos);
    if (!stream.get(ch))
        throw parse_error(std::string("parsing compound: expected name, value, or '") + rightDelim + "'", offset);
    while (ch != rightDelim)
    {
        stream.unget();
        if (ch == '"') // either a named argument or a positional string value
        {
            std::string str;

            string_from_stream(stream, str);
            stream >> std::ws;
            offset = std::size_t(stream.tellg() - startPos);
            if (!stream.get(ch) || (ch != nameValueSep && ch != elementDelim && ch != rightDelim))
                throw parse_error(std::string("parsing compound: expected '") + nameValueSep + "', '" + elementDelim + "', or '" + rightDelim + "'", offset);
            if (ch == nameValueSep)
            {
                    // named argument; look up and deserialize
                hadNamedArgs = true;
                member_by_name(stream, str);
                stream >> std::ws;
            }
            else if (ch == rightDelim)
                break;
            else // ch == elementDelim
            {
                if (hadNamedArgs)
                    throw parse_error("parsing compound: positional members cannot appear after named members", offset);

                    // this is awkward: we have to put the value back in a stream and deserialize it using the serializer and the proper type
                std::stringstream sstr;
                string_to_stream(sstr, str);
                member_by_index(sstr, pos);
                ++pos;
            }
        }
        else
        {
                // positional string value
            if (hadNamedArgs)
                throw parse_error("parsing compound: positional members cannot appear after named members", offset);
            member_by_index(stream, pos);
            ++pos;
        }
        stream >> std::ws;
        offset = std::size_t(stream.tellg() - startPos);
        if (!stream.get(ch) || (ch != elementDelim && ch != rightDelim))
            throw parse_error(std::string("parsing compound: expected '") + elementDelim + "' or '" + rightDelim + "'", offset);
        if (ch == rightDelim)
            break;
        stream >> std::ws;
        offset = std::size_t(stream.tellg() - startPos);
        if (!stream.get(ch))
            throw parse_error(std::string("parsing compound: expected name, value, or '") + rightDelim + "'", offset);
    }
}

void compound_from_stream(std::istream& stream, stream_compound_member_deserializer_base& memberDeserializer, const compound_serialization_options& options)
{
    //try
    //{
        compound_from_stream_impl(stream, memberDeserializer, options);
    //}
    //catch (...)
    //{
    //    std::throw_with_nested(std::runtime_error("parsing compound"));
    //}
}


} // namespace detail

} // namespace makeshift
