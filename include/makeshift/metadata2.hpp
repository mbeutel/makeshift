
#ifndef INCLUDED_MAKESHIFT_METADATA2_HPP_
#define INCLUDED_MAKESHIFT_METADATA2_HPP_


#include <array>
#include <tuple>
#include <utility>     // for move()
#include <type_traits> // for is_enum<>, common_type<>
#include <string_view>

#include <makeshift/type_traits2.hpp> // for type<>

#include <makeshift/detail/workaround.hpp> // for cand()


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
    constexpr named2<T> with_name(T value, std::string_view name) noexcept
{
    return { std::move(value), name };
}


} // inline namespace metadata


namespace detail
{


template <typename T>
    struct as_named_value
{
    using value_type = T;

    static constexpr named2<T> invoke(T value)
    {
        return { std::move(value), { } };
    }
};
template <typename T>
    struct as_named_value<named2<T>>
{
    using value_type = T;

    static constexpr named2<T> invoke(named2<T> value)
    {
        return std::move(value);
    }
};


template <typename ValuesT>
    struct raw_enum_metadata
{
    ValuesT values;
};
template <>
    struct raw_enum_metadata<void>
{
};


template <typename MembersT>
    struct raw_class_metadata
{
    MembersT members;
};


} // namespace detail


inline namespace metadata
{


template <typename... Ts>
    constexpr auto reflect_enum_values(Ts... args)
{
    static_assert(makeshift::detail::cand(std::is_enum<typename makeshift::detail::as_named_value<std::decay_t<Ts>>::value_type>::value...),
        "cannot use enumeration() for arguments of non-enumeration type");
    using T = std::common_type_t<typename makeshift::detail::as_named_value<Ts>::value_type...>; // TODO: is this clean?
    using Values = std::array<named2<T>, sizeof...(Ts)>;
    return makeshift::detail::raw_enum_metadata<Values>{ { makeshift::detail::as_named_value<Ts>::invoke(args)... } };
}
constexpr inline auto reflect_enum_values(void)
{
    return makeshift::detail::raw_enum_metadata<void>{ };
}


template <typename... Ts>
    constexpr auto reflect_compound_members(Ts... args)
{
    using Members = std::tuple<named2<typename makeshift::detail::as_named_value<Ts>::value_type>...>;
    return makeshift::detail::raw_class_metadata<Members>{ { makeshift::detail::as_named_value<Ts>::invoke(args)... } };
}


} // inline namespace metadata

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_METADATA2_HPP_
