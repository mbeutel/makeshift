
#include <cstddef>     // for size_t
#include <cctype>      // for tolower()
#include <algorithm>   // for equal(), lexicographical_compare()

#include <makeshift/detail/string_compare.hpp>


namespace makeshift
{

namespace detail
{


constexpr std::size_t fnv_offset_basis(void) noexcept
{
    if constexpr (sizeof(std::size_t) == 4) return 2166136261U;
    else if constexpr (sizeof(std::size_t) == 8) return 14695981039346656037ULL;
}
constexpr std::size_t fnv_prime(void) noexcept
{
    if constexpr (sizeof(std::size_t) == 4) return 16777619U;
    else if constexpr (sizeof(std::size_t) == 8) return 1099511628211ULL;
}

struct fnv1a_hasher
{
    std::size_t val = fnv_offset_basis();

    std::size_t add_byte(unsigned char b) noexcept
    {
        val ^= static_cast<std::size_t>(b);
        val *= fnv_prime();
        return val;
    }
    std::size_t add_bytes(const unsigned char* first, const unsigned char* last) noexcept
    {
        for (; first != last; ++first)
            add_byte(*first);
        return val;
    }
};


bool string_equal_to(const string_comparer_options& options, std::string_view lhs, std::string_view rhs) noexcept
{
    if (options.comparison == string_comparison::ordinal_ignore_case)
        return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
            [](char a, char b) { return std::tolower(a) == std::tolower(b); });
    else
        return lhs == rhs;
}
bool string_less(const string_comparer_options& options, std::string_view lhs, std::string_view rhs) noexcept
{
    if (options.comparison == string_comparison::ordinal_ignore_case)
        return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
            [](char a, char b) { return std::tolower(a) < std::tolower(b); });
    else
        return lhs < rhs;

}
std::size_t string_hash(const string_comparer_options& options, std::string_view obj) noexcept
{
    makeshift::detail::fnv1a_hasher hasher;
    if (options.comparison == string_comparison::ordinal_ignore_case)
    {
        for (auto ch : obj)
            hasher.add_byte(static_cast<unsigned char>(char(std::tolower(ch))));
    }
    else
    {
        for (auto ch : obj)
            hasher.add_byte(static_cast<unsigned char>(char(ch)));
    }
    return hasher.val;
}


} // namespace detail

} // namespace makeshift
