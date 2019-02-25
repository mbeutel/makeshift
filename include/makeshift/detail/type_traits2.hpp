
#ifndef INCLUDED_MAKESHIFT_DETAIL_TYPE_TRAITS2_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_TYPE_TRAITS2_HPP_


#include <iterator>    // for begin(), end()
#include <type_traits> // for declval<>()


namespace makeshift
{

namespace detail
{

namespace is_iterable_ns
{


using std::begin;
using std::end;


template <typename T> using is_iterable_r = decltype(begin(std::declval<T&>()) != end(std::declval<T&>()));


} // is_iterable_ns

} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_TYPE_TRAITS2_HPP_
