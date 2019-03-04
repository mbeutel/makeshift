
#ifndef INCLUDED_MAKESHIFT_COMPOUND_HPP_
#define INCLUDED_MAKESHIFT_COMPOUND_HPP_


#include <tuple>       // for tuple_size<>
#include <cstddef>     // for size_t
#include <type_traits> // for decay<>
#include <utility>     // for move(), integer_sequence<>
#include <functional>  // for equal_to<>, less<>

#include <makeshift/reflect2.hpp>    // for compound_members_of()
#include <makeshift/tuple2.hpp>      // for tuple_all_of(), tuple_reduce()
#include <makeshift/functional2.hpp> // for hash<>
#include <makeshift/version.hpp>     // for MAKESHIFT_NODISCARD

#include <makeshift/detail/functional2.hpp> // for adapter_base<>


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
    // Equality comparer for compound types which determines equivalence by comparing members for equality.
    //
template <typename CompoundMembersT, typename EqualToT = std::equal_to<>>
    struct compound_equal_to : private makeshift::detail::adapter_base<EqualToT, CompoundMembersT>
{
    using makeshift::detail::adapter_base<EqualToT, CompoundMembersT>::adapter_base;

    template <typename T>
        MAKESHIFT_NODISCARD constexpr bool operator ()(const T& lhs, const T& rhs) const noexcept
    {
        const EqualToT& equal = *this;
        const CompoundMembersT& compoundMembers = *this;
        return tuple_all_of(
            compoundMembers(type_v<T>),
            [&equal, &lhs, &rhs](auto&& member)
            {
                return equal(member(lhs), member(rhs));
            });
    }
};


    //ᅟ
    // Hasher for compound types which computes a hash by combining the hashes of the members.
    //
template <typename CompoundMembersT, typename HashT = hash2<>>
    struct compound_hash : private makeshift::detail::adapter_base<HashT, CompoundMembersT>
{
    using makeshift::detail::adapter_base<HashT, CompoundMembersT>::adapter_base;

    template <typename T>
        MAKESHIFT_NODISCARD constexpr std::size_t operator ()(const T& obj) const noexcept
    {
        const HashT& hash = *this;
        const CompoundMembersT& compoundMembers = *this;
        return tuple_reduce(
            compoundMembers(type_v<T>),
            std::size_t(0),
            [&hash, &obj](std::size_t seed, auto&& member)
            {
                std::size_t memberValueHash = hash(member(obj));
                return makeshift::detail::hash_combine(seed, memberValueHash);
            });
    }
};


    //ᅟ
    // Ordering comparer for compound types which determines order by lexicographically comparing members.
    //
template <typename CompoundMembersT, typename LessT = std::less<>>
    struct compound_less : private makeshift::detail::adapter_base<LessT, CompoundMembersT>
{
    using makeshift::detail::adapter_base<LessT, CompoundMembersT>::adapter_base;

private:
    template <typename MembersT, typename T>
        constexpr bool invoke_(std::index_sequence<>, MembersT&&, const T&, const T&) const noexcept
    {
        return false;
    }
    template <std::size_t I0, std::size_t... Is, typename MembersT, typename T>
        constexpr bool invoke_(std::index_sequence<I0, Is...>, MembersT&& members, const T& lhs, const T& rhs) const noexcept
    {
        const auto& lhsMember = std::get<I0>(members)(lhs);
        const auto& rhsMember = std::get<I0>(members)(rhs);
        const LessT& less = *this;
        if (less(lhsMember, rhsMember)) return true;
        if (less(rhsMember, lhsMember)) return false;
        return invoke_(std::index_sequence<Is...>{ }, members, lhs, rhs);
    }

public:
    template <typename T>
        MAKESHIFT_NODISCARD constexpr bool operator ()(const T& lhs, const T& rhs) const noexcept
    {
        const CompoundMembersT& compoundMembers = *this;
        auto members = compoundMembers(type_v<T>);
        return invoke_(std::make_index_sequence<std::tuple_size<std::decay_t<decltype(members)>>::value>{ },
            members, lhs, rhs);
    }
};


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_COMPOUND_HPP_
