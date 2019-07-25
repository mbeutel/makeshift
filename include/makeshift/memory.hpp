
#ifndef INCLUDED_MAKESHIFT_MEMORY_HPP_
#define INCLUDED_MAKESHIFT_MEMORY_HPP_


#include <new>         // for align_val_t
#include <cstddef>     // for size_t, ptrdiff_t
#include <memory>      // for unique_ptr<>, allocator<>, allocator_traits<>
#include <utility>     // for forward<>()
#include <numeric>     // for lcm()
#include <type_traits> // for is_nothrow_default_constructible<>, enable_if<>, is_same<>, remove_cv<>

#include <makeshift/type_traits2.hpp> // for can_instantiate<>
#include <makeshift/version.hpp>      // for MAKESHIFT_NODISCARD

#include <makeshift/detail/memory.hpp>


namespace makeshift
{


    //ᅟ
    // Allocator adaptor that interposes construct() calls to convert value initialization into default initialization.
    // cf. https://stackoverflow.com/a/21028912
    //
template <typename T, typename A = std::allocator<T>>
    class default_init_allocator : public A
{
public:
    using A::A;

    template <typename U>
        struct rebind
    {
        using other = default_init_allocator<U, typename std::allocator_traits<A>::template rebind_alloc<U>>;
    };

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


    //ᅟ
    // Represents an alignment to use for aligned allocations.
    // In addition to the special values, any positive integer may be casted to `alignment`.
    //
enum class alignment : std::ptrdiff_t
{
    page = -2,
    cache_line = -1,
    none = 0
};


    //ᅟ
    // Allocator adaptor that aligns memory allocations by the given size.
    //
template <typename T, alignment Alignment, typename A = std::allocator<T>>
    class aligned_allocator; // currently not implemented for any allocator other than `std::allocator<T>`

template <typename T, alignment Alignment>
    class aligned_allocator<T, Alignment, std::allocator<T>> : public std::allocator<T>
{
public:
    using std::allocator<T>::allocator;

    template <typename U>
        struct rebind
    {
        using other = aligned_allocator<U, Alignment, typename std::allocator_traits<std::allocator<T>>::template rebind_alloc<U>>;
    };

    MAKESHIFT_NODISCARD T* allocate(std::size_t n)
    {
        return static_cast<T*>(::operator new(sizeof(T) * n, std::align_val_t(makeshift::detail::alignment_in_bytes(Alignment, alignof(T)))));
    }
    void deallocate(T* ptr, std::size_t /*n*/)
    {
        ::operator delete(ptr, std::align_val_t(makeshift::detail::alignment_in_bytes(Alignment, alignof(T))));
    }
};


    //ᅟ
    // Deleter for `std::unique_ptr<>` which supports custom allocators.
    //ᅟ
    //ᅟ    auto p1 = allocate_unique<float>(MyAllocator<float>{ }, 3.14159f); // returns `std::unique_ptr<float, allocator_deleter<float, MyAllocator<float>>>`
    //ᅟ    auto p2 = allocate_unique<float[]>(MyAllocator<float>{ }, 42); // returns `std::unique_ptr<float[], allocator_deleter<float[], MyAllocator<float>>>`
    //ᅟ    auto p3 = allocate_unique<float[42]>(MyAllocator<float>{ }); // returns `std::unique_ptr<float[42], allocator_deleter<float[42], MyAllocator<float>>>`
    //
template <typename T, typename A>
    class allocator_deleter
{
private:
    A alloc_;

public:
    allocator_deleter(const A& _alloc)
        : alloc_(_alloc)
    {
    }
    void operator ()(T* ptr)
    {
        std::allocator_traits<A>::destroy(alloc_, ptr);
        std::allocator_traits<A>::deallocate(alloc_, ptr, 1);
    }
};

template <typename T, typename A>
    class allocator_deleter<T[], A>
{
private:
    A alloc_;
    std::size_t size_;

public:
    allocator_deleter(const A& _alloc, std::size_t _size)
        : alloc_(_alloc), size_(_size)
    {
    }
    void operator ()(T* ptr)
    {
        for (std::ptrdiff_t i = 0, n = std::ptrdiff_t(size_); i != n; ++i)
        {
            std::allocator_traits<A>::destroy(alloc_, &ptr[i]);
        }
        std::allocator_traits<A>::deallocate(alloc_, ptr, size_);
    }
};

template <typename T, std::ptrdiff_t N, typename A>
    class allocator_deleter<T[N], A>
{
private:
    A alloc_;

public:
    allocator_deleter(const A& _alloc)
        : alloc_(_alloc)
    {
    }
    void operator ()(T* ptr)
    {
        for (std::ptrdiff_t i = 0; i != N; ++i)
        {
            std::allocator_traits<A>::destroy(alloc_, &ptr[i]);
        }
        std::allocator_traits<A>::deallocate(alloc_, ptr, N);
    }
};


