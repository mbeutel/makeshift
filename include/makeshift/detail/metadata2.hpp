
#ifndef INCLUDED_MAKESHIFT_DETAIL_METADATA2_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_METADATA2_HPP_


#include <array>
#include <cstddef> // for size_t
#include <utility> // for move(), integer_sequence<>

#include <makeshift/version.hpp> // for MAKESHIFT_NODISCARD


namespace makeshift
{

namespace detail
{


template <std::size_t... Is, typename T, std::size_t N>
    constexpr std::array<std::remove_cv_t<T>, N> to_array2_impl(std::index_sequence<Is...>, const T (&array)[N])
{
    return { array[Is]... };
}
template <typename T, std::size_t N>
    constexpr std::array<std::remove_cv_t<T>, N> to_array2(const T (&array)[N])
{
    return to_array2_impl(std::make_index_sequence<N>{ }, array);
}


struct values_base { };

struct no_values_tag { };

template <typename T, std::size_t N>
    struct values_t : values_base
{
    std::array<T, N> values;

    constexpr values_t(const std::array<T, N>& _values) : values(_values) { }
};
template <typename T, std::size_t N>
    MAKESHIFT_NODISCARD constexpr std::array<T, N> to_array(const values_t<T, N>& values)
{
    return values.values;
}

template <typename T>
    struct values_initializer_t
{
    MAKESHIFT_NODISCARD constexpr values_t<T, 0> operator =(no_values_tag) const
    {
        return { { } };
    }
    template <std::size_t N>
        MAKESHIFT_NODISCARD constexpr values_t<T, N> operator =(T (&&vals)[N]) const
    {
        return { makeshift::detail::to_array2(vals) };
    }
    template <std::size_t N> 
        MAKESHIFT_NODISCARD constexpr values_t<T, N> operator =(const std::array<T, N>& vals) const
    {
        return { vals };
    }
};


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_METADATA2_HPP_
