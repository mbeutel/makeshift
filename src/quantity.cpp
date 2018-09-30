
#include <string>
#include <string_view>
#include <stdexcept> // for runtime_error

#include <makeshift/quantity.hpp>
#include <makeshift/string.hpp>


namespace makeshift
{

inline namespace types
{


std::string unit_to_string(quantity_unit unit)
{
    char buf[makeshift::detail::min_char_max + 1];
    makeshift::detail::quantity_unit_to_string(std::uint64_t(unit), buf);
    return { buf };
}
quantity_unit unit_from_string(std::string_view string)
{
    std::uint64_t result = makeshift::detail::parse_quantity_unit(string.data(), string.size());
    if (result == makeshift::detail::unit_invalid)
    {
        if (string.size() > makeshift::detail::min_char_max)
            throw std::runtime_error("invalid unit '" + std::string(string) + "': too many characters");
        throw std::runtime_error("invalid unit '" + std::string(string) + "': unsupported characters");
    }
    return quantity_unit(result);
}


} // inline namespace types


namespace detail
{


[[noreturn]] void raise_quantity_conversion_error(std::uint64_t from, std::uint64_t to)
{
    throw std::runtime_error("cannot convert from '" + unit_to_string(quantity_unit(from)) + "' to '" + unit_to_string(quantity_unit(to)) + "' incompatible units");
}


} // namespace detail

} // namespace makeshift
