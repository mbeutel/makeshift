
#ifndef INCLUDED_MAKESHIFT_BUFFER_HPP_
#define INCLUDED_MAKESHIFT_BUFFER_HPP_


#include <makeshift/utility2.hpp> // for dim2

#include <makeshift/detail/buffer.hpp>


namespace makeshift
{

inline namespace types
{


template <typename T, dim2 Extent = -1, dim2 MaxLocalBufferBytes = -1>
    class buffer
        : public makeshift::detail::buffer_base<T, Extent, MaxLocalBufferBytes, makeshift::detail::determine_memory_location(sizeof(T), Extent, MaxLocalBufferBytes)>
{
private:
    using _base = makeshift::detail::buffer_base<T, Extent, MaxLocalBufferBytes, makeshift::detail::determine_memory_location(sizeof(T), Extent, MaxLocalBufferBytes)>;

public:
    using _base::_base;
};


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_BUFFER_HPP_
