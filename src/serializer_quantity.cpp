
#include <iostream>
#include <numeric>     // for gcd()
#include <stdexcept>
#include <string_view>

#include <makeshift/serializers/quantity.hpp>


namespace makeshift
{

namespace detail
{


struct time_unit_entry
{
    std::string_view str;
    std::intmax_t num, den;

    template <typename PeriodT>
        constexpr time_unit_entry(std::string_view _str, const PeriodT&) noexcept
            : str(_str), num(PeriodT::num), den(PeriodT::den)
    {
    }
};

static time_unit_entry knownTimeUnits[] = {
    { "as",  std::atto    { } },
    { "fs",  std::femto   { } },
    { "ps",  std::pico    { } },
    { "ns",  std::nano    { } },
    { "us",  std::micro   { } },
    { "ms",  std::milli   { } },
    { "cs",  std::centi   { } },
    { "ds",  std::deci    { } },
    { "s",   std::ratio<1>{ } },
    { "das", std::deca    { } },
    { "hs",  std::hecto   { } },
    { "ks",  std::kilo    { } },
    { "Ms",  std::mega    { } },
    { "Gs",  std::giga    { } },
    { "Ts",  std::tera    { } },
    { "Ps",  std::peta    { } },
    { "Es",  std::exa     { } },
    { "min",    std::chrono::minutes::period{ } },
    { "h",      std::chrono::hours::period  { } },
    { "hr",     std::chrono::hours::period  { } },
    { "d",      std::ratio<86400>   { } },
    { "weeks",  std::ratio<604800>  { } },
    { "months", std::ratio<2629746> { } },
    { "a",      std::ratio<31556952>{ } },
    { "years",  std::ratio<31556952>{ } }
};

void time_unit_to_stream(std::ostream& stream, std::intmax_t num, std::intmax_t den)
{
    for (auto& unit : knownTimeUnits)
        if (unit.num == num && unit.den == den)
        {
            stream << unit.str;
            return;
        }
    
    throw std::runtime_error("unknown time unit");
}

    // defined in serializer_stream.hpp
void unit_str_from_stream(std::istream& stream, std::string& str);

void time_unit_from_stream(std::istream& stream, std::intmax_t& num, std::intmax_t& den)
{
    std::string str;
    unit_str_from_stream(stream, str);
    for (auto& unit : knownTimeUnits)
        if (unit.str == str)
        {
            num = unit.num;
            den = unit.den;
            return;
        }
    
    throw std::runtime_error("unknown time unit '" + str + "'");
}
void time_unit_combine(std::intmax_t& num, std::intmax_t& den, std::intmax_t dstNum, std::intmax_t dstDen)
{
    num *= dstDen;
    den *= dstNum;
    std::intmax_t gcd = std::gcd(num, den);
    num /= gcd;
    den /= gcd;
}


} // namespace detail

} // namespace makeshift
