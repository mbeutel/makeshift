
#ifndef INCLUDED_MAKESHIFT_DETAIL_QUANTITY_STRING_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_QUANTITY_STRING_HPP_


#include <string>
#include <string_view>

#include <makeshift/quantity.hpp>
#include <makeshift/string.hpp>


namespace makeshift
{

inline namespace types
{


MAKESHIFT_DLLFUNC std::string unit_to_string(quantity_unit unit);
MAKESHIFT_DLLFUNC quantity_unit unit_from_string(std::string_view string);


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_QUANTITY_STRING_HPP_
