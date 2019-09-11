
#ifndef INCLUDED_MAKESHIFT_MEMORY_HPP_
#define INCLUDED_MAKESHIFT_MEMORY_HPP_


#include <new>         // for align_val_t, bad_alloc
#include <cstddef>     // for size_t, ptrdiff_t
#include <cstdlib>     // for calloc(), free()
#include <cstring>     // for memcpy()
#include <memory>      // for unique_ptr<>, allocator<>, allocator_traits<>, align()
#include <utility>     // for forward<>()
#include <type_traits> // for is_nothrow_default_constructible<>, enable_if<>, is_same<>, remove_cv<>

#include <gsl/gsl-lite.hpp> // for Expects()

#include <makeshift/type_traits.hpp> // for can_instantiate<>
#include <makeshift/macros.hpp>      // for MAKESHIFT_CXXLEVEL, MAKESHIFT_NODISCARD

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
    // Allocator that always returns zero-initialized memory.
    //
template <typename T>
    class zero_init_allocator
{
public:
    using value_type = T;

    constexpr zero_init_allocator(void) noexcept
    {
    }
    template <typename U>
        constexpr zero_init_allocator(zero_init_allocator<U> const&) noexcept
    {
    }

    MAKESHIFT_NODISCARD value_type* allocate(std::size_t n)
    {
        auto mem = std::calloc(n, sizeof(value_type));
        if (mem == nullptr) throw std::bad_alloc{ };
        return static_cast<value_type*>(mem);
    }

    void deallocate(value_type* p, std::size_t) noexcept
    {
        std::free(p);
    }
};

template <typename T, typename U>
    MAKESHIFT_NODISCARD bool operator==(zero_init_allocator<T> const&, zero_init_allocator<U> const&) noexcept
{
    return true;
}
template <class T, class U>
    MAKESHIFT_NODISCARD bool operator!=(zero_init_allocator<T> const& x, zero_init_allocator<U> const& y) noexcept
{
    return !(x == y);
}


    //ᅟ
    // Represents an alignment to use for aligned allocations.
    // In addition to the special values, any positive integer that is a power of 2 may be cast to `alignment`.
    //
enum class alignment : std::ptrdiff_t
{
    page = -2,
    cache_line = -1,
    none = 1
};


    //ᅟ
    // Allocator adaptor that aligns memory allocations by the given size.
    //
template <typename T, alignment Alignment, typename A = std::allocator<T>>
    class aligned_allocator : public A
{
    static_assert(Alignment == alignment::page || Alignment == alignment::cache_line || makeshift::detail::is_alignment_power_of_2(std::ptrdiff_t(Alignment)));

public:
    using A::A;

    template <typename U>
        struct rebind
    {
        using other = aligned_allocator<U, Alignment, typename std::allocator_traits<A>::template rebind_alloc<U>>;
    };

