
#include <numeric> // for gcd()

#include <makeshift/serializers/quantity.hpp>


namespace makeshift
{

namespace detail
{


void time_unit_to_stream(std::ostream& stream, std::intmax_t num, std::intmax_t den)
{
    // TODO
}
void time_unit_from_stream(std::istream& stream, std::intmax_t& num, std::intmax_t& den)
{
    // TODO
}
void time_unit_combine(std::intmax_t& num, std::intmax_t& den, std::intmax_t dstNum, std::intmax_t dstDen)
{
     // TODO
}
/*
void convert_time_unit_to_seconds(std::intmax_t& num, std::intmax_t& den, std::string_view srcUnit)
{
}
*/

} // namespace detail

} // namespace makeshift
