
#ifndef INCLUDED_MAKESHIFT_DETAIL_FUNCTIONAL_OPERATORS_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_FUNCTIONAL_OPERATORS_HPP_


#include <functional>  // for hash<>
#include <type_traits> // for is_default_constructible<>

#include <makeshift/type_traits.hpp> // for can_apply<>


namespace makeshift
{

inline namespace metadata
{

    // defined in reflect.hpp

template <typename T, typename MetadataTagT, template <typename...> class MemberHashT>
    class aggregate_hash;
template <typename T, typename MetadataTagT, typename MemberEqualToT>
    class aggregate_equal_to;
template <typename T, typename MetadataTagT, typename MemberLessT>
    class aggregate_less;

} // inline namespace metadata


namespace detail
{


template <typename KeyT> struct is_std_hash_enabled : std::is_default_constructible<std::hash<KeyT>> { };
template <typename KeyT> constexpr bool is_std_hash_enabled_v = is_std_hash_enabled<KeyT>::value;

template <typename KeyT, typename MetadataTagT> constexpr bool use_aggregate = type_category_of<KeyT, MetadataTagT> == type_category::aggregate;
template <typename KeyT, typename MetadataTagT> constexpr bool use_aggregate_hash = use_aggregate<KeyT, MetadataTagT> && !is_std_hash_enabled_v<KeyT>;


} // namespace detail


inline namespace types
{


    //ᅟ
    // Like `std::hash<>` but permits metadata-based hashing and conditional specialization with `enable_if<>`.
    // This is required to support specializations for e.g. constrained integers.
    //
template <typename KeyT, typename MetadataTagT = reflection_metadata_tag, typename = void>
    struct hash : std::conditional_t<makeshift::detail::use_aggregate_hash<KeyT, MetadataTagT>, aggregate_hash<KeyT, MetadataTagT, hash>, std::hash<KeyT>>
{
};


    //ᅟ
    // Like `std::equal_to<>` but permits metadata-based comparison.
    //
template <typename MetadataTagT = reflection_metadata_tag>
    struct equal_to
{
    template <typename T>
        constexpr bool operator()(const T& lhs, const T& rhs) const noexcept
    {
        if constexpr (makeshift::detail::use_aggregate<T, MetadataTagT>)
            return aggregate_equal_to<T, MetadataTagT, equal_to<MetadataTagT>>{ }(lhs, rhs);
        else
            return lhs == rhs;
    }
};


    //ᅟ
    // Like `std::less<>` but permits metadata-based comparison.
    //
template <typename MetadataTagT = reflection_metadata_tag>
    struct less
{
    template <typename T>
        constexpr bool operator()(const T& lhs, const T& rhs) const noexcept
    {
        if constexpr (makeshift::detail::use_aggregate<T, MetadataTagT>)
            return aggregate_less<T, MetadataTagT, less<MetadataTagT>>{ }(lhs, rhs);
        else
            return lhs < rhs;
    }
};


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_FUNCTIONAL_OPERATORS_HPP_
