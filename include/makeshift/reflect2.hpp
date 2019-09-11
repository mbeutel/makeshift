
#ifndef INCLUDED_MAKESHIFT_REFLECT2_HPP_
#define INCLUDED_MAKESHIFT_REFLECT2_HPP_


#include <type_traits> // for integral_constant<>

#include <makeshift/macros.hpp> // for MAKESHIFT_NODISCARD

#include <makeshift/detail/reflect2.hpp>


namespace makeshift
{


template <typename T>
    struct metadata2_of : makeshift::detail::metadata_of_0<T, makeshift::detail::declares_metadata<T>::value>
{
    using base_ = makeshift::detail::metadata_of_0<T, makeshift::detail::declares_metadata<T>::value>;
    using base_::base_;
};


template <typename T> constexpr inline metadata2_of<T> metadata2_of_v = { };


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_REFLECT2_HPP_
