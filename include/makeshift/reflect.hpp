
#ifndef INCLUDED_MAKESHIFT_REFLECT_HPP_
#define INCLUDED_MAKESHIFT_REFLECT_HPP_


#include <type_traits> // for integral_constant<>, enable_if<>, decay<>, is_same<>, is_enum<>

#include <makeshift/type_traits.hpp> // for type_sequence<>, tag<>, is_instantiation_of<>
#include <makeshift/tuple.hpp>       // for type_tuple<>
#include <makeshift/metadata.hpp>    // for have_metadata<>, metadata_of<>

#include <makeshift/detail/functional_operators.hpp> // for hash<>


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


} // namespace detail


inline namespace metadata
{


    //ᅟ
    // Compares two struct objects for member-wise equality using metadata-based reflection.
    //
template <typename T, typename MetadataTagT = reflection_metadata_tag, typename MemberEqualToT = equal_to<MetadataTagT>>
    class aggregate_equal_to : MemberEqualToT
{
private:
    static constexpr auto memberAccessors_ = makeshift::detail::get_member_accessors<T, MetadataTagT>();

public:
    template <typename = std::enable_if_t<std::is_default_constructible<MemberEqualToT>::value>>
        constexpr aggregate_equal_to(void) noexcept(std::is_nothrow_default_constructible<MemberEqualToT>::value)
    {
    }
    constexpr aggregate_equal_to(MemberEqualToT _equalTo) : MemberEqualToT(_equalTo) { }
    template <typename = std::enable_if_t<std::is_default_constructible<MemberEqualToT>::value>>
        constexpr aggregate_equal_to(tag<T>, MetadataTagT = { }) noexcept(std::is_nothrow_default_constructible<MemberEqualToT>::value)
    {
    }
    constexpr aggregate_equal_to(tag<T>, MetadataTagT, MemberEqualToT _equalTo) : MemberEqualToT(_equalTo) { }

    constexpr bool operator()(const T& lhs, const T& rhs) const noexcept
    {
        return memberAccessors_
            | tuple_all([this, &lhs, &rhs](auto&& accessor){ return static_cast<const MemberEqualToT&>(*this)(accessor(lhs), accessor(rhs)); });
    }
};
template <typename T>
    aggregate_equal_to(tag<T>) -> aggregate_equal_to<T>;
template <typename T, typename MetadataTagT>
    aggregate_equal_to(tag<T>, const MetadataTagT&) -> aggregate_equal_to<T, MetadataTagT>;
template <typename T, typename MetadataTagT, typename MemberEqualToT>
    aggregate_equal_to(tag<T>, const MetadataTagT&, MemberEqualToT&&) -> aggregate_equal_to<T, MetadataTagT, std::decay_t<MemberEqualToT>>;


    //ᅟ
    // Compares two struct objects for member-wise ordering using metadata-based reflection.
    //
template <typename T, typename MetadataTagT = reflection_metadata_tag, typename MemberLessT = less<MetadataTagT>>
    class aggregate_less : MemberLessT
{
private:
    static constexpr auto memberAccessors_ = makeshift::detail::get_member_accessors<T, MetadataTagT>();

    constexpr bool lexicographical_compare_(const T&, const T&, std::index_sequence<>) const noexcept
    {
        return false;
    }
    template <std::size_t I0, std::size_t... Is>
        constexpr bool lexicographical_compare_(const T& lhs, const T& rhs, std::index_sequence<I0, Is...>) const noexcept
    {
        auto accessor = std::get<I0>(memberAccessors_);
        const auto& lhsMember = accessor(lhs);
        const auto& rhsMember = accessor(rhs);
        if (static_cast<const MemberLessT&>(*this)(lhsMember, rhsMember)) return true;
        if (static_cast<const MemberLessT&>(*this)(rhsMember, lhsMember)) return false;
        return lexicographical_compare_(lhs, rhs, std::index_sequence<Is...>{ });
    }


public:
    template <typename = std::enable_if_t<std::is_default_constructible<MemberLessT>::value>>
        constexpr aggregate_less(void) noexcept(std::is_nothrow_default_constructible<MemberLessT>::value)
    {
    }
    constexpr aggregate_less(MemberLessT _less) : MemberLessT(_less) { }
    template <typename = std::enable_if_t<std::is_default_constructible<MemberLessT>::value>>
        constexpr aggregate_less(tag<T>, MetadataTagT = { }) noexcept(std::is_nothrow_default_constructible<MemberLessT>::value)
    {
    }
    constexpr aggregate_less(tag<T>, MetadataTagT, MemberLessT _less) : MemberLessT(_less) { }

    constexpr bool operator()(const T& lhs, const T& rhs) const noexcept
    {
        using MemberAccessors = decltype(memberAccessors_);
        return lexicographical_compare_(lhs, rhs, std::make_index_sequence<std::tuple_size<MemberAccessors>::value>{ });
    }
};
template <typename T>
    aggregate_less(tag<T>) -> aggregate_less<T>;
template <typename T, typename MetadataTagT>
    aggregate_less(tag<T>, const MetadataTagT&) -> aggregate_less<T, MetadataTagT>;
template <typename T, typename MetadataTagT, typename MemberLessT>
    aggregate_less(tag<T>, const MetadataTagT&, MemberLessT&&) -> aggregate_less<T, MetadataTagT, std::decay_t<MemberLessT>>;


    //ᅟ
    // Computes the hash of a struct by combining member hashes using metadata-based reflection.
    //
template <typename T, typename MetadataTagT = reflection_metadata_tag, template <typename...> class MemberHashT = hash>
    class aggregate_hash
{
private:
    static constexpr auto memberAccessors_ = makeshift::detail::get_member_accessors<T, MetadataTagT>();

    static std::size_t hash_combine_(std::size_t seed, std::size_t newHash) noexcept
    {
            // taken from boost::hash_combine()
        return seed ^ newHash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

public:
    constexpr aggregate_hash(void) noexcept = default;
    constexpr aggregate_hash(tag<T>, MetadataTagT = { }) noexcept { }

    constexpr std::size_t operator()(const T& obj) const noexcept
    {
        return memberAccessors_
            | tuple_fold(0, [&obj](std::size_t seed, auto&& accessor)
              {
                  using MemberType = typename std::decay_t<decltype(accessor)>::value_type;
                  using MemberHash = std::conditional_t<is_same_template_v<MemberHashT, hash>, hash<MemberType, MetadataTagT>, MemberHashT<MemberType>>;
                  return hash_combine_(seed, MemberHash{ }(accessor(obj)));
              });
    }
};
template <typename T>
    aggregate_hash(tag<T>) -> aggregate_hash<T>;
template <typename T, typename MetadataTagT>
    aggregate_hash(tag<T>, const MetadataTagT&) -> aggregate_hash<T, MetadataTagT>;


} // inline namespace metadata

} // namespace makeshift


#ifdef INCLUDED_MAKESHIFT_VARIANT_HPP_
 #include <makeshift/detail/variant_reflect.hpp>
#endif // INCLUDED_MAKESHIFT_VARIANT_HPP_


#endif // INCLUDED_MAKESHIFT_REFLECT_HPP_