    MAKESHIFT_NODISCARD T* allocate(std::size_t n)
    {
        constexpr std::size_t nExtra = (sizeof(void*) - 1) / sizeof(T) + 1; // = ⌈sizeof(void*) ÷ sizeof(T)⌉

        std::size_t nAlign = makeshift::detail::alignment_in_elements(Alignment, sizeof(T), alignof(T));
        std::size_t nAlloc = (n + nExtra) + nAlign;
        if (nAlloc <= n) throw std::bad_alloc{ }; // overflow

        void* mem = std::allocator_traits<A>::allocate(*this, nAlloc);

        std::size_t allocSize = nAlloc * sizeof(T); // cannot overflow, otherwise upstream `allocate()` would have thrown
        std::size_t size = (n + nExtra) * sizeof(T); // cannot overflow either because `nAlloc >= n + nExtra`
        std::size_t alignment = nAlign * sizeof(T); // cannot overflow, otherwise `alignment_in_elements()` would have failed
        void* alignedMem = mem;
        void* alignResult = std::align(alignment, size, alignedMem, allocSize);
        Expects(alignResult != nullptr); // should not happen

            // Store pointer to actual allocation at end of buffer. Use `memcpy()` so we don't have to worry about alignment.
        std::memcpy(static_cast<char*>(alignResult) + sizeof(T) * n, &mem, sizeof(void*));

        return static_cast<T*>(alignResult);
    }
    void deallocate(T* ptr, std::size_t n) noexcept
    {
        constexpr std::size_t nExtra = (sizeof(void*) - 1) / sizeof(T) + 1; // = ⌈sizeof(void*) ÷ sizeof(T)⌉

            // Retrieve pointer to actual allocation from end of buffer. Use `memcpy()` so we don't have to worry about alignment.
        void* mem;
        std::memcpy(&mem, reinterpret_cast<char*>(ptr) + sizeof(T) * n, sizeof(void*));

        std::size_t nAlign = makeshift::detail::alignment_in_elements(Alignment, sizeof(T), alignof(T));
        std::size_t nAlloc = (n + nExtra) + nAlign; // we already checked for overflow in `allocate()`, so here we assume there is no overflow

        std::allocator_traits<A>::deallocate(*this, static_cast<T*>(mem), nAlloc);
    }
};
#if MAKESHIFT_CXXLEVEL >= 17
template <typename T, alignment Alignment>
    class aligned_allocator<T, Alignment, std::allocator<T>> : public std::allocator<T>
{
    static_assert(Alignment == alignment::page || Alignment == alignment::cache_line || makeshift::detail::is_power_of_2(std::ptrdiff_t(Alignment)));

public:
    using std::allocator<T>::allocator;

    template <typename U>
        struct rebind
    {
        using other = aligned_allocator<U, Alignment, typename std::allocator_traits<std::allocator<T>>::template rebind_alloc<U>>;
    };

    MAKESHIFT_NODISCARD T* allocate(std::size_t n)
    {
            // When adapting the default allocator, make use of `operator new()` with alignment support.
        std::size_t size = n * sizeof(T);
        if (size / sizeof(T) != n) throw std::bad_alloc{ }; // overflow
        return static_cast<T*>(::operator new(size, std::align_val_t(makeshift::detail::alignment_in_bytes(Alignment, alignof(T)))));
    }
    void deallocate(T* ptr, std::size_t /*n*/) noexcept
    {
        ::operator delete(ptr, std::align_val_t(makeshift::detail::alignment_in_bytes(Alignment, alignof(T))));
    }
};
template <typename T, alignment Alignment>
    class aligned_allocator<T, Alignment, default_init_allocator<T, std::allocator<T>>> : public default_init_allocator<T, aligned_allocator<T, Alignment, std::allocator<T>>>
{
public:
    using default_init_allocator<T, aligned_allocator<T, Alignment, std::allocator<T>>>::default_init_allocator;
};
#endif // MAKESHIFT_CXXLEVEL >= 17


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
template <typename T, typename A, typename... ArgsT>
    std::enable_if_t<!can_instantiate_v<makeshift::detail::remove_extent_only_t, T>, std::unique_ptr<T, allocator_deleter<T, A>>>
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
template <typename ArrayT, typename A>
    std::enable_if_t<makeshift::detail::extent_only<ArrayT>::value != 0, std::unique_ptr<ArrayT, allocator_deleter<ArrayT, A>>>
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
template <typename ArrayT, typename A>
    std::enable_if_t<makeshift::detail::extent_only<ArrayT>::value == 0, std::unique_ptr<ArrayT, allocator_deleter<ArrayT, A>>>
    allocate_unique(A alloc, std::size_t size)
{
    using T = std::remove_cv_t<makeshift::detail::remove_extent_only_t<ArrayT>>;
    static_assert(std::is_same<typename std::allocator_traits<A>::value_type, T>::value, "allocator has mismatching value_type");

    T* ptr = makeshift::detail::allocate_array<T>(std::is_nothrow_default_constructible<T>{ }, alloc, size);
    return { ptr, { std::move(alloc), size } };
}



} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_MEMORY_HPP_
