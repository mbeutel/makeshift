
#include <string>
#include <string_view>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cctype>

#include <gsl/gsl_assert> // for Expects()

#include <makeshift/string.hpp>
#include <makeshift/quantity.hpp>

#include <makeshift/serializers/stream.hpp>
#include <makeshift/serializers/quantity.hpp>

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


static bool isIdentifierStartChar(char ch)
{
    return std::isalpha(ch) || ch == '_' || ch == '.';
}
static bool isIdentifierChar(char ch)
{
    return isIdentifierStartChar(ch) || std::isdigit(ch) || ch == '-';
}

void raw_string_to_stream(std::ostream& stream, std::string_view string)
{
    stream << string;
}

bool tryPeekChar(std::istream& stream, char& ch)
{
    int result = stream.peek();
    if (result == std::istream::traits_type::eof())
        return false;
    ch = char(result);
    return true;
}

void name_to_stream(std::ostream& stream, std::string_view name, std::string_view nameIndicator)
{
    if (name.size() == 0
     || !isIdentifierStartChar(name.front())
     || !std::all_of(name.begin(), name.end(), isIdentifierChar))
    {
            // need quoting
        string_to_stream(stream, name);
    }
    else
    {
        stream << nameIndicator << name;
    }
    if (!stream)
        raise_ostream_error(stream);
}
void name_to_stream(std::ostream& stream, std::string_view name)
{
    name_to_stream(stream, name, { });
}
void name_from_stream(std::istream& stream, std::string& name)
{
    stream >> std::ws;
    char ch;
    if (tryPeekChar(stream, ch))
    {
        if (ch == '"')
            string_from_stream(stream, name);
        else if (isIdentifierStartChar(ch))
        {
            name = ch;
            stream.get();
            while (tryPeekChar(stream, ch) && isIdentifierChar(ch))
            {
                name += ch;
                stream.get();
            }
        }
    }
    if (!stream)
        raise_istream_error(stream);
}

void unit_to_stream(std::ostream& stream, quantity_unit unit)
{
    stream << unit_to_string(unit);
}

void unit_str_from_stream(std::istream& stream, std::string& str)
{
    stream >> std::ws;
    char ch;
    if (tryPeekChar(stream, ch))
    {
        if (makeshift::detail::to_min_char(ch) >= 0)
        {
            str = ch;
            stream.get();
            while (tryPeekChar(stream, ch) && makeshift::detail::to_min_char(ch) >= 0)
            {
                str += ch;
                stream.get();
            }
        }
    }
    if (!stream)
        raise_istream_error(stream);
}

void unit_from_stream(std::istream& stream, quantity_unit& unit)
{
    std::string str;
    unit_str_from_stream(stream, str);
    unit = unit_from_string(str);
}


char trim_delim_char(std::string_view s); // defined in serialize_enum.cpp

template <typename T, typename MapT>
    static std::string listOfAlternatives(std::initializer_list<T> args, MapT&& stringMap)
{
    switch (args.size())
    {
    case 0: return { };
    case 1: return stringMap(*args.begin());
    case 2: return stringMap(*args.begin()) + " or " + stringMap(*std::next(args.begin()));
    default:
        {
            std::string result;
            auto beforeEnd = std::prev(args.end());
            for (auto it = args.begin(); it != beforeEnd; ++it)
            {
                result += stringMap(*it);
                result += ", ";
            }
            result += " or ";
            result += stringMap(*beforeEnd);
            return result;
        }
    }
}

