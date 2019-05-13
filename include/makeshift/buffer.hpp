
#ifndef INCLUDED_MAKESHIFT_BUFFER_HPP_
#define INCLUDED_MAKESHIFT_BUFFER_HPP_


#include <makeshift/utility2.hpp> // for dim

#include <makeshift/detail/buffer.hpp>


namespace makeshift
{


template <typename ArrayT, typename C, dim MaxStaticBufferExtent = -1>
    using buffer = makeshift::detail::buffer<ArrayT, makeshift::detail::static_dim<C>(), MaxStaticBufferExtent>;


template <typename ArrayT, typename C>
    constexpr buffer<ArrayT, C> make_buffer(C size)
{
    return { size };
}

template <typename ArrayT, dim MaxStaticBufferExtent, typename C>
    constexpr buffer<ArrayT, C, MaxStaticBufferExtent> make_buffer(C size)
{
    return { size };
}


template <typename ArrayT, typename C, dim MaxBufferExtent>
    using fixed_buffer = makeshift::detail::fixed_buffer<ArrayT, makeshift::detail::static_dim<C>(), MaxBufferExtent>;


template <typename ArrayT, typename C>
    constexpr fixed_buffer<ArrayT, C, makeshift::detail::constval_value<C>> make_fixed_buffer(C size)
{
    return { size };
}

template <typename ArrayT, dim MaxBufferExtent, typename C>
    constexpr fixed_buffer<ArrayT, C, MaxBufferExtent> make_fixed_buffer(C size)
{
    return { size };
}


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_BUFFER_HPP_
