
#ifndef INCLUDED_MAKESHIFT_DETAIL_COPY_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_COPY_HPP_


#include <utility> // for copy()


namespace makeshift
{

namespace detail
{


     // copy for insecure iterators
template <class InIt, class OutIt>
    inline OutIt copy(InIt first, InIt last, OutIt dest)
{
#if defined(_MSC_VER) && !defined(_SCL_SECURE_NO_WARNINGS) && !defined(_SCL_SECURE_NO_DEPRECATE)
        // define makeshift::detail::copy() as an unchecked variant of std::copy()
    return std::_Copy_no_deprecate(first, last, dest);
#else
    return std::copy(first, last, dest);
#endif
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_COPY_HPP_
