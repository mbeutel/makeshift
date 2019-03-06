
#ifndef INCLUDED_MAKESHIFT_DETAIL_UTILITY2_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_UTILITY2_HPP_


#include <makeshift/type_traits.hpp>  // for tag<>
#include <makeshift/type_traits2.hpp> // for flags_base, flags_tag, type<>


namespace makeshift
{

namespace detail
{

namespace adl
{


template <typename FlagsT, typename UnderlyingTypeT>
    struct define_flags_base : makeshift::detail::flags_base
{
    using base_type = UnderlyingTypeT;
    enum class flags : UnderlyingTypeT { none = 0 };
    using flag = flags; // alias for declaring flag constants

    friend constexpr tag<FlagsT> flag_type_of_(flags, makeshift::detail::flags_tag) { return { }; }

        // We just forward the metadata defined for the derived type.
        // TODO: ensure that have_metadata<flag> is false if no metadata is defined for FlagsT.
    template <typename MetadataTagT>
        friend constexpr auto reflect(tag<flag>, MetadataTagT) -> decltype(reflect(tag<FlagsT>{ }, MetadataTagT{ }))
    {
        return reflect(tag<FlagsT>{ }, MetadataTagT{ });
    }

        // We just forward the metadata defined for the derived type.
        // TODO: ensure that have_metadata<flag> is false if no metadata is defined for FlagsT.
    template <typename U = FlagsT>
        friend constexpr auto reflect(type<flag>) -> decltype(reflect(type<U>{ }))
    {
        return reflect(type<FlagsT>{ });
    }
};


} // namespace adl

} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_UTILITY2_HPP_
