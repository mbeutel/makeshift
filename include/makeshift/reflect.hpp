
#ifndef INCLUDED_MAKESHIFT_REFLECT_HPP_
#define INCLUDED_MAKESHIFT_REFLECT_HPP_


#include <makeshift/constval.hpp>    // for make_constval_t<>
#include <makeshift/type_traits.hpp> // for can_instantiate<>

#include <makeshift/detail/reflect.hpp>


namespace makeshift
{


template <typename T, typename = void> struct values_provider { };

template <typename T> struct have_values_of : disjunction<makeshift::detail::have_default_values<T>, makeshift::detail::have_reflect_values<T>, makeshift::detail::have_values_provider<T>> { };
template <typename T> constexpr bool have_values_of_v = have_values_of<T>::value;

template <typename T> using values_of_t = decltype(makeshift::detail::values_of_<T>{ }());

template <typename T> constexpr values_of_t<T> values_of = makeshift::detail::values_of_<T>{ }();

template <typename T> constexpr make_constval_t<makeshift::detail::values_of_<T>> values_of_c{ };


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_REFLECT_HPP_
