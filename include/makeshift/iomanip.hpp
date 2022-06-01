
#ifndef INCLUDED_MAKESHIFT_IOMANIP_HPP_
#define INCLUDED_MAKESHIFT_IOMANIP_HPP_


#include <type_traits>  // for remove_reference<>

#include <gsl-lite/gsl-lite.hpp>  // for gsl_CPP17_OR_GREATER

#if !gsl_CPP17_OR_GREATER
# error makeshift requires C++17 mode or higher
#endif // !gsl_CPP17_OR_GREATER

#include <makeshift/metadata.hpp>

#include <makeshift/detail/iomanip.hpp>


namespace makeshift {

namespace gsl = ::gsl_lite;


template <typename T, typename ReflectorT = reflector>
constexpr detail::enum_manipulator<T, ReflectorT>
as_enum(T&& value, ReflectorT = { })
{
    return { value };
}


template <typename T, typename ReflectorT = reflector>
constexpr detail::flags_manipulator<T, ReflectorT>
as_flags(T&& value, ReflectorT = { })
{
    return { value };
}


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_IOMANIP_HPP_
