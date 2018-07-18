
#include <cstddef>     // for size_t
#include <cctype>      // for tolower()
#include <type_traits> // for is_trivial<>
#include <algorithm>   // for equal(), lexicographical_compare()

#include <makeshift/detail/string_compare.hpp>


namespace makeshift
{

namespace detail
{


template <unsigned HashSize>
    struct fnv1a_hasher_base;
template <>
    struct fnv1a_hasher_base<4>
{
    static constexpr std::size_t fnv_offset_basis = 2166136261U;
    static constexpr std::size_t fnv_prime = 16777619U;
};
template <>
    struct fnv1a_hasher_base<8>
{
    static constexpr std::size_t fnv_offset_basis = 14695981039346656037ULL;
    static constexpr std::size_t fnv_prime = 1099511628211ULL;
};
struct fnv1a_hasher : fnv1a_hasher_base<sizeof(std::size_t)>
{
    std::size_t val = fnv_offset_basis;

    std::size_t add_byte(unsigned char b) noexcept
    {
        val ^= static_cast<std::size_t>(b);
        val *= fnv_prime;
        return val;
    }
    std::size_t add_bytes(const unsigned char* first, const unsigned char* last) noexcept
    {
        for (; first != last; ++first)
            add_byte(*first);
        return val;
    }
};


} // namespace detail


inline namespace utility
{


bool string_equal_to::operator()(std::string_view lhs, std::string_view rhs) const noexcept
{
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
        [](char a, char b) { return std::tolower(a) == std::tolower(b); });
}

std::size_t string_hash::operator()(std::string_view arg) const noexcept
{
    makeshift::detail::fnv1a_hasher hasher;
    for (auto ch : arg)
        hasher.add_byte(static_cast<unsigned char>(char(std::tolower(ch))));
    return hasher.val;
}

bool string_less::operator()(std::string_view lhs, std::string_view rhs) const noexcept
{
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
        [](char a, char b) { return std::tolower(a) < std::tolower(b); });
}


} // inline namespace utility

} // namespace makeshift
