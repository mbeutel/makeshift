
#ifndef INCLUDED_MAKESHIFT_BUFFER_HPP_
#define INCLUDED_MAKESHIFT_BUFFER_HPP_


#include <makeshift/utility2.hpp> // for dim

#include <makeshift/detail/buffer.hpp>


namespace makeshift
{


    // TODO: do we really want this interface, just to permit hybrid usage? Shouldn't we just use dynamic_extent like span<> does?
template <typename T, typename C, dim MaxStaticBufferExtent = -1>
    using buffer = makeshift::detail::buffer<T, makeshift::detail::static_dim<C>(), MaxStaticBufferExtent>;


template <typename T, typename C>
    constexpr buffer<T, C> make_buffer(C size)
{
    return { size };
}

template <typename T, dim MaxStaticBufferExtent, typename C>
    constexpr buffer<T, C, MaxStaticBufferExtent> make_buffer(C size)
{
    return { size };
}


template <typename T, typename C, dim MaxBufferExtent>
    using fixed_buffer = makeshift::detail::fixed_buffer<T, makeshift::detail::static_dim<C>(), MaxBufferExtent>;


template <typename T, typename C>
    constexpr fixed_buffer<T, C, makeshift::detail::constval_value<C>> make_fixed_buffer(C size)
{
    return { size };
}

template <typename T, dim MaxBufferExtent, typename C>
    constexpr fixed_buffer<T, C, MaxBufferExtent> make_fixed_buffer(C size)
{
    return { size };
}


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_BUFFER_HPP_
