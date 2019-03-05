
#ifndef INCLUDED_MAKESHIFT_DETAIL_REFLECT2_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_REFLECT2_HPP_


#include <array>
#include <cstddef>     // for size_t
#include <type_traits> // for is_base_of<>, integral_constant<>

#include <makeshift/type_traits.hpp>  // for can_apply<>
#include <makeshift/type_traits2.hpp> // for type<>
#include <makeshift/metadata2.hpp>    // for named2<>
#include <makeshift/tuple2.hpp>       // for array_transform2()


namespace makeshift
{

namespace detail
{


template <typename T> using raw_metadata_of_r = decltype(reflect(type<T>{ }));
template <typename T> struct have_raw_metadata : can_apply<makeshift::detail::raw_metadata_of_r, T> { };
template <typename T> struct is_value_metadata : std::is_base_of<values_base, raw_metadata_of_r<T>> { };


struct strip_names_functor
{
    template <typename T>
        constexpr T operator ()(const named2<T> v) const
    {
        return v.value;
    }
};
struct add_names_functor
{
    template <typename T>
        constexpr named2<T> operator ()(const T& v) const
    {
        return { v, { } };
    }
};

template <typename ValuesT>
    struct values_reflector0;
template <typename T, std::size_t N>
    struct values_reflector0<values_t<T, N>>
{
    constexpr auto operator ()(void) const noexcept
    {
        return reflect(type<T>{ }).values;
    }
};
template <typename T, std::size_t N>
    struct values_reflector0<values_t<named2<T>, N>>
{
    constexpr auto operator ()(void) const noexcept
    {
        return makeshift::array_transform2(strip_names_functor{ }, reflect(type<T>{ }).values);
    }
};

template <typename ValuesT>
    struct named_values_reflector0;
template <typename T, std::size_t N>
    struct named_values_reflector0<values_t<T, N>>
{
    constexpr auto operator ()(void) const noexcept
    {
        return makeshift::array_transform2(add_names_functor{ }, reflect(type<T>{ }).values);
    }
};
template <typename T, std::size_t N>
    struct named_values_reflector0<values_t<named2<T>, N>>
{
    constexpr auto operator ()(void) const noexcept
    {
        return reflect(type<T>{ }).values;
    }
};

template <typename T>
    struct values_reflector : values_reflector0<raw_metadata_of_r<T>>
{
};
template <>
    struct values_reflector<bool>
{
    constexpr std::array<bool, 2> operator ()(void) const noexcept
    {
        return { false, true };
    }
};

template <typename T>
    struct named_values_reflector : named_values_reflector0<raw_metadata_of_r<T>>
{
};
template <>
    struct named_values_reflector<bool>
{
    constexpr std::array<named2<bool>, 2> operator ()(void) const noexcept
    {
        return { named2<bool>{ false, "false" }, named2<bool>{ true, "true" } };
    }
};


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_REFLECT2_HPP_
