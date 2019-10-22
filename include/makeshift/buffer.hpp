
#ifndef INCLUDED_MAKESHIFT_BUFFER_HPP_
#define INCLUDED_MAKESHIFT_BUFFER_HPP_


#include <array>
#include <utility>     // for tuple_size<>, tuple_element<>
#include <iterator>    // for move_iterator<>
#include <algorithm>   // for copy()
#include <type_traits> // for is_convertible<>

#include <gsl/gsl-lite.hpp> // for Expects()

#include <makeshift/detail/buffer.hpp>


namespace makeshift
{


constexpr std::ptrdiff_t dynamic_extent = -1;


    //ᅟ
    // Array-like class with configurable small-buffer optimization.
    // The buffer stores `Extent` elements. If `Extent == dynamic_extent`, the number of elements is determined at runtime.
    // The buffer allocates the elements in-place if the number of elements is smaller than `MaxStaticBufferExtent`, and on the heap otherwise.
    // If `MaxStaticBufferExtent == -1`, the elements are allocated in-place if `Extent != dynamic_extent`, and on the heap otherwise.
    //ᅟ
    //ᅟ    std::size_t numElements = ...;
    //ᅟ    auto buf = make_buffer<float>(numElements); // returns `buffer<float, dynamic_extent>`; allocates on the heap
    //ᅟ    auto buf = make_buffer<float, 16>(numElements); // returns `buffer<float, dynamic_extent, 16>`; allocates on the heap if `numElements > 16`
    //ᅟ    auto numElementsC = std::integral_constant<std::size_t, N>{ };
    //ᅟ    auto buf = make_buffer<float>(numElementsC); // returns `buffer<float, N>`; allocates in-place
    //ᅟ    auto buf = make_buffer<float, 16>(numElementsC); // returns `buffer<float, N, 16>`; allocates on the heap if `N > 16`
    //
template <typename T, std::ptrdiff_t Extent = dynamic_extent, std::ptrdiff_t MaxStaticBufferExtent = -1>
    class buffer
        : public makeshift::detail::buffer_base<T, Extent, MaxStaticBufferExtent, makeshift::detail::determine_memory_location(Extent, MaxStaticBufferExtent)>
{
private:
    using base_ = makeshift::detail::buffer_base<T, Extent, MaxStaticBufferExtent, makeshift::detail::determine_memory_location(Extent, MaxStaticBufferExtent)>;

public:
    template <typename ExtentC,
              std::enable_if_t<std::is_convertible<ExtentC, std::size_t>::value, int> = 0>
        explicit constexpr buffer(ExtentC extent)
            : base_(extent)
    {
        constexpr std::ptrdiff_t rhsExtent = makeshift::detail::buffer_extent_from_constval(ExtentC{ });
        static_assert(Extent == dynamic_extent || rhsExtent == -1 || Extent == rhsExtent, "static extents must match");
        makeshift::detail::check_buffer_extents(std::integral_constant<bool, Extent == dynamic_extent>{ }, Extent, extent);
    }
    template <std::ptrdiff_t RExtent, typename U>
        constexpr buffer(U (&&array)[RExtent])
            : base_(RExtent)
    {
        static_assert(Extent == dynamic_extent || RExtent == Extent, "array extent does not match");
        static_assert(std::is_convertible<U, T>::value, "incompatible array element types");
        std::copy(std::make_move_iterator(array), std::make_move_iterator(array + RExtent), this->begin());
    }
    template <std::ptrdiff_t RExtent, typename U>
        constexpr buffer& operator =(U (&&array)[RExtent])
    {
        static_assert(Extent == dynamic_extent || RExtent == Extent, "array extent does not match");
        static_assert(std::is_convertible<U, T>::value, "incompatible array element types");
        Expects(RExtent == this->size());
        std::copy(std::make_move_iterator(array), std::make_move_iterator(array + RExtent), this->begin());
        return *this;
    }
};
//#if MAKESHIFT_CXX >= 17
//template <std::ptrdiff_t Extent, typename T>
//    buffer(T (&&)[Extent]) -> buffer<T, Extent>;
//#endif // MAKESHIFT_CXX >= 17

    //ᅟ
    // Construct array-like class with configurable small-buffer optimization.
    // The number of elements is allocated in-place if `size` is a constval, and on the heap otherwise.
    //ᅟ
    //ᅟ    std::size_t numElements = ...;
    //ᅟ    auto buf = make_buffer<float>(numElements); // returns `buffer<float, dynamic_extent>`; allocates on the heap
    //ᅟ    auto numElementsC = std::integral_constant<std::size_t, N>{ };
    //ᅟ    auto buf = make_buffer<float>(numElementsC); // returns `buffer<float, N>`; allocates in-place
    //
template <typename T,
          typename C>
    MAKESHIFT_NODISCARD constexpr
    buffer<T, makeshift::detail::buffer_extent_from_constval(C{ })>
    make_buffer(C size)
{
    return buffer<T, makeshift::detail::buffer_extent_from_constval(C{ })>(size);
}

