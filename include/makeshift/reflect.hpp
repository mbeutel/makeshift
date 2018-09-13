
#ifndef INCLUDED_MAKESHIFT_REFLECT_HPP_
#define INCLUDED_MAKESHIFT_REFLECT_HPP_


#include <type_traits> // for integral_constant<>, enable_if<>, decay<>, is_same<>, is_enum<>

#include <makeshift/type_traits.hpp> // for type_sequence<>, tag<>, is_same_template<>
#include <makeshift/tuple.hpp>       // for type_tuple<>
#include <makeshift/metadata.hpp>    // for have_metadata<>, metadata_of<>


namespace makeshift
{

namespace detail
{


template <typename T, T... Vs>
    constexpr type_tuple<std::integral_constant<T, Vs>...> sequence_to_tuple(sequence<T, Vs...>) noexcept
{
    return { };
}


} // namespace detail


inline namespace metadata
{


    //ᅟ
    // For enum types with metadata, returns a tuple of type-encoded possible values.
    //ᅟ
    //ᅟ    enum Color { red, green, blue };
    //ᅟ    constexpr inline auto reflect(tag<Color>, reflection_metadata_tag) noexcept
    //ᅟ    {
    //ᅟ        return type<Color>(value<red>(), value<green>(), value<blue>());
    //ᅟ    }
    //ᅟ    auto colorValues = values_from_metadata<Color>(); // returns type_tuple<constant<red>, constant<green>, constant<blue>>
    //
template <typename T, typename MetadataTagT = reflection_metadata_tag>
    constexpr auto values_from_metadata(tag<T> = { }, MetadataTagT = { })
{
    static_assert(have_metadata_v<T, MetadataTagT>, "no metadata available for given type and tag");
    static_assert(std::is_enum<T>::value, "values from metadata only supported for enum types");

    auto values = metadata_of<T, MetadataTagT>.attributes
        | tuple_filter(template_predicate_v<is_same_template, value_metadata>)
        //| tuple_map([](const auto& v) { return c<std::decay_t<decltype(v)>::value_type::value>; });
        | tuple_map([](const auto& v) { return std::integral_constant<T, std::decay_t<decltype(v)>::value_type::value>{ }; }); // workaround for ICE in VC++
    using Values = decltype(values); // std::tuple<constant<Cs>...>
    return apply_t<type_tuple, Values>{ }; // type_tuple<constant<Cs>...>
}


    //ᅟ
    // For bool and for constrained integer types, returns a tuple of type-encoded possible values.
    //ᅟ
    //ᅟ    auto boolValues = values_from_type<bool>(); // returns type_tuple<constant<false>, constant<true>>
    //
template <typename T>
    constexpr auto values_from_type(tag<T> = { })
{
    static_assert(std::is_same<T, bool>::value || is_constrained_integer_v<T>, "values from type only supported for bool and constrained integer types");

    if constexpr (std::is_same<T, bool>::value)
        return type_tuple<std::false_type, std::true_type>{ };
    else // is_constrained_integer_v<T>
        return makeshift::detail::sequence_to_tuple(T::verifier::get_valid_values(typename T::constraint{ }));
}


    //ᅟ
    // For bool, constrained integer types, and for enum types with metadata, returns a tuple of type-encoded possible values.
    //ᅟ
template <typename T, typename MetadataTagT = reflection_metadata_tag>
    constexpr auto values_from_type_or_metadata(tag<T> = { }, MetadataTagT = { })
{
    if constexpr (std::is_same<T, bool>::value || is_constrained_integer_v<T>)
        return values_from_type<T>();
    else
        return values_from_metadata<T, MetadataTagT>();
}

} // inline namespace metadata

} // namespace makeshift


#ifdef INCLUDED_MAKESHIFT_VARIANT_HPP_
 #include <makeshift/detail/variant_reflect.hpp>
#endif // INCLUDED_MAKESHIFT_VARIANT_HPP_


#endif // INCLUDED_MAKESHIFT_REFLECT_HPP_
