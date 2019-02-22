
#ifndef INCLUDED_MAKESHIFT_METADATA2_HPP_
#define INCLUDED_MAKESHIFT_METADATA2_HPP_


#include <array>
#include <tuple>
#include <utility>     // for move()
#include <type_traits> // for is_enum<>, common_type<>
#include <string_view>

#include <makeshift/type_traits.hpp> // for tag<>

#include <makeshift/detail/workaround.hpp>    // for cand()
#include <makeshift/detail/utility_flags.hpp>


namespace makeshift
{

inline namespace metadata
{


    //ᅟ
    // Determines qualities of an enumeration type.
    //
struct enum_flag : define_flags<enum_flag>
{
        //ᅟ
        // Indicates that the enumeration type is a flags type.
        //
    static constexpr flag flags_enum { 1 };
};
using enum_flags = enum_flag::flags;


    //ᅟ
    // Determines qualities of a class type.
    //
struct class_flag : define_flags<class_flag>
{
        //ᅟ
        // Indicates that the class type is a value type (either a scalar type or a user-defined wrapper).
        //
    static constexpr flag value { 1 };

        //ᅟ
        // Indicates that the class type is a compound type, i.e. it has the semantics of a named2 tuple with regard to identity and comparability.
        // This type flag does not necessarily require aggregate-ness as defined in the C++ standard (`std::is_aggregate<>`), which imposes
        // unnecessary limitations (e.g. it may be reasonable for a compound type to have a user-defined constructor).
        //
    static constexpr flag compound { 2 };

        //ᅟ
        // Indicates that the class type is a composite type which itself forms a value, e.g. a geometrical point defined as `struct Point { int x, y; };`.
        //
    static constexpr flag compound_value = compound | value;
};
using class_flags = class_flag::flags;


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

    static constexpr named2<T> invoke(T value) noexcept
    {
        return { std::move(value), { } };
    }
};
template <typename T>
    struct as_named_value<named2<T>>
{
    using value_type = T;

    static constexpr named2<T> invoke(named2<T> value) noexcept
    {
        return std::move(value);
    }
};


template <enum_flags Flags, typename ValuesT>
    struct raw_enum_metadata
{
    static constexpr enum_flags flags = Flags;
    ValuesT values;
};
template <enum_flags Flags>
    struct raw_enum_metadata<Flags, void>
{
    static constexpr enum_flags flags = Flags;
};


template <class_flags Flags, typename MembersT>
    struct raw_class_metadata
{
    static constexpr class_flags flags = Flags;
    MembersT members;
};


} // namespace detail


inline namespace metadata
{


template <typename... Ts>
    constexpr auto flags_enumeration(Ts... args) noexcept
{
    static_assert(makeshift::detail::cand(std::is_enum<typename makeshift::detail::as_named_value<std::decay_t<Ts>>::value_type>::value...),
        "cannot use flags_enumeration() for arguments of non-enumeration type");
    using T = std::common_type_t<typename makeshift::detail::as_named_value<Ts>::value_type...>; // TODO: is this clean?
    using Values = std::array<named2<T>, sizeof...(Ts)>;
    return makeshift::detail::raw_enum_metadata<enum_flag::flags_enum, Values>{ { makeshift::detail::as_named_value<Ts>::invoke(args)... } };
}
constexpr inline auto flags_enumeration(void) noexcept
{
    return makeshift::detail::raw_enum_metadata<enum_flag::flags_enum, void>{ };
}

template <typename... Ts>
    constexpr auto enumeration(Ts... args) noexcept
{
    static_assert(makeshift::detail::cand(std::is_enum<typename makeshift::detail::as_named_value<std::decay_t<Ts>>::value_type>::value...),
        "cannot use flags_enumeration() for arguments of non-enumeration type");
    using T = std::common_type_t<typename makeshift::detail::as_named_value<Ts>::value_type...>; // TODO: is this clean?
    using Values = std::array<named2<T>, sizeof...(Ts)>;
    return makeshift::detail::raw_enum_metadata<enum_flags::none, Values>{ { makeshift::detail::as_named_value<Ts>::invoke(args)... } };
}
constexpr inline auto enumeration(void) noexcept
{
    return makeshift::detail::raw_enum_metadata<enum_flags::none, void>{ };
}


template <typename... Ts>
    constexpr auto compound(Ts... args) noexcept
{
    using Members = std::tuple<typename makeshift::detail::as_named_value<Ts>::value_type...>;
    return makeshift::detail::raw_class_metadata<class_flag::compound, Members>{ { makeshift::detail::as_named_value<Ts>::invoke(args)... } };
}

template <typename... Ts>
    constexpr auto compound_value(Ts... args) noexcept
{
    using Members = std::tuple<typename makeshift::detail::as_named_value<Ts>::value_type...>;
    return makeshift::detail::raw_class_metadata<class_flag::compound_value, Members>{ { makeshift::detail::as_named_value<Ts>::invoke(args)... } };
}


} // inline namespace metadata

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_METADATA2_HPP_
