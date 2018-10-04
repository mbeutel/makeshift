
#include <string>
#include <string_view>
#include <iostream>
#include <sstream>

#include <makeshift/streamable.hpp>
#include <makeshift/string.hpp>


namespace makeshift
{

namespace detail
{


std::string streamable_to_string(ostreamable_arg arg)
{
    std::ostringstream sstr;
    sstr << arg;
    return sstr.str();
}
void streamable_from_string(istreamable_arg arg, std::string_view string)
{
    std::istringstream sstr;
    sstr.str(std::string(string));
    sstr >> arg;
}


} // namespace detail

} // namespace makeshift
