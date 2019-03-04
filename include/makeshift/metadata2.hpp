
#ifndef INCLUDED_MAKESHIFT_METADATA2_HPP_
#define INCLUDED_MAKESHIFT_METADATA2_HPP_


#include <array>
#include <tuple>
#include <utility>     // for move()
#include <type_traits> // for common_type<>
#include <string_view>

#include <makeshift/version.hpp> // for MAKESHIFT_NODISCARD

#include <makeshift/detail/metadata2.hpp> // for values_initializer_t<>, named_values_initializer_t<>


namespace makeshift
{

inline namespace metadata
{


template <typename T>
    struct named2
{
    T value;
    std::string_view name;
};
template <typename T>
    named2(T&&, std::string_view) -> named2<std::decay_t<T>>;


template <typename T>
    constexpr inline makeshift::detail::values_initializer_t<T> values = { };


template <typename T>
    constexpr inline makeshift::detail::values_initializer_t<named2<T>> named_values = { };


} // inline namespace metadata

} // namespace makeshift


#endif // INCLUDED_MAKEHIFT_METADATA2_HPP_
