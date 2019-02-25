
#ifndef INCLUDED_MAKESHIFT_METADATA2_HPP_
#define INCLUDED_MAKESHIFT_METADATA2_HPP_


#include <array>
#include <tuple>
#include <utility>     // for move()
#include <type_traits> // for is_enum<>, common_type<>
#include <string_view>

#include <makeshift/type_traits2.hpp> // for type<>
#include <makeshift/version.hpp>      // for MAKESHIFT_NODISCARD


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


} // inline namespace metadata


namespace detail
{


template <typename T> struct unwrap_named_ { using type = T; };
template <typename T> struct unwrap_named_<named2<T>> { using type = T; };
template <typename T>
    constexpr named2<T> wrap_named(T value) noexcept
{
    return { std::move(value) };
}
template <typename T>
    constexpr named2<T> wrap_named(named2<T> value) noexcept
{
    return { std::move(value) };
}

struct value_metadata_base { };
struct compound_metadata_base { };

template <typename ValuesT>
    struct raw_value_metadata : private value_metadata_base
{
    ValuesT values;

    constexpr raw_value_metadata(ValuesT _values)
        : values{ std::move(_values) }
    {
    }
};
template <>
    struct raw_value_metadata<void> : private value_metadata_base
{
};


template <typename MembersT>
    struct raw_compound_metadata : private compound_metadata_base
{
    MembersT members;

    constexpr raw_compound_metadata(MembersT _members)
        : members{ std::move(_members) }
    {
    }
};


} // namespace detail


inline namespace metadata
{


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