static void compound_from_stream_impl(std::istream& stream, stream_compound_member_deserializer_base& memberDeserializer, const compound_serialization_options& options)
{
    std::streampos startPos = stream.tellg();
    std::size_t offset = 0;

    std::size_t argPos = 0;
    bool hadNamedArgsOutOfOrder = false;
    auto member_by_name = [&hadNamedArgsOutOfOrder, &memberDeserializer, &argPos, &offset](std::istream& istream, std::string_view name)
    {
        std::size_t index = std::size_t(-1);
        bool found = true;
        try
        {
            istream >> std::ws;
            found = memberDeserializer.by_name(istream, name, index);
            if (found && index != argPos)
                hadNamedArgsOutOfOrder = true;
        }
        catch (const std::runtime_error&)
        {
            std::throw_with_nested(parse_error("parsing compound:", offset));
        }
        if (!found)
            throw parse_error("parsing compound: member '" + std::string(name) + "' not found", offset);
        ++argPos;
    };
    auto member_by_pos = [&hadNamedArgsOutOfOrder, &memberDeserializer, &argPos, &offset](std::istream& istream)
    {
        if (hadNamedArgsOutOfOrder)
            throw parse_error("parsing compound: positional members cannot appear after out-of-order named members", offset);
        bool found = true;
        try
        {
            istream >> std::ws;
            found = memberDeserializer.by_index(istream, argPos);
        }
        catch (const std::runtime_error&)
        {
            std::throw_with_nested(parse_error("parsing compound:", offset));
        }
        if (!found)
            throw parse_error("parsing compound: too many members", offset);
        ++argPos;
    };

    char leftDelim = trim_delim_char(options.opening_delimiter);
    char rightDelim = trim_delim_char(options.closing_delimiter);
    char nameValueSep = trim_delim_char(options.name_value_separator);
    char nameIndicator = trim_delim_char(options.name_indicator);
    char elementDelim = trim_delim_char(options.element_delimiter);

    auto peekDelim = [&stream, &startPos, &offset](std::initializer_list<char> expectedChars)
    {
        Expects(expectedChars.size() > 0);
        stream >> std::ws;
        offset = std::size_t(stream.tellg() - startPos);
        char ch;
        if (!tryPeekChar(stream, ch) || std::find(expectedChars.begin(), expectedChars.end(), ch) == expectedChars.end())
        {
            throw parse_error("parsing compound: expected " + listOfAlternatives<char>(expectedChars,
                    [](char expectedChar)
                    {
                        std::string result;
                        result += '\'';
                        result += expectedChar;
                        result += '\'';
                        return result;
                    }),
                offset);
        }
        return ch;
    };
    auto peekNameOrValueOrRightDelim = [&](void)
    {
        stream >> std::ws;
        char ch;
        offset = std::size_t(stream.tellg() - startPos);
        if (!tryPeekChar(stream, ch))
            throw parse_error(std::string("parsing compound: expected name, value, or '") + rightDelim + "'", offset);
        return ch;
    };

    peekDelim({ leftDelim });
    stream.get(); // eat opening delimiter

    char ch = peekNameOrValueOrRightDelim();
    while (ch != rightDelim)
    {
        if (ch == '"') // either a named argument or a positional string value
        {
                // get quoted string
            std::string str;
            string_from_stream(stream, str);

            ch = peekDelim({ nameValueSep, elementDelim, rightDelim });
            if (ch == nameValueSep) // named argument
            {
                stream.get(); // eat name--value delimiter
                member_by_name(stream, str);
                ch = peekDelim({ elementDelim, rightDelim });
            }
            else // element delimiter or closing delimiter => string value
            {
                    // this is awkward: we have to put the value back in a stream and deserialize it using the serializer and the proper type
                std::stringstream sstr;
                string_to_stream(sstr, str);
                member_by_pos(sstr);
            }
        }
        else if (ch == nameIndicator) // member name identifier
        {
            stream.get(); // eat member name indicator
            
            std::string identifier;
            name_from_stream(stream, identifier);
            
            ch = peekDelim({ nameValueSep });
            stream.get(); // eat name--value delimiter

            member_by_name(stream, identifier);
            ch = peekDelim({ elementDelim, rightDelim });
        }
        else // must be a positional value
        {
            member_by_pos(stream);
            ch = peekDelim({ elementDelim, rightDelim });
        }

        if (ch == elementDelim)
        {
            stream.get(); // eat element delimiter
            ch = peekNameOrValueOrRightDelim();
        }
    }
    stream.get(); // eat closing delimiter
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