    //ᅟ
    // Allocates an object of type `T` with the given allocator, constructs it with the given arguments and returns a `std::unique_ptr<>` to the object.
    //ᅟ
    //ᅟ    auto p = allocate_unique<float>(MyAllocator<float>{ }, 3.14159f);
    //ᅟ    // returns `std::unique_ptr<float, allocator_deleter<float, MyAllocator<float>>>`
    //
template <typename T, typename A, typename... ArgsT,
          std::enable_if_t<!can_instantiate_v<makeshift::detail::remove_extent_only_t, T>, int> = 0>
    std::unique_ptr<T, allocator_deleter<T, A>>
    allocate_unique(A alloc, ArgsT&&... args)
{
    using NCVT = std::remove_cv_t<T>;
    static_assert(std::is_same<typename std::allocator_traits<A>::value_type, NCVT>::value, "allocator has mismatching value_type");

    NCVT* ptr = makeshift::detail::allocate<NCVT>(std::is_nothrow_constructible<NCVT, ArgsT...>{ }, alloc, std::forward<ArgsT>(args)...);
    return { ptr, { std::move(alloc) } };
}

    //ᅟ
    // Allocates a fixed-size array of type `ArrayT` with the given allocator, default-constructs the elements and returns a `std::unique_ptr<>` to the array.
    //ᅟ
    //ᅟ    auto p = allocate_unique<float[42]>(MyAllocator<float>{ });
    //ᅟ    // returns `std::unique_ptr<float[42], allocator_deleter<float[42], MyAllocator<float>>>`
    //
template <typename ArrayT, typename A,
          std::enable_if_t<makeshift::detail::extent_only<ArrayT>::value != 0, int> = 0>
    std::unique_ptr<ArrayT, allocator_deleter<ArrayT, A>>
    allocate_unique(A alloc)
{
    using T = std::remove_cv_t<makeshift::detail::remove_extent_only_t<ArrayT>>;
    static_assert(std::is_same<typename std::allocator_traits<A>::value_type, T>::value, "allocator has mismatching value_type");

    T* ptr = makeshift::detail::allocate_array<T>(std::is_nothrow_default_constructible<T>{ }, alloc, makeshift::detail::extent_only<ArrayT>{ });
    return { ptr, { std::move(alloc) } };
}

    //ᅟ
    // Allocates an array of type `ArrayT` with the given allocator, default-constructs the elements and returns a `std::unique_ptr<>` to the array.
    //ᅟ
    //ᅟ    auto p = allocate_unique<float[]>(MyAllocator<float>{ }, 42);
    //ᅟ    // returns `std::unique_ptr<float[], allocator_deleter<float[], MyAllocator<float>>>`
    //
template <typename ArrayT, typename A,
          std::enable_if_t<makeshift::detail::extent_only<ArrayT>::value == 0, int> = 0>
    std::unique_ptr<ArrayT, allocator_deleter<ArrayT, A>>
    allocate_unique(A alloc, std::size_t size)
{
    using T = std::remove_cv_t<makeshift::detail::remove_extent_only_t<ArrayT>>;
    static_assert(std::is_same<typename std::allocator_traits<A>::value_type, T>::value, "allocator has mismatching value_type");

    T* ptr = makeshift::detail::allocate_array<T>(std::is_nothrow_default_constructible<T>{ }, alloc, size);
    return { ptr, { std::move(alloc), size } };
}



} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_MEMORY_HPP_
