
#ifndef INCLUDED_MAKESHIFT_RANGES_HPP_
#define INCLUDED_MAKESHIFT_RANGES_HPP_


#include <cstddef>      // for size_t
#include <utility>      // for move(), tuple_size<>, forward<>()
#include <type_traits>  // for enable_if<>, is_convertible<>

#include <gsl-lite/gsl-lite.hpp>  // for index, dim, gsl_Expects(), gsl_CPP17_OR_GREATER

#if !gsl_CPP17_OR_GREATER
# error makeshift requires C++17 mode or higher
#endif // !gsl_CPP17_OR_GREATER

#if gsl_CPP20_OR_GREATER
# include <ranges>  // for borrowed_range<>
#endif // gsl_CPP20_OR_GREATER

#include <makeshift/detail/ranges.hpp>


namespace makeshift {

namespace gsl = ::gsl_lite;


    //
    // Represents a pair of iterators.
    //
template <typename It, typename EndIt = It, std::ptrdiff_t Extent = -1>
class range : public detail::range_base<It, EndIt, detail::range_iterator_tag<It>, Extent>
{
    using base = detail::range_base<It, EndIt, detail::range_iterator_tag<It>, Extent>;

public:
    constexpr range(It first, EndIt last)
        : base(std::move(first), std::move(last))
    {
    }
    template <typename ExtentC,
              std::enable_if_t<std::is_convertible<ExtentC, std::size_t>::value, int> = 0>
    constexpr range(It start, ExtentC extentC)
        : base(start, start + extentC)
    {
        constexpr std::ptrdiff_t rhsExtent = detail::range_extent_from_constval(ExtentC{ });
        static_assert(Extent == -1 || rhsExtent == -1 || Extent == rhsExtent, "static extents must match");
        gsl_Expects(Extent == -1 || static_cast<std::ptrdiff_t>(extentC) == Extent);
    }
};
template <typename It, typename EndIt>
range(It, EndIt) -> range<It, std::enable_if_t<!std::is_convertible<EndIt, std::size_t>::value, EndIt>, -1>;
template <typename It, typename ExtentC>
range(It, ExtentC) -> range<It, std::enable_if_t<std::is_convertible<ExtentC, std::size_t>::value, It>, detail::range_extent_from_constval(ExtentC{ })>;

    //
    // Construct a range from a pair of iterators.
    //
template <typename It, typename EndIt>
[[nodiscard]] std::enable_if_t<!std::is_convertible<EndIt, std::size_t>::value, range<It, EndIt>>
make_range(It first, EndIt last)
{
    // the is_convertible<> trait also covers integer constvals due to normalization

    return { std::move(first), std::move(last) };
}

    //
    // Construct a range from an iterator and an extent.
    //
template <typename It, typename ExtentC>
[[nodiscard]] std::enable_if_t<!std::is_convertible<ExtentC, std::size_t>::value, range<It, It, detail::range_extent_from_constval(ExtentC{ })>>
make_range(It start, ExtentC extentC)
{
    return { std::move(start), extentC };
}


    //
    // Represents a range of index values [0, num).
    //ᅟ
    //ᅟ    auto indices = index_range(3);
    //ᅟ    for (auto index : indices)
    //ᅟ    {
    //ᅟ        std::cout << index << '\n';
    //ᅟ    }
    //ᅟ    // prints "0\n1\n2\n"
    //
class index_range
{
private:
    gsl::index first_;
    gsl::index last_;

public:
    using const_iterator = index_iterator;
    using iterator = index_iterator;

    explicit constexpr index_range(gsl::index _num)
        : first_(0), last_(_num)
    {
        gsl_Expects(_num >= 0);
    }
    explicit constexpr index_range(gsl::index _first, gsl::index _last)
        : first_(_first), last_(_last)
    {
        gsl_Expects(_first <= _last);
    }
    constexpr const_iterator
    begin(void) const noexcept
    {
        return const_iterator(first_);
    }
    constexpr const_iterator
    end(void) const noexcept
    {
        return const_iterator(last_);
    }
};


} // namespace makeshift


    // Implement tuple-like protocol for fixed-size `range<>`.
template <typename It, std::ptrdiff_t Extent> class std::tuple_size<makeshift::range<It, It, Extent>> : public std::integral_constant<std::size_t, Extent> { };
template <typename It, typename EndIt> class std::tuple_size<makeshift::range<It, EndIt, -1>>; // not defined
template <std::size_t I, typename It, std::ptrdiff_t Extent> class std::tuple_element<I, makeshift::range<It, It, Extent>> { public: using type = std::decay_t<decltype(*std::declval<It>())>; };
template <std::size_t I, typename It, typename EndIt> class std::tuple_element<I, makeshift::range<It, EndIt, -1>>; // not defined

    // Declare `range<>` and `index_range<>` as borrowed ranges.
#if gsl_CPP20_OR_GREATER
template <typename It, typename EndIt, std::ptrdiff_t Extent>
inline constexpr bool std::ranges::enable_borrowed_range<makeshift::range<It, EndIt, Extent>> = true;
template <>
inline constexpr bool std::ranges::enable_borrowed_range<makeshift::index_range> = true;
#endif // gsl_CPP20_OR_GREATER

#endif // INCLUDED_MAKESHIFT_RANGES_HPP_
