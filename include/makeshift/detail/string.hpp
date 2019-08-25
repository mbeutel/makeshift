
#ifndef INCLUDED_MAKESHIFT_DETAIL_STRING_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_STRING_HPP_


#include <cstddef>     // for size_t
#include <string_view>

#include <makeshift/detail/export.hpp> // for MAKESHIFT_PUBLIC


namespace makeshift
{


enum class string_comparison : int;
struct string_comparer_options;


namespace detail
{


MAKESHIFT_PUBLIC bool string_equal_to(const string_comparer_options& options, std::string_view lhs, std::string_view rhs) noexcept;
MAKESHIFT_PUBLIC bool string_less(const string_comparer_options& options, std::string_view lhs, std::string_view rhs) noexcept;
MAKESHIFT_PUBLIC std::size_t string_hash(const string_comparer_options& options, std::string_view obj) noexcept;


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_STRING_HPP_
