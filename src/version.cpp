
#include <makeshift/version.hpp>


namespace makeshift
{

namespace detail
{


void get_version(int& major, int& minor, int& patch, int& api)
{
    major = MAKESHIFT_VERSION_MAJOR;
    minor = MAKESHIFT_VERSION_MINOR;
    patch = MAKESHIFT_VERSION_PATCH;
    api = MAKESHIFT_VERSION_API;
}


} // namespace detail

} // namespace makeshift
