
#ifndef INCLUDED_MAKESHIFT_RANGE_HPP_
#define INCLUDED_MAKESHIFT_RANGE_HPP_


#include <cstddef>     // for size_t
#include <utility>     // for move(), forward<>()
#include <type_traits> // for enable_if<>, is_convertible<>

#include <makeshift/constval.hpp> // for is_constval<>
#include <makeshift/version.hpp>  // for MAKESHIFT_NODISCARD

#include <makeshift/detail/range.hpp>


namespace makeshift
{


    //ᅟ
    // Represents a pair of iterators.
    //
template <typename It, typename EndIt = It>
    struct range : makeshift::detail::range_base_<It, range<It, EndIt>>::type
{
    It first;
    EndIt last;

    constexpr range(It _first, EndIt _last)
        : first(std::move(_first)), last(std::move(_last))
    {
    }
    template <std::size_t Size>
        constexpr range(makeshift::detail::fixed_random_access_range<It, Size> _rhs)
            : first(std::move(_rhs.first)), last(first + Size)
    {
    }

    MAKESHIFT_NODISCARD constexpr const It& begin(void) const noexcept { return first; }
    MAKESHIFT_NODISCARD constexpr const EndIt& end(void) const noexcept { return last; }
};

    //ᅟ
    // Construct a range from a pair of iterators.
    //
template <typename It, typename EndIt,
          std::enable_if_t<!std::is_convertible<EndIt, std::size_t>::value, int> = 0>
    MAKESHIFT_NODISCARD range<It, EndIt> make_range(It first, EndIt last)
{
    // the is_convertible<> trait also covers integer constvals due to normalization

    return { std::move(first), std::move(last) };
}

    //ᅟ
    // Construct a range from an iterator and an extent.
    //
template <typename It, typename ExtentC,
          std::enable_if_t<std::is_convertible<ExtentC, std::size_t>::value, int> = 0>
    MAKESHIFT_NODISCARD auto make_range(It start, ExtentC extentC)
{
    return typename makeshift::detail::range_by_extent_<It, ExtentC, range>::type{ start, start + makeshift::constval_extract(extentC) };
}


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_RANGE_HPP_