    //ᅟ
    // Construct array-like class with configurable small-buffer optimization.
    // If `MaxStaticBufferExtent >= 0`, the elements are allocated in-place if `size <= MaxStaticBufferExtent`, and on the heap otherwise.
    // If `MaxStaticBufferExtent == -1`, the elements are allocated in-place if `size` is a constval, and on the heap otherwise.
    //ᅟ
    //ᅟ    std::size_t numElements = ...;
    //ᅟ    auto buf = make_buffer<float, 16>(numElements); // returns `buffer<float, dynamic_extent, 16>`; allocates on the heap if `numElements > 16`
    //ᅟ    auto numElementsC = std::integral_constant<std::size_t, N>{ };
    //ᅟ    auto buf = make_buffer<float, 16>(numElementsC); // returns `buffer<float, N, 16>`; allocates on the heap if `N > 16`
    //
template <typename T, std::ptrdiff_t MaxStaticBufferExtent,
          typename C>
    MAKESHIFT_NODISCARD constexpr
    buffer<T, makeshift::detail::buffer_extent_from_constval(C{ }), MaxStaticBufferExtent>
    make_buffer(C size)
{
    return buffer<T, makeshift::detail::buffer_extent_from_constval(C{ }), MaxStaticBufferExtent>(size);
}


    //ᅟ
    // Array-like class with in-place storage.
    // The buffer stores `Extent` elements. If `Extent == dynamic_extent`, the number of elements is determined at runtime.
    // Buffer construction fails if the number of elements exceeds `MaxStaticBufferExtent`.
    //ᅟ
    //ᅟ    std::size_t numElements = ...;
    //ᅟ    auto buf = make_fixed_buffer<float, 16>(numElements); // returns `fixed_buffer<float, dynamic_extent, 16>`; asserts that `numElements <= 16`
    //ᅟ    auto numElementsC = std::integral_constant<std::size_t, N>{ };
    //ᅟ    auto buf = make_fixed_buffer<float>(numElementsC); // returns `fixed_buffer<float, N, N>`
    //ᅟ    auto buf = make_fixed_buffer<float, 16>(numElementsC); // returns `fixed_buffer<float, N, 16>`; asserts at compile-time that `N <= 16`
    //
template <typename T, std::ptrdiff_t Extent, std::ptrdiff_t MaxBufferExtent>
    class fixed_buffer
        : public makeshift::detail::static_buffer_base<T, Extent, MaxBufferExtent>
{
    static_assert(MaxBufferExtent >= 0, "invalid maximal buffer extent");
    static_assert(Extent <= MaxBufferExtent, "size exceeds buffer extent");

private:
    using base_ = makeshift::detail::static_buffer_base<T, Extent, MaxBufferExtent>;

public:
    template <typename ExtentC,
              std::enable_if_t<std::is_convertible<ExtentC, std::size_t>::value, int> = 0>
        explicit constexpr fixed_buffer(ExtentC extent)
            : base_(extent)
    {
        constexpr std::ptrdiff_t rhsExtent = makeshift::detail::buffer_extent_from_constval(ExtentC{ });
        static_assert(Extent == dynamic_extent || rhsExtent == -1 || Extent == rhsExtent, "static extents must match");
        static_assert(rhsExtent == -1 || rhsExtent <= MaxBufferExtent, "size exceeds buffer extent");
        makeshift::detail::check_fixed_buffer_extents(std::integral_constant<bool, Extent == dynamic_extent>{ }, Extent, extent, MaxBufferExtent);
    }
    template <std::ptrdiff_t RExtent, typename U>
        constexpr fixed_buffer(U (&&array)[RExtent])
            : base_(RExtent)
    {
        static_assert(Extent == dynamic_extent || RExtent == Extent, "array extent does not match");
        static_assert(std::is_convertible<U, T>::value, "incompatible array element types");
        std::copy(std::make_move_iterator(array), std::make_move_iterator(array + RExtent), this->begin());
    }
    template <std::ptrdiff_t RExtent, typename U>
        constexpr fixed_buffer& operator =(U (&&array)[RExtent])
    {
        static_assert(Extent == dynamic_extent || RExtent == Extent, "array extent does not match");
        static_assert(std::is_convertible<U, T>::value, "incompatible array element types");
        Expects(RExtent == this->size());
        std::copy(std::make_move_iterator(array), std::make_move_iterator(array + RExtent), this->begin());
        return *this;
    }
};
//#if MAKESHIFT_CXX >= 17
//template <std::ptrdiff_t Extent, typename T>
//    buffer(T (&&)[Extent]) -> buffer<T, Extent>;
//#endif // MAKESHIFT_CXX >= 17

