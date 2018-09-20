
#ifndef INCLUDED_MAKESHIFT_DETAIL_FUNCTIONAL_COMPARER_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_FUNCTIONAL_COMPARER_HPP_


#include <cstddef>     // for size_t
#include <functional>  // for hash<>
#include <type_traits> // for is_default_constructible<>

#include <makeshift/type_traits.hpp> // for can_apply<>

#include <makeshift/detail/utility_chainable.hpp> // for define_chainable<>


namespace makeshift
{

inline namespace types
{


template <template <typename...> class ComparerT, typename BaseT, typename ComparerArgsT = void, typename... Ts>
    using define_comparer = define_chainable<ComparerT, BaseT, ComparerArgsT, void, Ts...>;


} // inline namespace types


inline namespace metadata
{


    //ᅟ
    // Use this class with `chain()` to inject a metadata tag into a metadata-based comparer.
    //
template <typename MetadataTagT = reflection_metadata_tag, typename BaseT = void>
    struct metadata_tag_for_comparer : define_comparer<metadata_tag_for_comparer, BaseT, void, MetadataTagT>
{
    using base = define_comparer<makeshift::metadata_tag_for_comparer, BaseT, void, MetadataTagT>;
    using base::base;

    using metadata_tag = MetadataTagT;
};
template <typename MetadataTagT = reflection_metadata_tag> constexpr metadata_tag_for_comparer<MetadataTagT> metadata_tag_for_comparer_v { };


    //ᅟ
    // Retrieves the metadata tag to be used for metadata-based comparers.
    // Defaults to `reflection_metadata_tag` if the user did not override the tag by chaining with a `metadata_tag_of_comparer<>`.
    //
template <typename ComparerT> struct metadata_tag_of_comparer : std::conditional_t<can_apply_v<makeshift::detail::metadata_tag_rt, ComparerT>, makeshift::detail::metadata_tag_r<ComparerT>, tag<reflection_metadata_tag>> { };

    //ᅟ
    // Retrieves the metadata tag to be used for metadata-based comparers.
    // Defaults to `reflection_metadata_tag` if the user did not override the tag by chaining with a `metadata_tag_of_comparer<>`.
    //
template <typename ComparerT> using metadata_tag_of_comparer_t = typename metadata_tag_of_comparer<ComparerT>::type;


} // inline namespace metadata


namespace detail
{


template <typename KeyT, typename MetadataTagT> constexpr bool is_aggregate = type_category_of<KeyT, MetadataTagT> == type_category::aggregate;


