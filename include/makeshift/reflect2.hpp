
#ifndef INCLUDED_MAKESHIFT_REFLECT2_HPP_
#define INCLUDED_MAKESHIFT_REFLECT2_HPP_


#include <type_traits> // for integral_constant<>

#include <makeshift/version.hpp> // for MAKESHIFT_NODISCARD

#include <makeshift/detail/reflect2.hpp>


namespace makeshift
{

inline namespace metadata
{


template <typename T>
    struct metadata_of : makeshift::detail::metadata_of<T>
{
    using base_ = makeshift::detail::metadata_of<T>;
    using base_::base_;
};


template <typename T> constexpr inline metadata_of<T> metadata_of_v = { };


} // inline namespace metadata

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_REFLECT2_HPP_
