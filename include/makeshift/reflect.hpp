
#ifndef INCLUDED_MAKESHIFT_REFLECT_HPP_
#define INCLUDED_MAKESHIFT_REFLECT_HPP_


#include <type_traits> // for integral_constant<>, enable_if<>, decay<>, is_same<>, is_enum<>, is_member_object_pointer<>
#include <utility>     // for move(), forward<>()

#include <makeshift/type_traits.hpp> // for sequence<>, tag<>, is_instantiation_of<>
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


template <typename PtrT>
    struct member_object_pointer_accessor;
template <typename T, typename C, T C::* Pointer>
    struct member_object_pointer_accessor<std::integral_constant<T C::*, Pointer>>
{
    using value_type = T;

    constexpr member_object_pointer_accessor(void) noexcept = default;
    constexpr member_object_pointer_accessor(std::integral_constant<T C::*, Pointer>) noexcept { }

    constexpr const T& operator ()(const C& instance) const noexcept { return instance.*Pointer; }
    constexpr void operator ()(C& instance, const T& value) const { instance.*Pointer = value; }
    constexpr void operator ()(C& instance, T&& value) const { instance.*Pointer = std::move(value); }
};
template <typename T, typename C, T C::* Pointer>
    member_object_pointer_accessor(std::integral_constant<T C::*, Pointer>) -> member_object_pointer_accessor<std::integral_constant<T C::*, Pointer>>;


    //ᅟ
    // For enum types with metadata, returns a tuple of type-encoded possible values.
    //ᅟ
    //ᅟ    enum Color { red, green, blue };
    //ᅟ    constexpr inline auto reflect(tag<Color>, reflection_tag) noexcept
    //ᅟ    {
    //ᅟ        return type<Color>(value<red>(), value<green>(), value<blue>());
    //ᅟ    }
    //ᅟ    auto colorValues = values_from_metadata<Color>(); // returns type_tuple<constant<red>, constant<green>, constant<blue>>
    //
template <typename T, typename MetadataTagT = reflection_tag>
    constexpr auto values_from_metadata(tag<T> = { }, MetadataTagT = { })
{
    static_assert(have_metadata_v<T, MetadataTagT>, "no metadata available for given type and tag");
    static_assert(std::is_enum<T>::value, "values from metadata only supported for enum types");

    auto lvalues = metadata_of<T, MetadataTagT>.attributes
        | tuple_filter(template_trait_v<is_instantiation_of, value_metadata>)
        //| tuple_map([](const auto& v) { return c<std::decay_t<decltype(v)>::value>; });
        | tuple_map([](const auto& v) { return typename std::decay_t<decltype(v)>::type{ }; }); // workaround for ICE in VC++
    using Values = decltype(lvalues); // std::tuple<constant<Cs>...>
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

} // namespace detail


inline namespace metadata
{


    //ᅟ
    // For bool, constrained integer types, and for enum types with metadata, returns a tuple of type-encoded possible values.
    //ᅟ
template <typename T, typename MetadataTagT = reflection_tag>
    constexpr auto values_of(tag<T> = { }, MetadataTagT = { })
{
    if constexpr (std::is_same<T, bool>::value || is_constrained_integer_v<T>)
        return makeshift::detail::values_from_type<T>();
    else
        return makeshift::detail::values_from_metadata<T, MetadataTagT>();
}


    //ᅟ
    // Returns a tuple of member metadata objects describing a struct type.
    //
template <typename T, typename MetadataTagT = reflection_tag>
    constexpr auto members_of(tag<T> = { }, MetadataTagT = { })
{
    static_assert(have_metadata_v<T, MetadataTagT>, "no metadata available for given type and tag");
    
    return metadata_of<T, MetadataTagT>.attributes
        | tuple_filter(template_trait_v<is_instantiation_of, member_metadata>);
}


    //ᅟ
    // Returns an accessor for the given member metadata object.
    //
template <typename MemberMetadataT>
    constexpr auto member_accessor(const MemberMetadataT&)
{
    using AccessorsTuple = apply_t<type_tuple, typename MemberMetadataT::accessors>;
    auto memberObjectPointer = AccessorsTuple{ }
        | tuple_filter(trait_v<std::is_member_object_pointer>(value_type_transform_v))
        | single_or_none();
    if constexpr (!is_none_v<decltype(memberObjectPointer)>)
        return makeshift::detail::member_object_pointer_accessor{ memberObjectPointer };
}


} // inline namespace metadata

} // namespace makeshift


#ifdef INCLUDED_MAKESHIFT_VARIANT_HPP_
 #include <makeshift/detail/variant-reflect.hpp>
#endif // INCLUDED_MAKESHIFT_VARIANT_HPP_

#ifdef INCLUDED_MAKESHIFT_DETAIL_FUNCTIONAL_COMPARER_HPP_
 #include <makeshift/detail/functional_comparer-reflect.hpp>
#endif // INCLUDED_MAKESHIFT_DETAIL_FUNCTIONAL_COMPARER_HPP_

#ifdef INCLUDED_MAKESHIFT_SERIALIZERS_STREAM_HPP_
 #include <makeshift/detail/serializers_stream-reflect.hpp>
#endif // INCLUDED_MAKESHIFT_SERIALIZERS_STREAM_HPP_

#ifdef INCLUDED_MAKESHIFT_SERIALIZERS_HINT_HPP_
 #include <makeshift/detail/serializers_hint-reflect.hpp>
#endif // INCLUDED_MAKESHIFT_SERIALIZERS_HINT_HPP_


#endif // INCLUDED_MAKESHIFT_REFLECT_HPP_
