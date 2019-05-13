
#ifndef INCLUDED_MAKESHIFT_METADATA2_HPP_
#define INCLUDED_MAKESHIFT_METADATA2_HPP_


#include <array> // for decay<>
#include <type_traits> // for decay<>

#include <makeshift/version.hpp> // for MAKESHIFT_NODISCARD

#include <makeshift/detail/string_view.hpp>
#include <makeshift/detail/metadata2.hpp>


namespace makeshift
{


MAKESHIFT_NODISCARD constexpr inline makeshift::detail::name_t name(std::string_view _name) noexcept
{
    return { _name };
}

template <typename T, typename... ParamsT>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::value_t<T, ParamsT...> value(T value, ParamsT... params)
{
    return { std::move(value), std::move(params)... };
}

template <typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto values(Ts... values)
{
    return makeshift::detail::values_impl_0<type_sequence<makeshift::detail::values_tag>>(std::move(values)...);
}

template <typename... ParamsT>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::parameter_set<ParamsT...> define_metadata(ParamsT... params)
{
    return { params... };
}


} // namespace makeshift


#endif // INCLUDED_MAKEHIFT_METADATA2_HPP_
