
#ifndef INCLUDED_MAKESHIFT_DETAIL_STRING_COMPARE_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_STRING_COMPARE_HPP_


#include <string_view>
#include <cstddef>     // for size_t

#include <makeshift/detail/cfg.hpp>  // for MAKESHIFT_DLLFUNC


namespace makeshift
{

inline namespace utility
{


    //ᅟ
    // Specifies a string comparison mode.
    //
enum class string_comparison
{
    //ᅟ
    // Compares strings using ordinal (binary) ordering rules.
    //
    ordinal,

    //ᅟ
    // Compares strings using ordinal (binary) ordering rules. Ignores case of ASCII characters.
    //
    ordinal_ignore_case
};


    //ᅟ
    // Compares the two strings for equality using the given string comparison mode.
    //
class string_equal_to
{
private:
    string_comparison comparison_;

public:
    constexpr string_equal_to(string_comparison _comparison) noexcept : comparison_(_comparison) { }

    MAKESHIFT_DLLFUNC bool operator()(std::string_view lhs, std::string_view rhs) const noexcept;
};


    //ᅟ
    // Computes the hash of a string using the given string comparison mode.
    //
class string_hash
{
private:
    string_comparison comparison_;

public:
    constexpr string_hash(string_comparison _comparison) noexcept : comparison_(_comparison) { }

    MAKESHIFT_DLLFUNC std::size_t operator()(std::string_view arg) const noexcept;
};


    //ᅟ
    // Compares the two strings lexicographically using the given string comparison mode. Returns `true` if lower-case `lhs`
    // lexicographically precedes lower-case `rhs`, `false` otherwise.
    //
class string_less
{
private:
    string_comparison comparison_;

public:
    constexpr string_less(string_comparison _comparison) noexcept : comparison_(_comparison) { }

    MAKESHIFT_DLLFUNC bool operator()(std::string_view lhs, std::string_view rhs) const noexcept;
};


} // inline namespace utility

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_STRING_COMPARE_HPP_
