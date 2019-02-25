
#ifndef INCLUDED_MAKESHIFT_METADATA2_HPP_
#define INCLUDED_MAKESHIFT_METADATA2_HPP_


#include <array>
#include <tuple>
#include <utility>     // for move()
#include <type_traits> // for common_type<>
#include <string_view>

#include <makeshift/version.hpp>      // for MAKESHIFT_NODISCARD

#include <makeshift/detail/metadata2.hpp> // for raw_value_metadata<>, raw_compound_metadata<>, unwrap_named_<>


namespace makeshift
{

inline namespace metadata
{


template <typename T>
    struct named2
{
    using value_type = T;

    T value;
    std::string_view name;
};

template <typename T>
    constexpr named2<T> with_name(T value, std::string_view name) noexcept
{
    return { std::move(value), name };
}


template <typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto reflect_values(Ts... args)
{
    using T = std::common_type_t<typename makeshift::detail::unwrap_named_<Ts>::type...>; // TODO: is this clean?
    using Values = std::array<named2<T>, sizeof...(Ts)>;
    return makeshift::detail::raw_value_metadata<Values>{ { makeshift::detail::wrap_named(std::move(args))... } };
}
MAKESHIFT_NODISCARD constexpr inline auto reflect_values(void)
{
    return makeshift::detail::raw_value_metadata<void>{ };
}


template <typename... Ts>
    MAKESHIFT_NODISCARD constexpr auto reflect_compound_members(Ts... args)
{
    using Members = std::tuple<named2<typename makeshift::detail::unwrap_named_<Ts>::type>...>;
    return makeshift::detail::raw_compound_metadata<Members>{ {  makeshift::detail::wrap_named(std::move(args))... } };
}


} // inline namespace metadata

} // namespace makeshift


#endif // INCLUDED_MAKEHIFT_METADATA2_HPP_
