
#ifndef INCLUDED_MAKESHIFT_DETAIL_REFLECT2_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_REFLECT2_HPP_


#include <array>
#include <cstddef>     // for size_t
#include <type_traits> // for is_base_of<>, integral_constant<>

#include <makeshift/type_traits2.hpp> // for can_apply<>, type<>
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

template <typename T, typename = void>
    struct values_inferrer;
template <typename T> struct can_infer_values_ : can_apply<values_inferrer, T> { };
template <>
    struct values_inferrer<bool>
{
    constexpr std::array<bool, 2> operator ()(void) const noexcept
    {
        return { false, true };
    }
};
template <typename T>
    struct values_inferrer<T, std::enable_if_t<std::conjunction_v<std::is_empty<T>, std::is_trivially_default_constructible<T>>>>
{
    constexpr std::array<T, 1> operator ()(void) const noexcept
    {
        return { { } };
    }
};
template <typename ValuesT>
    struct values_reflector1;
template <typename T, std::size_t N>
    struct values_reflector1<values_t<T, N>>
{
    constexpr auto operator ()(void) const noexcept
    {
        return reflect(type<T>{ }).values;
    }
};
template <typename T, std::size_t N>
    struct values_reflector1<values_t<named2<T>, N>>
{
    constexpr auto operator ()(void) const noexcept
    {
        return makeshift::array_transform2(strip_names_functor{ }, reflect(type<T>{ }).values);
    }
};

template <typename T, bool HaveRawMetadata>
    struct values_reflector0;
template <typename T>
    struct values_reflector0<T, false> : values_inferrer<T>
{
};
template <typename T>
    struct values_reflector0<T, true> : values_reflector1<raw_metadata_of_r<T>>
{
};

template <typename T, typename = void>
    struct values_reflector : values_reflector0<T, have_raw_metadata<T>::value>
{
};

template <typename T>
    struct named_values_inferrer
{
    constexpr auto operator ()(void) const noexcept
    {
        return makeshift::array_transform2(add_names_functor{ }, values_inferrer<T>{ }());
    }
};
template <>
    struct named_values_inferrer<bool>
{
    constexpr std::array<named2<bool>, 2> operator ()(void) const noexcept
    {
        return { named2<bool>{ false, "false" }, named2<bool>{ true, "true" } };
    }
};

template <typename ValuesT>
    struct named_values_reflector1;
template <typename T, std::size_t N>
    struct named_values_reflector1<values_t<T, N>>
{
    constexpr auto operator ()(void) const noexcept
    {
        return makeshift::array_transform2(add_names_functor{ }, reflect(type<T>{ }).values);
    }
};
template <typename T, std::size_t N>
    struct named_values_reflector1<values_t<named2<T>, N>>
{
    constexpr auto operator ()(void) const noexcept
    {
        return reflect(type<T>{ }).values;
    }
};

template <typename T, bool HaveRawMetadata>
    struct named_values_reflector0;
template <typename T>
    struct named_values_reflector0<T, false> : named_values_inferrer<T>
{
};
template <typename T>
    struct named_values_reflector0<T, true> : named_values_reflector1<raw_metadata_of_r<T>>
{
};

template <typename T, typename = void>
    struct named_values_reflector : named_values_reflector0<T, have_raw_metadata<T>::value>
{
};


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_REFLECT2_HPP_
