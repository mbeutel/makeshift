
#include <string>

#include <gsl/gsl-lite.hpp> // for narrow<>()

#include <makeshift/type_traits.hpp>        // for tag<>
#include <makeshift/serializers/string.hpp>


namespace makeshift
{

namespace detail
{


unsigned scalar_from_string(tag<unsigned>, const std::string& string)
{
    return gsl::narrow<unsigned>(std::stoul(string));
}


} // namespace detail

} // namespace makeshift
