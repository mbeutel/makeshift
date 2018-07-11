
#ifndef MAKESHIFT_VERSION_HPP_
#define MAKESHIFT_VERSION_HPP_


#include <makeshift/detail/cfg.hpp> // for MAKESHIFT_DLLFUNC


#define MAKESHIFT_VERSION_MAJOR 1
#define MAKESHIFT_VERSION_MINOR 0
#define MAKESHIFT_VERSION_PATCH 0

#define MAKESHIFT_VERSION_API 1


namespace makeshift
{

namespace detail
{


MAKESHIFT_DLLFUNC void get_version(int& major, int& minor, int& patch, int& api);


} // namespace detail

} // namespace makeshift


#endif // MAKESHIFT_VERSION_HPP_
