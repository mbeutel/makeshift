
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


template <typename PtrT>
    struct member_object_pointer_accessor;
template <typename T, typename C, T C::* Pointer>
    struct member_object_pointer_accessor<std::integral_constant<T C::*, Pointer>>
{
    constexpr member_object_pointer_accessor(void) noexcept = default;
    constexpr member_object_pointer_accessor(std::integral_constant<T C::*, Pointer>) noexcept { }

    static const T& operator ()(const C& instance) { return instance.*Pointer; }
    static void operator ()(C& instance, const T& value) { instance.*Pointer = value; }
    static void operator ()(C& instance, T&& value) { instance.*Pointer = std::move(value); }
};


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


    //ᅟ
    // Returns a tuple of member metadata objects describing a struct type.
    //
template <typename T, typename MetadataTagT = reflection_metadata_tag>
    constexpr auto members_from_metadata(tag<T> = { }, MetadataTagT = { })
{
    static_assert(have_metadata_v<T, MetadataTagT>, "no metadata available for given type and tag");
    
    return metadata_of<T, MetadataTagT>.attributes
        | tuple_filter(template_predicate_v<is_same_template, member_metadata>);
}


    //ᅟ
    // Returns an accessor for the given member metadata object.
    //
template <typename AccessorsC, typename AttributesT>
    constexpr auto member_accessor(const member_metadata<AccessorsC, AttributesT>& memberMetadata)
{
    using AccessorsTuple = apply<type_tuple, AccessorsC>;
    auto memberObjectPointer = AccessorsTuple{ }
        | tuple_filter(value_type_predicate{ predicate_v<std::is_member_object_pointer> })
        | single_or_none();
    if constexpr (!is_none_v<decltype(memberObjectPointer)>)
        return makeshift::detail::member_object_pointer_accessor{ memberObjectPointer };
}


    //ᅟ
    // Compares the two strings for equality using the given string comparison mode.
    //
template <typename T, typename MetadataTagT = reflection_metadata_tag>
    class struct_equal_to
{
private:
    static constexpr auto memberAccessors_ = metadata_of<T, MetadataTagT>.attributes
        | tuple_filter(template_predicate_v<is_same_template, member_metadata>)
        | tuple_map([](auto v) { return member_accessor(v); });

public:
    constexpr struct_equal_to(void) noexcept = default;
    constexpr struct_equal_to(tag<T>, MetadataTagT = { }) noexcept = default;

    template <typename LhsT, typename RhsT>
        constexpr bool operator()(LhsT&& lhs, RhsT&& rhs) const noexcept
    {
        return memberAccessors_
            | tuple_fold([&](auto&& accessor){  })
    }
};
template <typename T>
    class struct_equal_to(tag<T>) -> struct_equal_to<T>;
template <typename T, typename MetadataTagT>
    class struct_equal_to(tag<T>, const MetadataTagT&) -> struct_equal_to<T, MetadataTagT>;


    //ᅟ
    // Computes the hash of a string using the given string comparison mode.
    //
class struct_hash
{
private:
    string_comparison comparison_;

public:
    constexpr string_hash(string_comparison _comparison) noexcept : comparison_(_comparison) { }

    MAKESHIFT_DLLFUNC std::size_t operator()(std::string_view arg) const noexcept;
};


    //ᅟ
    // Compares the two strings lexicographically using the given string comparison mode. Returns `true` if lower-case `lhs`
    // lexicographically precedes lower-case `rhs`, `false` otherwise.
    //
class struct_less
{
private:
    string_comparison comparison_;

public:
    constexpr string_less(string_comparison _comparison) noexcept : comparison_(_comparison) { }

    MAKESHIFT_DLLFUNC bool operator()(std::string_view lhs, std::string_view rhs) const noexcept;
};


} // inline namespace metadata

} // namespace makeshift


#ifdef INCLUDED_MAKESHIFT_VARIANT_HPP_
 #include <makeshift/detail/variant_reflect.hpp>
#endif // INCLUDED_MAKESHIFT_VARIANT_HPP_


#endif // INCLUDED_MAKESHIFT_REFLECT_HPP_
