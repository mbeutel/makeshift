
#include <string>
#include <string_view>
#include <iostream>
#include <iomanip>

#include <makeshift/serializers/stream.hpp>


namespace makeshift
{

namespace detail
{


void string_to_stream(std::ostream& stream, std::string_view string)
{
    stream << std::quoted(string);
}
void string_from_stream(std::istream& stream, std::string& string)
{
    stream >> std::quoted(string);
}


} // namespace detail

} // namespace makeshift
