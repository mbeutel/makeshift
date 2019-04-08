
#ifndef INCLUDED_MAKESHIFT_BUFFER_HPP_
#define INCLUDED_MAKESHIFT_BUFFER_HPP_


#include <makeshift/utility2.hpp> // for dim2

#include <makeshift/detail/buffer.hpp>


namespace makeshift
{

inline namespace types
{


template <typename T, typename C, dim2 MaxStaticBufferExtent = -1>
    using buffer = makeshift::detail::buffer<T, makeshift::detail::static_dim<C>(), MaxStaticBufferExtent>;


template <typename T, typename C>
    constexpr buffer<T, C> make_buffer(C size)
{
    return { size };
}

template <typename T, dim2 MaxStaticBufferExtent, typename C>
    constexpr buffer<T, C, MaxStaticBufferExtent> make_buffer(C size)
{
    return { size };
}


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_BUFFER_HPP_
