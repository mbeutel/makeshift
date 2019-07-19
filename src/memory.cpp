
#include <numeric> // for lcm()
#include <cstddef> // for size_t, ptrdiff_t, max_align_t

#include <gsl/gsl_assert> // for Expects()

#include <makeshift/new.hpp>    // for hardware_page_size(), hardware_cache_line_size()
#include <makeshift/memory.hpp>



namespace makeshift
{

namespace detail
{


static inline std::size_t alignment_in_bytes(alignment a) noexcept
{
    switch (a)
    {
    case alignment::page:
        return hardware_page_size();
    case alignment::cache_line:
        return hardware_cache_line_size();
    case alignment::none:
        return 0;
    }
    Expects(std::ptrdiff_t(a) > 0);
    return std::ptrdiff_t(a);
}
std::size_t alignment_in_bytes(alignment a, std::size_t alignOfT) noexcept
{
    return std::lcm(alignment_in_bytes(a), alignOfT);
}


} // namespace detail

} // namespace makeshift