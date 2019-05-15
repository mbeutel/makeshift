
#ifndef INCLUDED_MAKESHIFT_DETAIL_PROTOTYPES_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_PROTOTYPES_HPP_


#include <makeshift/version.hpp>     // for MAKESHIFT_EMPTY_BASES
#include <makeshift/type_traits.hpp> // for type<>


namespace makeshift
{

namespace detail
{


template <typename... MixinsT>
    struct MAKESHIFT_EMPTY_BASES aggregate : MixinsT...
{
    using members = type_sequence_cat_t<typename MixinsT::members...>;

    constexpr aggregate(MembersT... members)
        : MembersT(std::move(members))...
    {
    }
};


template <typename MemberT, typename MixinT>
    struct define_member
{
    using members = type_sequence<MemberT...>;
    constexpr friend const MixinT& _makeshift_select_member(const MixinT& mixin, MemberT) noexcept { return mixin; }
};



} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_PROTOTYPES_HPP_
