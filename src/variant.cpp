
#include <makeshift/detail/variant.hpp>


namespace makeshift
{


char const* unsupported_runtime_value::what(void) const noexcept
{
    return "unsupported runtime value";
}


} // namespace makeshift
