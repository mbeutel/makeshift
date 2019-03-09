
#ifndef INCLUDED_MAKESHIFT_REFLECT2_HPP_
#define INCLUDED_MAKESHIFT_REFLECT2_HPP_


#include <type_traits> // for integral_constant<>, conjunction<>

#include <makeshift/version.hpp> // for MAKESHIFT_NODISCARD

#include <makeshift/detail/reflect2.hpp>


namespace makeshift
{

inline namespace metadata
{


    //ᅟ
    // Determines whether values can be enumerated for the given type.
    //
template <typename T> struct have_value_metadata : std::disjunction<
    makeshift::detail::can_infer_values_<T>,
    std::conjunction<makeshift::detail::have_raw_metadata<T>, makeshift::detail::is_value_metadata<T>>> { };
template <> struct have_value_metadata<bool> : std::true_type { };

    //ᅟ
    // Determines whether values can be enumerated for the given type.
    //
template <typename T> constexpr bool have_value_metadata_v = have_value_metadata<T>::value;


    //ᅟ
    // Returns an array of the values of a given type, derived from the type or retrieved from metadata.
    //
struct values_of_t
{
        //ᅟ
        // Returns an array of the values of a given type, derived from the type or retrieved from metadata.
        //
    template <typename T>
        MAKESHIFT_NODISCARD constexpr auto operator ()(type<T>) const
    {
        static_assert(have_value_metadata_v<T>, "cannot enumerate values without metadata");

        return makeshift::detail::values_reflector<T>{ }();
    }
};

    //ᅟ
    // Returns an array of the values of a given type, derived from the type or retrieved from metadata.
    //
static inline constexpr values_of_t values_of = { };


    //ᅟ
    // Returns an array of the names and values of a given type, derived from the type or retrieved from metadata.
    //
struct named_values_of_t
{
        //ᅟ
        // Returns an array of the names and values of a given type, derived from the type or retrieved from metadata.
        //
    template <typename T>
        MAKESHIFT_NODISCARD constexpr auto operator ()(type<T>) const
    {
        static_assert(have_value_metadata_v<T>, "cannot enumerate values without metadata");
    
        return makeshift::detail::named_values_reflector<T>{ }();
    }
};

    //ᅟ
    // Returns an array of the names and values of a given type, derived from the type or retrieved from metadata.
    //
static inline constexpr named_values_of_t named_values_of = { };


} // inline namespace metadata

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_REFLECT2_HPP_
