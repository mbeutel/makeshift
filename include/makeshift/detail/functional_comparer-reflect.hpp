
#ifndef INCLUDED_MAKESHIFT_DETAIL_FUNCTIONAL_COMPARER_REFLECT_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_FUNCTIONAL_COMPARER_REFLECT_HPP_


#include <type_traits> // for integral_constant<>, enable_if<>, decay<>, is_same<>, is_enum<>
#include <utility>     // for integer_sequence<>, tuple_size<>

#include <makeshift/type_traits.hpp> // for is_instantiation_of<>
#include <makeshift/metadata.hpp>    // for metadata_of<>
#include <makeshift/reflect.hpp>     // for member_accessor()


namespace makeshift
{

namespace detail
{


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


#endif // INCLUDED_MAKESHIFT_DETAIL_FUNCTIONAL_COMPARER_REFLECT_HPP_
