
#ifndef INCLUDED_MAKESHIFT_DETAIL_RANGES_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_RANGES_HPP_


#include <cstddef>      // for size_t, ptrdiff_t
#include <utility>      // for move(), integer_sequence<>
#include <iterator>     // for iterator_traits<>, random_access_iterator_tag
#include <type_traits>  // for is_base_of<>, declval<>(), integral_constant<>

#include <gsl-lite/gsl-lite.hpp>  // for index, gsl_Expects()

#include <makeshift/iterator.hpp>  // for index_iterator

#include <makeshift/detail/macros.hpp>  // for MAKESHIFT_DETAIL_FORCEINLINE


namespace makeshift {

namespace gsl = ::gsl_lite;

namespace detail {


template <typename It, typename EndIt>
class general_range_base
{
private:
    It first_;
    EndIt last_;

public:
    constexpr general_range_base(It first, EndIt last)
        : first_(std::move(first)), last_(std::move(last))
    {
    }

    [[nodiscard]] constexpr It const& begin(void) const noexcept { return first_; }
    [[nodiscard]] constexpr EndIt const& end(void) const noexcept { return last_; }
};
template <typename It, typename EndIt>
class random_access_range_base
{
private:
    It first_;
    EndIt last_;

public:
    constexpr random_access_range_base(It first, EndIt last)
        : first_(std::move(first)), last_(std::move(last))
    {
    }

    [[nodiscard]] constexpr It const& begin(void) const noexcept { return first_; }
    [[nodiscard]] constexpr EndIt const& end(void) const noexcept { return last_; }
    [[nodiscard]] constexpr std::size_t size(void) const noexcept
    {
        return last_ - first_;
    }
    [[nodiscard]] constexpr decltype(auto) operator [](std::size_t i) const noexcept
    {
        gsl_Expects(i < last_ - first_);
        return first_[i];
    }
};
template <typename It, std::ptrdiff_t Extent>
class static_random_access_range_base
{
    static constexpr std::ptrdiff_t extent_ = Extent;

    static_assert(Extent >= 0, "range extent must be non-negative");

private:
    It first_;

public:
    constexpr static_random_access_range_base(It first, It /*last*/)
        : first_(std::move(first))
    {
    }

    [[nodiscard]] constexpr It const& begin(void) const noexcept { return first_; }
    [[nodiscard]] constexpr It end(void) const noexcept { return first_ + Extent; }
    [[nodiscard]] constexpr std::size_t size(void) const noexcept
    {
        return Extent;
    }
    [[nodiscard]] constexpr decltype(auto) operator [](std::size_t i) const noexcept
    {
        gsl_Expects(i < std::size_t(Extent));
        return first_[i];
    }
};

template <typename It, typename EndIt, typename IteratorTagT, std::ptrdiff_t Extent>
class range_base;
template <typename It, typename EndIt, typename IteratorTagT>
class range_base<It, EndIt, IteratorTagT, -1> : public general_range_base<It, EndIt>
{
    using base = general_range_base<It, EndIt>;

public:
    using base::base;
};
template <typename It, typename EndIt>
class range_base<It, EndIt, std::random_access_iterator_tag, -1> : public random_access_range_base<It, EndIt>
{
    using base = random_access_range_base<It, EndIt>;

public:
    using base::base;
};
template <typename It>
class range_base<It, It, std::random_access_iterator_tag, -1> : public random_access_range_base<It, It>
{
    using base = random_access_range_base<It, It>;

public:
    using base::base;
};
template <typename It, std::ptrdiff_t Extent>
class range_base<It, It, std::random_access_iterator_tag, Extent> : public static_random_access_range_base<It, Extent>
{
    using base = static_random_access_range_base<It, Extent>;

public:
    using base::base;
};

template <typename It> using range_iterator_tag = std::conditional_t<
    std::is_base_of<std::random_access_iterator_tag, typename std::iterator_traits<It>::iterator_category>::value, // TODO: in C++20 we should use concepts to also permit non-legacy random-access iterators
    std::random_access_iterator_tag,
    std::input_iterator_tag>;

template <typename ExtentC>
constexpr std::ptrdiff_t range_extent_from_constval(ExtentC)
{
    return -1;
}
template <typename T, T V>
constexpr std::ptrdiff_t range_extent_from_constval(std::integral_constant<T, V>)
{
    return V;
}

template <typename T>
constexpr void check_buffer_extents(std::true_type /*dynamicExtent*/, std::size_t expectedExtent, std::size_t actualExtent)
{
    gsl_Expects(expectedExtent == actualExtent);
}
template <typename T>
constexpr void check_buffer_extents(std::false_type /*dynamicExtent*/, std::size_t /*expectedExtent*/, std::size_t /*actualExtent*/)
{
}

    // Implement tuple-like protocol for `range<>`.
template <class T>
constexpr T const& as_const(T& t) noexcept
{
    return t;
}
template <std::size_t I, typename It, std::ptrdiff_t Extent>
[[nodiscard]] constexpr std::enable_if_t<(Extent >= 0), decltype(*std::declval<It>())>
get(range_base<It, It, std::random_access_iterator_tag, Extent>& range) noexcept
{
    static_assert(I < std::size_t(Extent), "index out of range");
    return range[I];
}
template <std::size_t I, typename It, std::ptrdiff_t Extent>
[[nodiscard]] constexpr std::enable_if_t<(Extent >= 0), decltype(detail::as_const(*std::declval<It>()))>
get(range_base<It, It, std::random_access_iterator_tag, Extent> const& range) noexcept
{
    static_assert(I < std::size_t(Extent), "index out of range");
    return range[I];
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_RANGES_HPP_
