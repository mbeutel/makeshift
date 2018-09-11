
#include <string>

#include <makeshift/type_traits.hpp>        // for tag<>
#include <makeshift/arithmetic.hpp>         // for checked_cast<>()
#include <makeshift/serializers/string.hpp>


namespace makeshift
{

namespace detail
{


unsigned scalar_from_string(tag<unsigned>, const std::string& string)
{
    return checked_cast<unsigned>(std::stoul(string));
}


} // namespace detail

} // namespace makeshift
