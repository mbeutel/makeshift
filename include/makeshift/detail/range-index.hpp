
#ifndef INCLUDED_MAKESHIFT_DETAIL_RANGE_INDEX_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_RANGE_INDEX_HPP_


#include <cstddef> // for ptrdiff_t

#include <makeshift/type_traits.hpp> // for static_const<>


namespace makeshift
{

namespace detail
{


struct negation_fn
{
    constexpr bool operator ()(bool cond) const
    {
        return !cond;
    }
};

template <typename T>
struct fill_fn
{
    T const& value;

    constexpr T const& operator ()(void) const noexcept
    {
        return value;
    }
};


    //
    // Pass `range_index` to `range_*()`, `array_*()`, `template_*()`, or `tuple_*()` algorithms to have the array element index passed as a functor argument.
    // The argument is of type `index`.
    //ᅟ
    //ᅟ    auto indices = array_transform<3>(
    //ᅟ        [](index i) { return i; },
    //ᅟ        range_index);
    //ᅟ    // returns std::array<index, 3>{ 0, 1, 2 }
    //
struct range_index_t { };

template <std::size_t I>
constexpr std::ptrdiff_t get(range_index_t) noexcept
{
    return I;
}


} // namespace detail


    //
    // Pass `range_index` to `range_*()`, `array_*()`, `template_*()`, or `tuple_*()` algorithms to have the array element index passed as a functor argument.
    // The argument is of type `index`.
    //ᅟ
    //ᅟ    auto indices = array_transform<3>(
    //ᅟ        [](index i) { return i; },
    //ᅟ        range_index);
    //ᅟ    // returns std::array<index, 3>{ 0, 1, 2 }
    //
static constexpr detail::range_index_t const& range_index = static_const<detail::range_index_t>;


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_RANGE_INDEX_HPP_
