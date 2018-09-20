
#ifndef INCLUDED_MAKESHIFT_REFLECT_HPP_
#define INCLUDED_MAKESHIFT_REFLECT_HPP_


#include <type_traits> // for integral_constant<>, enable_if<>, decay<>, is_same<>, is_enum<>

#include <makeshift/type_traits.hpp> // for type_sequence<>, tag<>, is_instantiation_of<>
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
        | tuple_filter(template_trait_v<is_instantiation_of, value_metadata>)
        //| tuple_map([](const auto& v) { return c<std::decay_t<decltype(v)>::value>; });
        | tuple_map([](const auto& v) { return typename std::decay_t<decltype(v)>::type{ }; }); // workaround for ICE in VC++
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


namespace detail
{


template <typename T, typename MetadataTagT>
    constexpr auto get_member_accessors(void) noexcept
{
    return metadata_of<T, MetadataTagT>.attributes
        | tuple_filter(template_trait_v<is_instantiation_of, member_metadata>)
        | tuple_map([](auto v) { return member_accessor(v); });
}

template <typename ComparerT, typename AccessorsT, typename T>
    constexpr bool lexicographical_compare_members(ComparerT&&, const AccessorsT&, const T&, const T&, std::index_sequence<>) noexcept
{
    return false;
}
template <typename ComparerT, typename AccessorsT, typename T, std::size_t I0, std::size_t... Is>
    constexpr bool lexicographical_compare_members(ComparerT&& cmp, const AccessorsT& memberAccessors, const T& lhs, const T& rhs, std::index_sequence<I0, Is...>) noexcept
{
    auto accessor = std::get<I0>(memberAccessors);
    const auto& lhsMember = accessor(lhs);
    const auto& rhsMember = accessor(rhs);
    if (less_impl(lhsMember, rhsMember, cmp, cmp)) return true;
    if (less_impl(rhsMember, lhsMember, cmp, cmp)) return false;
    return lexicographical_compare_members(cmp, memberAccessors, lhs, rhs, std::index_sequence<Is...>{ });
}
template <typename T, typename ComparerT>
    constexpr bool aggregate_less(const T& lhs, const T& rhs, ComparerT&& cmp) noexcept
{
    using MetadataTag = metadata_tag_of_comparer_t<std::decay_t<ComparerT>>;
    constexpr auto memberAccessors = get_member_accessors<T, MetadataTag>();
    using MemberAccessors = decltype(memberAccessors);

    return lexicographical_compare_members(cmp, memberAccessors, lhs, rhs, std::make_index_sequence<std::tuple_size<MemberAccessors>::value>{ });
}

template <typename T, typename ComparerT>
    constexpr bool aggregate_equal_to(const T& lhs, const T& rhs, ComparerT&& cmp) noexcept
{
    using MetadataTag = metadata_tag_of_comparer_t<std::decay_t<ComparerT>>;
    constexpr auto memberAccessors = get_member_accessors<T, MetadataTag>();

    return memberAccessors
        | tuple_all([&lhs, &rhs, &cmp](auto&& accessor){ return equal_to_impl(accessor(lhs), accessor(rhs), cmp, cmp); });
}

static constexpr inline std::size_t hash_combine(std::size_t seed, std::size_t newHash) noexcept
{
        // taken from boost::hash_combine()
    return seed ^ newHash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
template <typename T, typename ComparerT>
    constexpr std::size_t aggregate_hash(const T& obj, ComparerT&& cmp) noexcept
{
    using MetadataTag = metadata_tag_of_comparer_t<std::decay_t<ComparerT>>;
    constexpr auto memberAccessors = get_member_accessors<T, MetadataTag>();

    return memberAccessors
        | tuple_fold(0, [&obj, &cmp](std::size_t seed, auto&& accessor) { return hash_combine(seed, hash_impl(accessor(obj), cmp, cmp)); });
}


} // namespace detail

} // namespace makeshift


#ifdef INCLUDED_MAKESHIFT_VARIANT_HPP_
 #include <makeshift/detail/variant_reflect.hpp>
#endif // INCLUDED_MAKESHIFT_VARIANT_HPP_


#endif // INCLUDED_MAKESHIFT_REFLECT_HPP_
