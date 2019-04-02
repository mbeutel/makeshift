
#ifndef INCLUDED_MAKESHIFT_METADATA2_HPP_
#define INCLUDED_MAKESHIFT_METADATA2_HPP_


#include <array> // for decay<>
#include <type_traits> // for decay<>

#include <makeshift/version.hpp> // for MAKESHIFT_NODISCARD

#include <makeshift/detail/string_view.hpp>
#include <makeshift/detail/metadata2.hpp>


namespace makeshift
{

inline namespace metadata
{


MAKESHIFT_NODISCARD constexpr inline makeshift::detail::name_t name(std::string_view _name) noexcept
{
    return { _name };
}

template <typename T, typename... AttributesT>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::value_t<T, AttributesT...> value(T value, AttributesT... attributes)
{
    return { std::move(value), std::move(attributes)... };
}

template <typename T>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::values_t<T, 0> values(void)
{
    return { };
}
MAKESHIFT_NODISCARD constexpr inline makeshift::detail::heterogeneous_values_t<std::tuple<>> values(void)
{
    return { { } };
}
template <typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto values(Ts... values)
{
    return makeshift::detail::values_raw_impl(makeshift::detail::equal_types_<Ts...>{ }, std::move(values)...);
}
template <typename... Ts, typename... ParamsT>
    MAKESHIFT_NODISCARD constexpr auto values(makeshift::detail::value_t<Ts, ParamsT...>... values)
{
    return makeshift::detail::values_impl(makeshift::detail::equal_types_<Ts...>{ }, std::move(values)...);
}

template <typename... ParamsT>
    MAKESHIFT_NODISCARD constexpr makeshift::detail::parameter_set<ParamsT...> define_metadata(ParamsT... params)
{
    return { params... };
}


} // inline namespace metadata

} // namespace makeshift


#endif // INCLUDED_MAKEHIFT_METADATA2_HPP_
