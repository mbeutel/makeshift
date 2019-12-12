
#ifndef INCLUDED_MAKESHIFT_RANGES_HPP_
#define INCLUDED_MAKESHIFT_RANGES_HPP_


#include <gsl/gsl-lite.hpp> // for gsl_CPP17_OR_GREATER, gsl_NODISCARD

#if !gsl_CPP17_OR_GREATER
# include <tuple>      // for tuple_size<>
#endif // !gsl_CPP17_OR_GREATER

#include <cstddef>     // for size_t
#include <utility>     // for move(), tuple_size<> (C++17), forward<>()
#include <type_traits> // for enable_if<>, is_convertible<>

#include <gsl/gsl-lite.hpp> // for Expects()

#include <makeshift/detail/ranges.hpp>


namespace makeshift
{


    //ᅟ
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
        Expects(Extent == -1 || static_cast<std::ptrdiff_t>(extentC) == Extent);
    }
};
#if gsl_CPP17_OR_GREATER
template <typename It, typename EndIt>
range(It, EndIt) -> range<It, std::enable_if_t<!std::is_convertible<EndIt, std::size_t>::value, EndIt>, -1>;
template <typename It, typename ExtentC>
range(It, ExtentC) -> range<It, std::enable_if_t<!std::is_convertible<ExtentC, std::size_t>::value, It>, detail::range_extent_from_constval(ExtentC{ })>;
#endif // gsl_CPP17_OR_GREATER

    //ᅟ
    // Construct a range from a pair of iterators.
    //
template <typename It, typename EndIt>
gsl_NODISCARD std::enable_if_t<!std::is_convertible<EndIt, std::size_t>::value, range<It, EndIt>>
make_range(It first, EndIt last)
{
    // the is_convertible<> trait also covers integer constvals due to normalization

    return { std::move(first), std::move(last) };
}

    //ᅟ
    // Construct a range from an iterator and an extent.
    //
template <typename It, typename ExtentC>
gsl_NODISCARD std::enable_if_t<!std::is_convertible<ExtentC, std::size_t>::value, range<It, It, detail::range_extent_from_constval(ExtentC{ })>>
make_range(It start, ExtentC extentC)
{
    return { std::move(start), extentC };
}


} // namespace makeshift


namespace std
{


    // Implement tuple-like protocol for fixed-size `range<>`.
template <typename It, std::ptrdiff_t Extent> class tuple_size<makeshift::range<It, It, Extent>> : public std::integral_constant<std::size_t, Extent> { };
template <typename It, typename EndIt> class tuple_size<makeshift::range<It, EndIt, -1>>; // not defined
template <std::size_t I, typename It, std::ptrdiff_t Extent> class tuple_element<I, makeshift::range<It, It, Extent>> { public: using type = std::decay_t<decltype(*std::declval<It>())>; };
template <std::size_t I, typename It, typename EndIt> class tuple_element<I, makeshift::range<It, EndIt, -1>>; // not defined


} // namespace std


#endif // INCLUDED_MAKESHIFT_RANGES_HPP_