    //ᅟ
    // Array-like class with in-place storage of `Size` elements.
    //ᅟ
    //ᅟ    auto numElementsC = std::integral_constant<std::size_t, N>{ };
    //ᅟ    auto buf = make_fixed_buffer<float>(numElementsC); // returns `fixed_buffer<float, N, N>`
    //
template <typename T, typename SizeT, SizeT Size>
    MAKESHIFT_NODISCARD constexpr
    fixed_buffer<T, Size, Size>
    make_fixed_buffer(std::integral_constant<SizeT, Size>)
{
    return fixed_buffer<T, Size, Size>(std::integral_constant<SizeT, Size>{ });
}

    //ᅟ
    // Array-like class with in-place storage of `size` elements.
    // Asserts that `size <= MaxBufferExtent`.
    //ᅟ
    //ᅟ    std::size_t numElements = ...;
    //ᅟ    auto buf = make_fixed_buffer<float, 16>(numElements); // returns `fixed_buffer<float, dynamic_extent, 16>`; asserts that `numElements <= 16`
    //ᅟ    auto numElementsC = std::integral_constant<std::size_t, N>{ };
    //ᅟ    auto buf = make_fixed_buffer<float, 16>(numElementsC); // returns `fixed_buffer<float, N, 16>`; asserts at compile-time that `N <= 16`
    //
template <typename T, std::ptrdiff_t MaxBufferExtent,
          typename C>
    MAKESHIFT_NODISCARD constexpr
    fixed_buffer<T, makeshift::detail::buffer_extent_from_constval(C{ }), MaxBufferExtent>
    make_fixed_buffer(C size)
{
    return fixed_buffer<T, makeshift::detail::buffer_extent_from_constval(C{ }), MaxBufferExtent>(size);
}


    // Implement tuple-like protocol for `buffer<>`.
template <std::size_t I, typename T, std::ptrdiff_t Extent, std::ptrdiff_t MaxStaticBufferExtent>
    MAKESHIFT_NODISCARD constexpr std::enable_if_t<Extent != dynamic_extent, T&>
    get(buffer<T, Extent, MaxStaticBufferExtent>& buffer) noexcept
{
    static_assert(I < Extent, "index out of range");
    return buffer[I];
}
template <std::size_t I, typename T, std::ptrdiff_t Extent, std::ptrdiff_t MaxStaticBufferExtent>
    MAKESHIFT_NODISCARD constexpr std::enable_if_t<Extent != dynamic_extent, T const&>
    get(buffer<T, Extent, MaxStaticBufferExtent> const& buffer) noexcept
{
    static_assert(I < Extent, "index out of range");
    return buffer[I];
}

    // Implement tuple-like protocol for `fixed_buffer<>`.
template <std::size_t I, typename T, std::ptrdiff_t Extent, std::ptrdiff_t MaxBufferExtent>
    MAKESHIFT_NODISCARD constexpr std::enable_if_t<Extent != dynamic_extent, T&>
    get(fixed_buffer<T, Extent, MaxBufferExtent>& buffer) noexcept
{
    static_assert(I < Extent, "index out of range");
    return buffer[I];
}
template <std::size_t I, typename T, std::ptrdiff_t Extent, std::ptrdiff_t MaxBufferExtent>
    MAKESHIFT_NODISCARD constexpr std::enable_if_t<Extent != dynamic_extent, T const&>
    get(fixed_buffer<T, Extent, MaxBufferExtent> const& buffer) noexcept
{
    static_assert(I < Extent, "index out of range");
    return buffer[I];
}


} // namespace makeshift


namespace std
{


    // Implement tuple-like protocol for `buffer<>`.
template <typename T, std::ptrdiff_t Extent, std::ptrdiff_t MaxStaticBufferExtent> class tuple_size<makeshift::buffer<T, Extent, MaxStaticBufferExtent>> : public std::integral_constant<std::size_t, Extent> { };
template <typename T, std::ptrdiff_t MaxStaticBufferExtent> class tuple_size<makeshift::buffer<T, -1, MaxStaticBufferExtent>>; // undefined
template <std::size_t I, typename T, std::ptrdiff_t Extent, std::ptrdiff_t MaxStaticBufferExtent> class tuple_element<I, makeshift::buffer<T, Extent, MaxStaticBufferExtent>> { public: using type = T; };

    // Implement tuple-like protocol for `fixed_buffer<>`.
template <typename T, std::ptrdiff_t Extent, std::ptrdiff_t MaxBufferExtent> class tuple_size<makeshift::fixed_buffer<T, Extent, MaxBufferExtent>> : public std::integral_constant<std::size_t, Extent> { };
template <typename T, std::ptrdiff_t MaxBufferExtent> class tuple_size<makeshift::fixed_buffer<T, -1, MaxBufferExtent>>; // undefined
template <std::size_t I, typename T, std::ptrdiff_t Extent, std::ptrdiff_t MaxBufferExtent> class tuple_element<I, makeshift::fixed_buffer<T, Extent, MaxBufferExtent>> { public: using type = T; };


} // namespace std


#endif // INCLUDED_MAKESHIFT_BUFFER_HPP_
