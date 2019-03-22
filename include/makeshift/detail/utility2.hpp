
#ifndef INCLUDED_MAKESHIFT_DETAIL_UTILITY2_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_UTILITY2_HPP_


#include <gsl/gsl_assert> // for Expects()

#include <makeshift/type_traits.hpp>  // for tag<>
#include <makeshift/type_traits2.hpp> // for flags_base, unwrap_enum_tag, type<>, type_sequence<>


namespace makeshift
{

namespace detail
{

namespace adl
{


template <typename FlagsT, typename UnderlyingTypeT>
    struct define_flags_base : makeshift::detail::flags_base
{
    using base_type = UnderlyingTypeT;
    enum class flags : UnderlyingTypeT { none = 0 };
    using flag = flags; // alias for declaring flag constants

    friend constexpr type<FlagsT> flag_type_of_(flags, makeshift::detail::unwrap_enum_tag) { return { }; }

        // We just forward the metadata defined for the derived type.
        // TODO: ensure that have_metadata<flag> is false if no metadata is defined for FlagsT.
    template <typename MetadataTagT>
        friend constexpr auto reflect(tag<flag>, MetadataTagT) -> decltype(reflect(tag<FlagsT>{ }, MetadataTagT{ }))
    {
        return reflect(tag<FlagsT>{ }, MetadataTagT{ });
    }

        // We just forward the metadata defined for the derived type.
        // TODO: ensure that have_metadata<flag> is false if no metadata is defined for FlagsT.
    template <typename U = FlagsT>
        friend constexpr auto reflect(type<flag>) -> decltype(reflect(type<U>{ }))
    {
        return reflect(type<FlagsT>{ });
    }
};


template <typename TypeEnumT, typename... Ts>
    struct define_type_enum_base : makeshift::detail::type_enum_base
{
private:
    enum class value_t : int { };

    value_t value_;

public:
    friend constexpr type<TypeEnumT> type_enum_type_of_(value_t, makeshift::detail::unwrap_enum_tag) { return { }; }

    using types = type_sequence2<Ts...>;
    static constexpr std::size_t size = sizeof...(Ts);

    constexpr define_type_enum_base(const define_type_enum_base&) = default;
    constexpr define_type_enum_base& operator =(const define_type_enum_base&) = default;

    template <typename T,
              typename = std::enable_if_t<try_index_of_type_v<T, Ts...> != -1>>
        constexpr define_type_enum_base(type<T>) noexcept
            : value_(value_t(int(try_index_of_type_v<T, Ts...>)))
    {
    }

    explicit constexpr define_type_enum_base(int _value)
        : value_(value_t(_value))
    {
        Expects(_value >= 0 && _value < int(sizeof...(Ts)));
    }

    operator bool(void) const = delete;

        // This conversion exists so type enums can be used in switch statements.
    constexpr operator value_t(void) const noexcept { return value_; }

    constexpr explicit operator int(void) const noexcept { return int(value_); }

    friend constexpr bool operator ==(define_type_enum_base lhs, define_type_enum_base rhs) noexcept
    {
        return lhs.value_ == rhs.value_;
    }
    friend constexpr bool operator !=(define_type_enum_base lhs, define_type_enum_base rhs) noexcept
    {
        return !(lhs == rhs);
    }
};

template <typename TypeEnumT, typename... Ts, typename T,
          typename = std::enable_if_t<try_index_of_type_v<T, Ts...> != -1>>
    constexpr bool operator ==(define_type_enum_base<TypeEnumT, Ts...> lhs, type<T>) noexcept
{
    return int(try_index_of_type_v<T, Ts...>) == int(lhs);
}
template <typename TypeEnumT, typename... Ts, typename T,
          typename = std::enable_if_t<try_index_of_type_v<T, Ts...> != -1>>
    constexpr bool operator ==(type<T> lhs, define_type_enum_base<TypeEnumT, Ts...> rhs) noexcept
{
    return rhs == lhs;
}
template <typename TypeEnumT, typename... Ts, typename T,
          typename = std::enable_if_t<try_index_of_type_v<T, Ts...> != -1>>
    constexpr bool operator !=(define_type_enum_base<TypeEnumT, Ts...> lhs, type<T> rhs) noexcept
{
    return !(lhs == rhs);
}
template <typename TypeEnumT, typename... Ts, typename T,
          typename = std::enable_if_t<try_index_of_type_v<T, Ts...> != -1>>
    constexpr bool operator !=(type<T> lhs, define_type_enum_base<TypeEnumT, Ts...> rhs) noexcept
{
    return !(rhs == lhs);
}


} // namespace adl

} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_UTILITY2_HPP_
