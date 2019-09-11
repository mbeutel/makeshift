
#ifndef INCLUDED_MAKESHIFT_DETAIL_COMPOUND_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_COMPOUND_HPP_


#include <tuple>       // for tuple_size<>
#include <cstddef>     // for size_t
#include <type_traits> // for decay<>
#include <utility>     // for move(), integer_sequence<>
#include <functional>  // for equal_to<>, less<>

#include <makeshift/reflect2.hpp>   // for compound_members_of()
#include <makeshift/tuple.hpp>      // for tuple_all_of(), tuple_reduce()
#include <makeshift/functional.hpp> // for hash<>
#include <makeshift/macros.hpp>     // for MAKESHIFT_NODISCARD, MAKESHIFT_DETAIL_EMPTY_BASES


namespace makeshift
{

namespace detail
{


static constexpr inline std::size_t hash_combine2(std::size_t seed, std::size_t newHash) noexcept
{
        // algorithm taken from boost::hash_combine()
    return seed ^ (newHash + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}


    //ᅟ
    // Equality comparer for compound types which determines equivalence by comparing members for equality.
    //
template <typename CompoundMembersT, typename EqualToT = std::equal_to<>>
    struct MAKESHIFT_DETAIL_EMPTY_BASES compound2_equal_to : CompoundMembersT, EqualToT
{
    constexpr compound2_equal_to(void) = default;
    constexpr compound2_equal_to(CompoundMembersT _compoundMembers, EqualToT _equal) : CompoundMembersT(std::move(_compoundMembers)), EqualToT(std::move(_equal)) { }

    template <typename T>
        MAKESHIFT_NODISCARD constexpr bool operator ()(const T& lhs, const T& rhs) const noexcept
    {
        const EqualToT& equal = *this;
        const CompoundMembersT& compoundMembers = *this;
        return makeshift::tuple_all_of(
            compoundMembers(type_c<T>),
            [&equal, &lhs, &rhs](auto&& member)
            {
                return equal(member(lhs), member(rhs));
            });
    }
};


    //ᅟ
    // Hasher for compound types which computes a hash by combining the hashes of the members.
    //
template <typename CompoundMembersT, typename HashT = hash<>>
    struct MAKESHIFT_DETAIL_EMPTY_BASES compound2_hash : CompoundMembersT, HashT
{
    constexpr compound2_hash(void) = default;
    constexpr compound2_hash(CompoundMembersT _compoundMembers, HashT _hash) : CompoundMembersT(std::move(_compoundMembers)), HashT(std::move(_hash)) { }

    template <typename T>
        MAKESHIFT_NODISCARD constexpr std::size_t operator ()(const T& obj) const noexcept
    {
        const HashT& hash = *this;
        const CompoundMembersT& compoundMembers = *this;
        return makeshift::tuple_reduce(
            compoundMembers(type_c<T>),
            std::size_t(0),
            [&hash, &obj](std::size_t seed, auto&& member)
            {
                std::size_t memberValueHash = hash(member(obj));
                return makeshift::detail::hash_combine2(seed, memberValueHash);
            });
    }
};


    //ᅟ
    // Ordering comparer for compound types which determines order by lexicographically comparing members.
    //
template <typename CompoundMembersT, typename LessT = std::less<>>
    struct MAKESHIFT_DETAIL_EMPTY_BASES compound2_less : CompoundMembersT, LessT
{
    constexpr compound2_less(void) = default;
    constexpr compound2_less(CompoundMembersT _compoundMembers, LessT _less) : CompoundMembersT(std::move(_compoundMembers)), LessT(std::move(_less)) { }

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
        auto members = compoundMembers(type_c<T>);
        return invoke_(std::make_index_sequence<std::tuple_size<std::decay_t<decltype(members)>>::value>{ },
            members, lhs, rhs);
    }
};


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_COMPOUND_HPP_
