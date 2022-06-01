
#ifndef INCLUDED_MAKESHIFT_EXPERIMENTAL_UTILITY_HPP_
#define INCLUDED_MAKESHIFT_EXPERIMENTAL_UTILITY_HPP_


#include <type_traits> // for integral_constant<>, enable_if<>, is_same<>, declval<>()

#include <gsl-lite/gsl-lite.hpp>  // for gsl_CPP17_OR_GREATER

#if !gsl_CPP17_OR_GREATER
# error makeshift requires C++17 mode or higher
#endif // !gsl_CPP17_OR_GREATER

#include <makeshift/detail/type_traits.hpp>  // for unwrap_enum_tag, search_type_pack_index<>

#include <makeshift/experimental/detail/utility.hpp>


namespace makeshift {

namespace gsl = ::gsl_lite;


    //
    // Generic type tag.
    //ᅟ
    // Always refer to this type by the `type<>` alias template. This type exists for technical reasons only (because we want the alias `type<T>::type`).
    //
template <typename T>
struct type_tag : detail::type_base<T>
{
    constexpr type_tag operator ()(void) const noexcept
    {
        return *this;
    }

        // This conversion exists so expressions of type `type<>` can be used as case labels of switch statements over type enums.
    template <typename EnumT,
              typename TypeEnumTypeT = decltype(type_enum_type_of_(std::declval<EnumT>(), detail::unwrap_enum_tag{ })),
              std::enable_if_t<detail::search_type_pack_index_in<T, typename TypeEnumTypeT::type::types>::value != -1, int> = 0>
    constexpr operator
    EnumT(void) const noexcept
    {
        return EnumT(int(detail::search_type_pack_index_in<T, typename TypeEnumTypeT::type::types>::value));
    }

        // This must be a proxy type to work around the problem that a class cannot have a static constexpr member of its own type.
    static constexpr detail::type_tag_proxy<T> value{ };
};
template <typename T>
constexpr detail::type_tag_proxy<T> type_tag<T>::value;

    //
    // Generic type tag.
    //
template <typename T> using type = type_tag<T>;

template <typename T1, typename T2>
[[nodiscard]] constexpr std::is_same<T1, T2>
operator ==(type<T1>, type<T2>) noexcept
{
    return { };
}
template <typename T1, typename T2>
[[nodiscard]] constexpr std::integral_constant<bool, !std::is_same<T1, T2>::value>
operator !=(type<T1>, type<T2>) noexcept
{
    return { };
}

    //
    // Class that can be constructed from any value via user-defined conversion sequence.
    // Useful for defining fallback overloads.
    //
    //     template <typename WidgetT>
    //     auto color(WidgetT widget) -> decltype(widget.color()) { ... }
    //     Color color(any_sink) { return Color::black }
    //
struct any_sink
{
    template <typename T>
    constexpr any_sink(T&&) noexcept
    {
    }
};


    //
    // Class from which any value can be constructed via user-defined conversion sequence.
    //
struct any_source
{
    template <typename T>
    constexpr operator T(void) const noexcept
    {
    }
};


    //
    // Use `type_c<T>` as a value representation of `T` for tag dispatching.
    //
template <typename T>
constexpr type<T>
type_c{ };


namespace detail {


template <typename T>
struct type_tag_inst
{
    static constexpr type_tag<T> value{ };
};
template <typename T> constexpr type_tag<T> type_tag_inst<T>::value;
template <typename T>
constexpr
type_tag_proxy<T>::operator type_tag<T> const&(void) const noexcept
{
    return type_tag_inst<T>::value;
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_EXPERIMENTAL_UTILITY_HPP_
