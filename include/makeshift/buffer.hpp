
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


template <typename T, std::ptrdiff_t Extent = dynamic_extent, std::ptrdiff_t MaxStaticBufferExtent = dynamic_extent>
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

template <typename T,
          typename C>
    MAKESHIFT_NODISCARD constexpr
    buffer<T, makeshift::detail::buffer_extent_from_constval(C{ })>
    make_buffer(C size)
{
    return buffer<T, makeshift::detail::buffer_extent_from_constval(C{ })>(size);
}

template <typename T, std::ptrdiff_t MaxStaticBufferExtent,
          typename C>
    MAKESHIFT_NODISCARD constexpr
    buffer<T, makeshift::detail::buffer_extent_from_constval(C{ }), MaxStaticBufferExtent>
    make_buffer(C size)
{
    return buffer<T, makeshift::detail::buffer_extent_from_constval(C{ }), MaxStaticBufferExtent>(size);
}


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

template <typename T, typename SizeT, SizeT Size>
    MAKESHIFT_NODISCARD constexpr
    fixed_buffer<T, Size, Size>
    make_fixed_buffer(std::integral_constant<SizeT, Size>)
{
    return fixed_buffer<T, Size, Size>(std::integral_constant<SizeT, Size>{ });
}

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
template <typename T, std::ptrdiff_t Extent, std::ptrdiff_t MaxStaticBufferExtent> struct tuple_size<makeshift::buffer<T, Extent, MaxStaticBufferExtent>> : std::integral_constant<std::size_t, Extent> { };
template <typename T, std::ptrdiff_t MaxStaticBufferExtent> struct tuple_size<makeshift::buffer<T, -1, MaxStaticBufferExtent>>; // undefined
template <std::size_t I, typename T, std::ptrdiff_t Extent, std::ptrdiff_t MaxStaticBufferExtent> struct tuple_element<I, makeshift::buffer<T, Extent, MaxStaticBufferExtent>> { using type = T; };

    // Implement tuple-like protocol for `fixed_buffer<>`.
template <typename T, std::ptrdiff_t Extent, std::ptrdiff_t MaxBufferExtent> struct tuple_size<makeshift::fixed_buffer<T, Extent, MaxBufferExtent>> : std::integral_constant<std::size_t, Extent> { };
template <typename T, std::ptrdiff_t MaxBufferExtent> struct tuple_size<makeshift::fixed_buffer<T, -1, MaxBufferExtent>>; // undefined
template <std::size_t I, typename T, std::ptrdiff_t Extent, std::ptrdiff_t MaxBufferExtent> struct tuple_element<I, makeshift::fixed_buffer<T, Extent, MaxBufferExtent>> { using type = T; };


} // namespace std


#endif // INCLUDED_MAKESHIFT_BUFFER_HPP_
