
#ifndef INCLUDED_MAKESHIFT_COMPOUND_HPP_
#define INCLUDED_MAKESHIFT_COMPOUND_HPP_


#include <tuple>       // for tuple_size<>
#include <cstddef>     // for size_t
#include <type_traits> // for decay<>
#include <utility>     // for move(), integer_sequence<>
#include <functional>  // for equal_to<>, less<>

#include <makeshift/reflect2.hpp> // for compound_members<>()
#include <makeshift/tuple2.hpp>   // for tuple_all_of(), tuple_reduce()
#include <makeshift/functional2.hpp> // for hash<>, adapter_base<>

#include <makeshift/detail/utility_flags.hpp> // for define_flags<>


namespace makeshift
{

namespace detail
{


static constexpr inline std::size_t hash_combine(std::size_t seed, std::size_t newHash) noexcept
{
        // taken from boost::hash_combine()
    return seed ^ (newHash + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}


} // namespace detail


inline namespace metadata
{


    //ᅟ
    // Hasher for compound types which computes a hash by combining the hashes of the members.
    //
template <typename HashT = hash2, typename CompoundMembersT = compound_members_t>
    struct compound_hash : private makeshift::detail::adapter_base<HashT, CompoundMembersT>
{
    using makeshift::detail::adapter_base::adapter_base;

    template <typename T>
        constexpr std::size_t operator ()(const T& obj) const noexcept
    {
        auto& hash = static_cast<const HashT&>(*this);
        auto& compoundMembers = static_cast<const CompoundMembersT&>(*this);
        return tuple_reduce(
            compoundMembers(type<T>),
            std::size_t(0),
            [&hash, &obj](std::size_t seed, auto&& member)
            {
                std::size_t memberValueHash = hash(get_member_value(obj, member));
                return makeshift::detail::hash_combine(seed, memberValueHash);
            });
    }
};


    //ᅟ
    // Equality comparer for compound types which determines equivalence by comparing members for equality.
    //
template <typename EqualToT = std::equal_to, typename CompoundMembersT = compound_members_t>
    struct compound_equal_to : private makeshift::detail::adapter_base<EqualToT, CompoundMembersT>
{
    using makeshift::detail::adapter_base::adapter_base;

    template <typename T>
        constexpr bool operator ()(const T& lhs, const T& rhs) const noexcept
    {
        auto& equalTo = static_cast<const EqualToT&>(*this);
        auto& compoundMembers = static_cast<const CompoundMembersT&>(*this);
        return tuple_all_of(
            compoundMembers(type<T>),
            [&equalTo, &obj](auto&& member)
            {
                return equalTo(get_member_value(lhs, member), get_member_value(rhs, member));
            });
    }
};


    //ᅟ
    // Ordering comparer for compound types which determines order by lexicographically comparing members.
    //
template <typename LessT = std::less, typename CompoundMembersT = compound_members_t>
    struct compound_less : private makeshift::detail::adapter_base<LessT, CompoundMembersT>
{
    using makeshift::detail::adapter_base::adapter_base;

private:
    template <typename MembersT, typename T>
        constexpr bool invoke_(std::index_sequence<>, MembersT&&, const T&, const T&) const noexcept
    {
        return false;
    }
    template <std::size_t I0, std::size_t... Is, typename MembersT, typename T>
        constexpr bool invoke_(std::index_sequence<I0, Is...>, MembersT&& members, const T& lhs, const T& rhs) const noexcept
    {
        const auto& lhsMember = get_member_value(lhs, std::get<I0>(members));
        const auto& rhsMember = get_member_value(rhs, std::get<I0>(members));
        const LessT& less = *this;
        if (less(lhsMember, rhsMember)) return true;
        if (less(rhsMember, lhsMember)) return false;
        return invoke_(std::index_sequence<Is...>{ }, members, lhs, rhs);
    }

public:
    template <typename T>
        constexpr bool operator ()(const T& lhs, const T& rhs) const noexcept
    {
        auto& compoundMembers = static_cast<const CompoundMembersT&>(*this);
        auto members = compoundMembers(type<T>);
        return invoke_(std::make_index_sequence<std::tuple_size<std::decay_t<decltype(members)>>::value>{ },
            members, lhs, rhs);
    }
};


/*struct regular_relation : define_flags<regular_relation>
{
    static constexpr flag equatable { 1 };
    static constexpr flag hashable { 2 };
    static constexpr flag comparable { 4 };
};
using regular_relations = regular_relation::flags;


template <regular_relations RegularRelations>*/


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_COMPOUND_HPP_
