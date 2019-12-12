
#ifndef INCLUDED_MAKESHIFT_DETAIL_RANGE_INDEX_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_RANGE_INDEX_HPP_


#include <cstddef> // for ptrdiff_t

#include <makeshift/type_traits.hpp> // for static_const<>


namespace makeshift
{

namespace detail
{


struct identity_transform_t
{
    template <typename T>
    constexpr auto operator ()(T&& arg) const
    {
        return std::forward<T>(arg);
    }
};

struct all_of_pred
{
    constexpr bool operator ()(bool cond) const
    {
        return cond;
    }
};
struct none_of_pred
{
    constexpr bool operator ()(bool cond) const
    {
        return !cond;
    }
};


    //ᅟ
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


    //ᅟ
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
