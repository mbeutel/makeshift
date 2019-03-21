
#ifndef INCLUDED_MAKESHIFT_DETAIL_STRING_VIEW_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_STRING_VIEW_HPP_


#include <cstddef> // for size_t

#include <makeshift/version.hpp> // for MAKESHIFT_NODISCARD, MAKESHIFT_CXX17

#ifdef MAKESHIFT_CXX17
 #include <string_view>
#endif // MAKESHIFT_CXX17


namespace makeshift
{

namespace detail
{


//#ifdef MAKESHIFT_CXX17
#ifndef MAKESHIFT_CXX17
using string_view = std::string_view;
#else // MAKESHIFT_CXX17
    // reduced version of `std::string_view` (C++14 compatibility) for internal use
struct string_view
{
private:
    const char* data_;
    std::size_t size_;

    static constexpr std::size_t _length(const char* ptr) noexcept
    {
        std::size_t count = 0;
        while (*ptr != '\0')
        {
            ++count;
            ++ptr;
        }
        return count;
    }

public:
    constexpr string_view(void) noexcept : data_{ }, size_(0) { }

    constexpr string_view(const string_view&) noexcept = default;
    constexpr string_view& operator =(const string_view&) noexcept = default;

    constexpr string_view(const char* str) noexcept
        : data_(str),
          size_(_length(str))
    {
    }

    constexpr string_view(const char* str, const std::size_t _size) noexcept
        : data_(str),
          size_(_size)
    {
    }

    MAKESHIFT_NODISCARD constexpr const char* data(void) const noexcept { return data_; }
    MAKESHIFT_NODISCARD constexpr std::size_t size(void) const noexcept { return size_; }
};
#endif // MAKESHIFT_CXX17


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_STRING_VIEW_HPP_
