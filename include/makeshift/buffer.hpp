
#ifndef INCLUDED_MAKESHIFT_BUFFER_HPP_
#define INCLUDED_MAKESHIFT_BUFFER_HPP_


#include <array>
#include <utility>     // for tuple_size<>, tuple_element<>
#include <iterator>    // for move_iterator<>
#include <type_traits> // for is_convertible<>

#include <gsl/gsl_assert> // for Expects()

#include <makeshift/constval.hpp> // for constval_extract()
#include <makeshift/utility2.hpp> // for dim

#include <makeshift/detail/buffer.hpp>


namespace makeshift
{


constexpr dim dynamic_extent = -1;


template <typename C>
    constexpr dim extent_from_constval(C = { })
{
    return makeshift::detail::extent_from_constval_impl<C>(is_constval<C>{ });
}


template <typename T, dim Extent = dynamic_extent, dim MaxStaticBufferExtent = dynamic_extent>
    class buffer
        : public makeshift::detail::buffer_base<T, Extent, MaxStaticBufferExtent, makeshift::detail::determine_memory_location(Extent, MaxStaticBufferExtent)>
{
private:
    using _base = makeshift::detail::buffer_base<T, Extent, MaxStaticBufferExtent, makeshift::detail::determine_memory_location(Extent, MaxStaticBufferExtent)>;

    struct check_size_functor
    {
        constexpr bool operator ()(dim size) const noexcept
        {
            return size >= 0
                && (Extent == dynamic_extent || size == Extent);
        }
    };

public:
    template <typename C>
        explicit constexpr buffer(C _size)
            : _base(makeshift::constval_extract(_size))
    {
        makeshift::constval_assert(check_size_functor{ }, _size); // ensure that static and dynamic sizes match
    }
    template <dim RExtent, typename U>
        constexpr buffer(U (&&array)[RExtent])
            : _base(RExtent)
    {
        static_assert(Extent == dynamic_extent || RExtent == Extent, "array extent does not match");
        static_assert(std::is_convertible<U, T>::value, "incompatible array element types");
        std::copy(std::make_move_iterator(array), std::make_move_iterator(array + RExtent), this->begin());
    }
    template <dim RExtent, typename U>
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
    buffer<T, makeshift::extent_from_constval<C>()>
    make_buffer(C size)
{
    return buffer<T, makeshift::extent_from_constval<C>()>{ size };
}

template <typename T, dim MaxStaticBufferExtent,
          typename C>
    MAKESHIFT_NODISCARD constexpr
    buffer<T, makeshift::extent_from_constval<C>(), MaxStaticBufferExtent>
    make_buffer(C size)
{
    return buffer<T, makeshift::extent_from_constval<C>(), MaxStaticBufferExtent>{ size };
}


template <typename T, dim Extent, dim MaxBufferExtent>
    class fixed_buffer
        : public makeshift::detail::static_buffer_base<T, Extent, MaxBufferExtent>
{
    static_assert(MaxBufferExtent >= 0, "invalid maximal buffer extent");
    static_assert(Extent <= MaxBufferExtent, "size exceeds buffer extent");

private:
    using _base = makeshift::detail::static_buffer_base<T, Extent, MaxBufferExtent>;

    struct check_size_functor
    {
        constexpr bool operator ()(dim size) const noexcept
        {
            return size >= 0
                && (Extent == dynamic_extent || size == Extent)
                && size <= MaxBufferExtent;
        }
    };

public:
    template <typename C>
        explicit constexpr fixed_buffer(C _size)
            : _base(makeshift::constval_extract(_size))
    {
        makeshift::constval_assert(check_size_functor{ }, _size); // ensure that static and dynamic sizes match
    }
    template <dim RExtent, typename U>
        constexpr fixed_buffer(U (&&array)[RExtent])
            : _base(RExtent)
    {
        static_assert(Extent == dynamic_extent || RExtent == Extent, "array extent does not match");
        static_assert(std::is_convertible<U, T>::value, "incompatible array element types");
        std::copy(std::make_move_iterator(array), std::make_move_iterator(array + RExtent), this->begin());
    }
    template <dim RExtent, typename U>
        constexpr fixed_buffer& operator =(U (&&array)[RExtent])
    {
        static_assert(Extent == dynamic_extent || RExtent == Extent, "array extent does not match");
        static_assert(std::is_convertible<U, T>::value, "incompatible array element types");
        Expects(RExtent == this->size());
        std::copy(std::make_move_iterator(array), std::make_move_iterator(array + RExtent), this->begin());
        return *this;
    }
};

template <typename T, typename C>
    MAKESHIFT_NODISCARD constexpr
    fixed_buffer<T, makeshift::extent_from_constval<C>(), makeshift::detail::constval_value<C>>
    make_fixed_buffer(C size)
{
    static_assert(is_constval_v<C>, "fixed buffer size must be a constval if no maximal extent is given");
    return fixed_buffer<T, makeshift::extent_from_constval<C>(), makeshift::detail::constval_value<C>>{ size };
}

template <typename T, dim MaxBufferExtent, typename C>
    MAKESHIFT_NODISCARD constexpr
    fixed_buffer<T, makeshift::extent_from_constval<C>(), MaxBufferExtent>
    make_fixed_buffer(C size)
{
    return fixed_buffer<T, makeshift::extent_from_constval<C>(), MaxBufferExtent>{ size };
}


    // Implement tuple-like protocol for `buffer<>`.
template <std::size_t I, typename T, dim Extent, dim MaxStaticBufferExtent,
          std::enable_if_t<Extent != dynamic_extent, int> = 0>
    MAKESHIFT_NODISCARD constexpr T& get(buffer<T, Extent, MaxStaticBufferExtent>& buffer) noexcept
{
    static_assert(I < Extent, "index out of range");
    return buffer[I];
}
template <std::size_t I, typename T, dim Extent, dim MaxStaticBufferExtent,
          std::enable_if_t<Extent != dynamic_extent, int> = 0>
    MAKESHIFT_NODISCARD constexpr const T& get(const buffer<T, Extent, MaxStaticBufferExtent>& buffer) noexcept
{
    static_assert(I < Extent, "index out of range");
    return buffer[I];
}

    // Implement tuple-like protocol for `fixed_buffer<>`.
template <std::size_t I, typename T, dim Extent, dim MaxBufferExtent,
          std::enable_if_t<Extent != dynamic_extent, int> = 0>
    MAKESHIFT_NODISCARD constexpr T& get(fixed_buffer<T, Extent, MaxBufferExtent>& buffer) noexcept
{
    static_assert(I < Extent, "index out of range");
    return buffer[I];
}
template <std::size_t I, typename T, dim Extent, dim MaxBufferExtent,
          std::enable_if_t<Extent != dynamic_extent, int> = 0>
    MAKESHIFT_NODISCARD constexpr const T& get(const fixed_buffer<T, Extent, MaxBufferExtent>& buffer) noexcept
{
    static_assert(I < Extent, "index out of range");
    return buffer[I];
}


} // namespace makeshift


namespace std
{


