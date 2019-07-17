
#ifndef INCLUDED_MAKESHIFT_DETAIL_MEMORY_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_MEMORY_HPP_


#include <cstddef>     // for size_t, ptrdiff_t
#include <type_traits> // for integral_constant<>
#include <memory>      // for allocator_traits<>


namespace makeshift
{


enum class alignment : std::ptrdiff_t;


namespace detail
{


template <typename T> struct remove_extent_only;
template <typename T> struct remove_extent_only<T[]> { using type = T; };
template <typename T, std::ptrdiff_t N> struct remove_extent_only<T[N]> { using type = T; };
template <typename T> using remove_extent_only_t = typename remove_extent_only<T>::type;

template <typename T> struct extent_only;
template <typename T> struct extent_only<T[]> : std::integral_constant<std::ptrdiff_t, 0> { };
template <typename T, std::ptrdiff_t N> struct extent_only<T[N]> : std::integral_constant<std::ptrdiff_t, N> { };
template <typename T> constexpr std::ptrdiff_t extent_only_v = extent_only<T>::value;


template <typename T, typename A, typename... ArgsT>
    T* allocate(std::true_type /*isNothrowConstructible*/, A& alloc, ArgsT&&... args)
{
    T* ptr = std::allocator_traits<A>::allocate(alloc, 1);
    std::allocator_traits<A>::construct(alloc, ptr, std::forward<ArgsT>(args)...);
    return ptr;
}
template <typename T, typename A, typename... ArgsT>
    T* allocate(std::false_type /*isNothrowConstructible*/, A& alloc, ArgsT&&... args)
{
    T* ptr = std::allocator_traits<A>::allocate(alloc, 1);
    try
    {
        std::allocator_traits<A>::construct(alloc, ptr, std::forward<ArgsT>(args)...);
    }
    catch (...)
    {
        std::allocator_traits<A>::deallocate(alloc, ptr, 1);
        throw;
    }
    return ptr;
}

template <typename T, typename A, typename SizeC>
    T* allocate_array(std::true_type /*isNothrowDefaultConstructible*/, A& alloc, SizeC sizeC)
{
    T* ptr = std::allocator_traits<A>::allocate(alloc, std::size_t(sizeC));
    for (std::ptrdiff_t i = 0; i != std::ptrdiff_t(sizeC); ++i)
    {
        std::allocator_traits<A>::construct(alloc, &ptr[i]);
    }
    return ptr;
}
template <typename T, typename A, typename SizeC>
    T* allocate_array(std::false_type /*isNothrowDefaultConstructible*/, A& alloc, SizeC sizeC)
{
    T* ptr = std::allocator_traits<A>::allocate(alloc, std::size_t(sizeC));
    std::ptrdiff_t i = 0;
    try
    {
        for (; i != std::ptrdiff_t(sizeC); ++i)
        {
            std::allocator_traits<A>::construct(alloc, &ptr[i]);
        }
    }
    catch (...)
    {
            // Revert by destroying all already-constructed items, deallocating, and then re-throwing the exception.
        for (--i; i >= 0; --i)
        {
            std::allocator_traits<A>::destroy(alloc, &ptr[i]);
        }
        std::allocator_traits<A>::deallocate(alloc, ptr, std::size_t(sizeC));
        throw;
    }
    return ptr;
}


MAKESHIFT_PUBLIC std::size_t alignment_in_bytes(alignment a, std::size_t alignOfT) noexcept;
//std::size_t augment_size_for_alignment(std::size_t n, std::size_t s, alignment a);


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_MEMORY_HPP_
