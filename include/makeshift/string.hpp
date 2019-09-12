
#ifndef INCLUDED_MAKESHIFT_STRING_HPP_
#define INCLUDED_MAKESHIFT_STRING_HPP_


#include <makeshift/macros.hpp> // for MAKESHIFT_CXX, MAKESHIFT_NODISCARD

#if MAKESHIFT_CXX < 17
 #error Header <makeshift/string.hpp> requires C++17 mode or higher.
#endif // MAKESHIFT_CXX < 17

#include <string_view>

#include <makeshift/detail/string.hpp>


namespace makeshift
{

    //ᅟ
    // Specifies a string comparison mode.
    //
enum class string_comparison : int
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
};


    //ᅟ
    // Configurable string equality comparer.
    //
class string_equal_to
{
private:
    string_comparer_options options_;

public:
    constexpr string_equal_to(string_comparer_options _options)
        : options_(_options)
    {
    }

    MAKESHIFT_NODISCARD bool operator ()(std::string_view lhs, std::string_view rhs) const noexcept
    {
        return makeshift::detail::string_equal_to(options_, lhs, rhs);
    }
};


    //ᅟ
    // Configurable string ordering comparer.
    //
class string_less
{
private:
    string_comparer_options options_;

public:
    constexpr string_less(string_comparer_options _options)
        : options_(_options)
    {
    }

    MAKESHIFT_NODISCARD bool operator ()(std::string_view lhs, std::string_view rhs) const noexcept
    {
        return makeshift::detail::string_less(options_, lhs, rhs);
    }
};


    //ᅟ
    // Configurable string hasher.
    //
class string_hash
{
private:
    string_comparer_options options_;

public:
    constexpr string_hash(string_comparer_options _options)
        : options_(_options)
    {
    }

    MAKESHIFT_NODISCARD std::size_t operator ()(std::string_view obj) const noexcept
    {
        return makeshift::detail::string_hash(options_, obj);
    }
};


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_STRING_HPP_