    // defined in reflect.hpp

template <typename T, typename ComparerT>
    constexpr bool aggregate_less(const T& lhs, const T& rhs, ComparerT&& cmp) noexcept;
template <typename T, typename ComparerT>
    constexpr bool aggregate_equal_to(const T& lhs, const T& rhs, ComparerT&& cmp) noexcept;
template <typename T, typename ComparerT>
    constexpr std::size_t aggregate_hash(const T& obj, ComparerT&& cmp) noexcept;


} // namespace detail


inline namespace types
{


    //ᅟ
    // Like `std::hash<>` but permits metadata-based hashing and conditional specialization with `enable_if<>`.
    // This is required to support specializations for e.g. constrained integers.
    //
template <typename KeyT, typename = void>
    struct default_hash : std::hash<KeyT>
{
    using std::hash<KeyT>::hash;
};


    //ᅟ
    // Default comparer which supports built-in types and aggregate types.
    //
template <typename BaseT = void>
    struct comparer : define_comparer<comparer, BaseT>
{
    using base = define_comparer<makeshift::comparer, BaseT>;
    using base::base;

    template <typename T, typename ComparerT>
        friend constexpr bool less_impl(const T& lhs, const T& rhs, const comparer&, ComparerT&& cmp) noexcept
    {
        (void) cmp;
        using MetadataTag = metadata_tag_of_comparer_t<std::decay_t<ComparerT>>;
        if constexpr (makeshift::detail::is_aggregate<T, MetadataTag>)
            return makeshift::detail::aggregate_less(lhs, rhs, cmp);
        else
            return lhs < rhs;
    }
    template <typename T, typename ComparerT>
        friend constexpr bool equal_to_impl(const T& lhs, const T& rhs, const comparer&, ComparerT&& cmp) noexcept
    {
        (void) cmp;
        using MetadataTag = metadata_tag_of_comparer_t<std::decay_t<ComparerT>>;
        if constexpr (makeshift::detail::is_aggregate<T, MetadataTag>)
            return makeshift::detail::aggregate_equal_to(lhs, rhs, cmp);
        else
            return lhs == rhs;
    }
    template <typename T, typename ComparerT>
        friend constexpr std::size_t hash_impl(const T& obj, const comparer&, ComparerT&& cmp) noexcept
    {
        (void) cmp;
        using MetadataTag = metadata_tag_of_comparer_t<std::decay_t<ComparerT>>;
        if constexpr (makeshift::detail::is_aggregate<T, MetadataTag>)
            return makeshift::detail::aggregate_hash(obj, cmp);
        else
            return default_hash<T>{ }(obj);
    }
};
comparer(void) -> comparer<>;


    //ᅟ
    // Function object similar to `std::hash<>` which uses the given comparer for hashing.
    // Uses `comparer<>` by default, which supports built-in types and aggregate types.
    //
template <typename ComparerT = comparer<>>
    struct hash : private ComparerT
{
    constexpr hash(void) noexcept = default;
    constexpr hash(const ComparerT& cmp) : ComparerT(cmp) { }
    constexpr hash(ComparerT&& cmp) : ComparerT(std::move(cmp)) { }

    template <typename T>
        constexpr std::size_t operator ()(const T& obj) const noexcept
    {
        const ComparerT& cmp = *this;
        return hash_impl(obj, cmp, cmp);
    }
};
hash(void) -> hash<>;
template <typename ComparerT>
    hash(ComparerT&&) -> hash<std::decay_t<ComparerT>>;


    //ᅟ
    // Function object similar to `std::equal_to<>` which uses the given comparer.
    // Uses `comparer<>` by default, which supports built-in types and aggregate types.
    //
template <typename ComparerT = comparer<>>
    struct equal_to : private ComparerT
{
    constexpr equal_to(void) noexcept = default;
    constexpr equal_to(const ComparerT& cmp) : ComparerT(cmp) { }
    constexpr equal_to(ComparerT&& cmp) : ComparerT(std::move(cmp)) { }

    template <typename T>
        constexpr bool operator ()(const T& lhs, const T& rhs) const noexcept
    {
        const ComparerT& cmp = *this;
        return equal_to_impl(lhs, rhs, cmp, cmp);
    }
};
equal_to(void) -> equal_to<>;
template <typename ComparerT>
    equal_to(ComparerT&&) -> equal_to<std::decay_t<ComparerT>>;


    //ᅟ
    // Function object similar to `std::less<>` which uses the given comparer.
    // Uses `comparer<>` by default, which supports built-in types and aggregate types.
    //
template <typename ComparerT = comparer<>>
    struct less : private ComparerT
{
    constexpr less(void) noexcept = default;
    constexpr less(const ComparerT& cmp) : ComparerT(cmp) { }
    constexpr less(ComparerT&& cmp) : ComparerT(std::move(cmp)) { }

    template <typename T>
        constexpr bool operator ()(const T& lhs, const T& rhs) const noexcept
    {
        const ComparerT& cmp = *this;
        return less_impl(lhs, rhs, cmp, cmp);
    }
};
less(void) -> less<>;
template <typename ComparerT>
    less(ComparerT&&) -> less<std::decay_t<ComparerT>>;


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_FUNCTIONAL_COMPARER_HPP_
