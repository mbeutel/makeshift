
#ifndef INCLUDED_MAKESHIFT_DETAIL_QUANTITY_STRING_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_QUANTITY_STRING_HPP_


#include <string>
#include <string_view>

#include <makeshift/quantity.hpp>
#include <makeshift/string.hpp>

#include <makeshift/detail/export.hpp> // for MAKESHIFT_PUBLIC


namespace makeshift
{

inline namespace types
{


MAKESHIFT_PUBLIC std::string unit_to_string(quantity_unit unit);
MAKESHIFT_PUBLIC quantity_unit unit_from_string(std::string_view string);


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_QUANTITY_STRING_HPP_
