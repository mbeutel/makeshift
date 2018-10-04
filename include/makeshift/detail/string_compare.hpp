
#ifndef INCLUDED_MAKESHIFT_DETAIL_STRING_COMPARE_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_STRING_COMPARE_HPP_


#include <string_view>
#include <cstddef>     // for size_t

#include <makeshift/detail/export.hpp>              // for MAKESHIFT_DLLFUNC
#include <makeshift/detail/functional_comparer.hpp> // for define_comparer<>


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


struct string_comparer_options
{
    string_comparison comparison = string_comparison::ordinal;

    constexpr string_comparer_options(void) noexcept = default;
    constexpr string_comparer_options(string_comparison _comparison) noexcept : comparison(_comparison) { }
};


} // inline namespace utility


namespace detail
{


MAKESHIFT_DLLFUNC bool string_equal_to(const string_comparer_options& options, std::string_view lhs, std::string_view rhs) noexcept;
MAKESHIFT_DLLFUNC bool string_less(const string_comparer_options& options, std::string_view lhs, std::string_view rhs) noexcept;
MAKESHIFT_DLLFUNC std::size_t string_hash(const string_comparer_options& options, std::string_view obj) noexcept;


} // namespace detail



inline namespace utility
{


    //ᅟ
    // String comparer which handles different string comparison modes.
    //
template <typename BaseT = void>
    struct string_comparer : define_comparer<string_comparer, BaseT, string_comparer_options>
{
    using base = define_comparer<makeshift::string_comparer, BaseT, string_comparer_options>;
    using base::base;

    template <typename ComparerT>
        friend bool less_impl(std::string_view lhs, std::string_view rhs, const string_comparer& stringComparer, ComparerT&&) noexcept
    {
        return makeshift::detail::string_less(data(stringComparer), lhs, rhs);
    }
    template <typename ComparerT>
        friend bool equal_to_impl(std::string_view lhs, std::string_view rhs, const string_comparer& stringComparer, ComparerT&&) noexcept
    {
        return makeshift::detail::string_equal_to(data(stringComparer), lhs, rhs);
    }
    template <typename ComparerT>
        friend std::size_t hash_impl(std::string_view obj, const string_comparer& stringComparer, ComparerT&&) noexcept
    {
        return makeshift::detail::string_hash(data(stringComparer), obj);
    }
};
string_comparer(void) -> string_comparer<>;
string_comparer(const string_comparer_options&) -> string_comparer<>;
string_comparer(string_comparer_options&&) -> string_comparer<>;


} // inline namespace utility

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_STRING_COMPARE_HPP_
