
#ifndef INCLUDED_MAKESHIFT_DETAIL_RANGE_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_RANGE_HPP_


#include <cstddef>     // for size_t, ptrdiff_t
#include <utility>     // for tuple_size<>, integer_sequence<>
#include <iterator>    // for iterator_traits<>, random_access_iterator_tag
#include <type_traits> // for is_base_of<>

#include <gsl/gsl_assert> // for Expects()

#include <makeshift/constval.hpp>     // for is_constval<>
#include <makeshift/type_traits2.hpp> // for can_instantiate<>
#include <makeshift/version.hpp>      // for MAKESHIFT_NODISCARD


namespace makeshift
{

namespace detail
{


struct range_base { };

template <typename RangeT>
    struct random_access_range_base : range_base
{
    MAKESHIFT_NODISCARD constexpr std::size_t size(void) const noexcept
    {
        return static_cast<const RangeT&>(*this).end() - static_cast<const RangeT&>(*this).begin();
    }
    MAKESHIFT_NODISCARD constexpr decltype(auto) operator [](std::ptrdiff_t i) const noexcept
    {
        return static_cast<const RangeT&>(*this).begin()[i];
    }
};

template <typename RangeT, bool IsRandomAccessIterator> struct range_base_0_;
template <typename RangeT> struct range_base_0_<RangeT, false> { using type = range_base; };
template <typename RangeT> struct range_base_0_<RangeT, true> { using type = random_access_range_base<RangeT>; };
template <typename It, typename RangeT> struct range_base_ : range_base_0_<RangeT, std::is_base_of<std::random_access_iterator_tag, typename std::iterator_traits<It>::iterator_category>::value> { };


template <typename It, std::size_t Size>
    struct fixed_random_access_range
{
    It first;

    constexpr fixed_random_access_range(It _first, It _last)
        : first(std::move(_first))
    {
        Expects(first + Size == _last);
    }

    MAKESHIFT_NODISCARD constexpr const It& begin(void) const noexcept { return first; }
    MAKESHIFT_NODISCARD constexpr const It& end(void) const noexcept { return first + Size; }
    MAKESHIFT_NODISCARD constexpr std::size_t size(void) const noexcept
    {
        return Size;
    }
    MAKESHIFT_NODISCARD constexpr decltype(auto) operator [](std::ptrdiff_t i) const noexcept
    {
        return first[i];
    }
};


template <typename It, typename ExtentC, template <typename, typename> class RangeT, bool IsConstval> struct range_by_extent_0_;
template <typename It, typename ExtentC, template <typename, typename> class RangeT> struct range_by_extent_0_<It, ExtentC, RangeT, false> { using type = RangeT<It, It>; };
template <typename It, typename ExtentC, template <typename, typename> class RangeT> struct range_by_extent_0_<It, ExtentC, RangeT, true> { using type = fixed_random_access_range<It, constval_value<ExtentC>>; };
template <typename It, typename ExtentC, template <typename, typename> class RangeT> struct range_by_extent_ : range_by_extent_0_<It, ExtentC, RangeT, is_constval_v<ExtentC>> { };


    // Implement tuple-like protocol for `fixed_random_access_range<>`.
template <std::size_t I, typename It, std::size_t Size>
    MAKESHIFT_NODISCARD constexpr decltype(auto) get(fixed_random_access_range<It, Size>& range) noexcept
{
    static_assert(I < Size, "index out of range");
    return range[I];
}
template <std::size_t I, typename It, std::size_t Size>
    MAKESHIFT_NODISCARD constexpr decltype(auto) get(const fixed_random_access_range<It, Size>& range) noexcept
{
    static_assert(I < Size, "index out of range");
    return range[I];
}


} // namespace detail

} // namespace makeshift


namespace std
{


    // Implement tuple-like protocol for `fixed_random_access_range<>`.
template <typename It, std::size_t Size> struct tuple_size<makeshift::detail::fixed_random_access_range<It, Size>> : public std::integral_constant<std::size_t, Size> { };
template <std::size_t I, typename It, std::size_t Size> struct tuple_element<I, makeshift::detail::fixed_random_access_range<It, Size>> { using type = std::decay_t<decltype(*std::declval<It>())>; };


} // namespace std


#endif // INCLUDED_MAKESHIFT_DETAIL_RANGE_HPP_
