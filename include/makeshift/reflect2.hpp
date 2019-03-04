
#ifndef INCLUDED_MAKESHIFT_REFLECT2_HPP_
#define INCLUDED_MAKESHIFT_REFLECT2_HPP_


#include <array>
#include <cstddef>     // for size_t
#include <type_traits> // for is_base_of<>, integral_constant<>

#include <makeshift/type_traits.hpp>  // for can_apply<>
#include <makeshift/type_traits2.hpp> // for type<>
#include <makeshift/metadata2.hpp>    // for named2<>
#include <makeshift/tuple2.hpp>       // for array_transform2()
#include <makeshift/version.hpp>      // for MAKESHIFT_NODISCARD


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
        return array_transform2(strip_names_functor{ }, reflect(type<T>{ }).values);
    }
};

template <typename ValuesT>
    struct named_values_reflector0;
template <typename T, std::size_t N>
    struct named_values_reflector0<values_t<T, N>>
{
    constexpr auto operator ()(void) const noexcept
    {
        return array_transform2(add_names_functor{ }, reflect(type<T>{ }).values);
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


inline namespace metadata
{


    //ᅟ
    // Determines whether values can be enumerated for the given type.
    //
template <typename T> struct have_value_metadata : std::conjunction<makeshift::detail::have_raw_metadata<T>, makeshift::detail::is_value_metadata<T>> { };
template <> struct have_value_metadata<bool> : std::true_type { };

    //ᅟ
    // Determines whether values can be enumerated for the given type.
    //
template <typename T> constexpr bool have_value_metadata_v = have_value_metadata<T>::value;


    //ᅟ
    // Returns an array of the values of a given type, retrieved from metadata.
    // Returns `std::array{ false, true }` if the type argument is `bool`.
    //
struct values_of_t
{
        //ᅟ
        // Returns an array of the values of a given type, retrieved from metadata.
        // Returns `std::array{ false, true }` if the type argument is `bool`.
        //
    template <typename T>
        MAKESHIFT_NODISCARD constexpr auto operator ()(type<T>) const
    {
        static_assert(have_value_metadata_v<T>, "cannot enumerate values without metadata");

        return makeshift::detail::values_reflector<T>{ }();
    }
};

    //ᅟ
    // Returns an array of the values of a given type, retrieved from metadata.
    // Returns `std::array{ false, true }` if the type argument is `bool`.
    //
static inline constexpr values_of_t values_of = { };


    //ᅟ
    // Returns an array of the names and values of a given type, retrieved from metadata.
    // Returns `std::array{ named{ false, "false" }, named{ true, "true" } }` if the type argument is `bool`.
    //
struct named_values_of_t
{
        //ᅟ
        // Returns an array of the names and values of a given type, retrieved from metadata.
        // Returns `std::array{ named{ false, "false" }, named{ true, "true" } }` if the type argument is `bool`.
        //
    template <typename T>
        MAKESHIFT_NODISCARD constexpr auto operator ()(type<T>) const
    {
        static_assert(have_value_metadata_v<T>, "cannot enumerate values without metadata");
    
        return makeshift::detail::named_values_reflector<T>{ }();
    }
};

    //ᅟ
    // Returns an array of the names and values of a given type, retrieved from metadata.
    // Returns `std::array{ named{ false, "false" }, named{ true, "true" } }` if the type argument is `bool`.
    //
static inline constexpr named_values_of_t named_values_of = { };


} // inline namespace metadata

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_REFLECT2_HPP_
