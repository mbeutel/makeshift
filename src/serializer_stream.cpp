
#include <string>
#include <string_view>
#include <iostream>
#include <iomanip>

#include <makeshift/serializers/stream.hpp>


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
    if (!(stream << std::quoted(string)))
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


} // namespace detail

} // namespace makeshift