    // Implement tuple-like protocol for `buffer<>`.
template <typename T, makeshift::dim Extent, makeshift::dim MaxStaticBufferExtent> struct tuple_size<makeshift::buffer<T, Extent, MaxStaticBufferExtent>> : std::integral_constant<std::size_t, Extent> { };
template <typename T, makeshift::dim MaxStaticBufferExtent> struct tuple_size<makeshift::buffer<T, -1, MaxStaticBufferExtent>>; // undefined
template <std::size_t I, typename T, makeshift::dim Extent, makeshift::dim MaxStaticBufferExtent> struct tuple_element<I, makeshift::buffer<T, Extent, MaxStaticBufferExtent>> { using type = T; };

    // Implement tuple-like protocol for `fixed_buffer<>`.
template <typename T, makeshift::dim Extent, makeshift::dim MaxBufferExtent> struct tuple_size<makeshift::fixed_buffer<T, Extent, MaxBufferExtent>> : std::integral_constant<std::size_t, Extent> { };
template <typename T, makeshift::dim MaxBufferExtent> struct tuple_size<makeshift::fixed_buffer<T, -1, MaxBufferExtent>>; // undefined
template <std::size_t I, typename T, makeshift::dim Extent, makeshift::dim MaxBufferExtent> struct tuple_element<I, makeshift::fixed_buffer<T, Extent, MaxBufferExtent>> { using type = T; };


} // namespace std


#endif // INCLUDED_MAKESHIFT_BUFFER_HPP_
