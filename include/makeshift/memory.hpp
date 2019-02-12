
#ifndef INCLUDED_MAKESHIFT_MEMORY_HPP_
#define INCLUDED_MAKESHIFT_MEMORY_HPP_


#include <memory> // for allocator<>, allocator_traits<>


namespace makeshift
{


    // Allocator adaptor that interposes construct() calls to convert value initialization into default initialization.
    // cf. https://stackoverflow.com/a/21028912
template <typename T, typename A = std::allocator<T>>
class default_init_allocator : public A
{
public:
    using A::A;

    template <typename U>
        void construct(U* ptr)
        noexcept(std::is_nothrow_default_constructible<U>::value)
    {
        ::new(static_cast<void*>(ptr)) U;
    }
    template <typename U, typename...ArgsT>
        void construct(U* ptr, ArgsT&&... args)
    {
        std::allocator_traits<A>::construct(static_cast<A&>(*this), ptr, std::forward<ArgsT>(args)...);
    }
};


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_MEMORY_HPP_
