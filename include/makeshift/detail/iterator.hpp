
#ifndef INCLUDED_MAKESHIFT_DETAIL_ITERATOR_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_ITERATOR_HPP_


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
private:
    constexpr const RangeT& self(void) const noexcept { return static_cast<const RangeT&>(*this); }

public:
    MAKESHIFT_NODISCARD constexpr std::size_t size(void) const noexcept
    {
        return self().end() - self().begin();
    }
    MAKESHIFT_NODISCARD constexpr decltype(auto) operator [](std::ptrdiff_t i) const noexcept
    {
        return self().begin()[i];
    }
};

template <typename RangeT, bool IsBidiIterator> struct range_base_0_;
template <typename RangeT> struct range_base_0_<RangeT, false> { using type = range_base; };
template <typename RangeT> struct range_base_0_<RangeT, true> { using type = random_access_range_base<RangeT>; };
template <typename It, typename RangeT> struct range_base_ : range_base_0_<RangeT, std::is_base_of<std::random_access_iterator_tag, typename std::iterator_traits<It>::iterator_category>::value> { };


template <typename It, std::size_t Size>
    struct fixed_random_access_range
{
    using iterator = It;
    using end_iterator = It;

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


template <typename T> using has_static_size_r = decltype(std::tuple_size<T>::value);
template <typename ContainerT, template <typename, typename> class RangeT, bool IsFixedSize> struct range_by_container_0_;
template <typename ContainerT, template <typename, typename> class RangeT>
    struct range_by_container_0_<ContainerT, RangeT, false>
{
    using It = decltype(std::declval<ContainerT>().begin());
    using EndIt = decltype(std::declval<ContainerT>().end());
    using type = RangeT<It, EndIt>;
};
template <typename ContainerT, template <typename, typename> class RangeT>
    struct range_by_container_0_<ContainerT, RangeT, true>
{
    using It = decltype(std::declval<ContainerT>().begin());
    using type = fixed_random_access_range<It, std::tuple_size<ContainerT>::value>;
};
template <typename ContainerT, template <typename, typename> class RangeT> struct range_by_container_ : range_by_container_0_<ContainerT, RangeT, can_instantiate_v<has_static_size_r, ContainerT>> { };

template <typename It, typename ExtentC, template <typename, typename> class RangeT, bool IsConstval> struct range_by_extent_0_;
template <typename It, typename ExtentC, template <typename, typename> class RangeT> struct range_by_extent_0_<It, ExtentC, RangeT, false> { using type = RangeT<It, It>; };
template <typename It, typename ExtentC, template <typename, typename> class RangeT> struct range_by_extent_0_<It, ExtentC, RangeT, true> { using type = fixed_random_access_range<It, constval_value<ExtentC>>; };
template <typename It, typename ExtentC, template <typename, typename> class RangeT> struct range_by_extent_ : range_by_extent_0_<It, ExtentC, RangeT, is_constval_v<ExtentC>> { };


} // namespace detail

} // namespace makeshift


namespace std
{


    // Specialize `tuple_size<>` for `fixed_random_access_range<>`.
template <typename It, std::size_t Size> struct tuple_size<makeshift::detail::fixed_random_access_range<It, Size>> : public std::integral_constant<std::size_t, Size> { };


} // namespace std


#endif // INCLUDED_MAKESHIFT_DETAIL_ITERATOR_HPP_
