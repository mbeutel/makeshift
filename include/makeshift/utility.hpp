
#ifndef INCLUDED_MAKESHIFT_UTILITY_HPP_
#define INCLUDED_MAKESHIFT_UTILITY_HPP_


#include <gsl/gsl-lite.hpp> // for gsl_CPP17_OR_GREATER, gsl_NODISCARD

#if !gsl_CPP17_OR_GREATER
# include <tuple>      // for tuple_size<>, tuple_element<>
#endif // !gsl_CPP17_OR_GREATER

#include <cstddef>     // for size_t
#include <utility>     // for tuple_size<> (C++17), tuple_element<> (C++17)
#include <type_traits> // for integral_constant<>, enable_if<>, is_same<>, declval<>()

#include <makeshift/detail/utility.hpp>
#include <makeshift/detail/type_traits.hpp> // for constval_tag, is_tuple_like_r<>, unwrap_enum_tag, try_index_of_type<>


namespace makeshift
{


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
              std::enable_if_t<detail::try_index_of_type_in<T, typename TypeEnumTypeT::type::types>::value != -1, int> = 0>
    constexpr operator EnumT(void) const noexcept
    {
        return EnumT(int(detail::try_index_of_type_in<T, typename TypeEnumTypeT::type::types>::value));
    }

        // This must be a proxy type to work around the problem that a class cannot have a static constexpr member of its own type.
    static constexpr detail::type_tag_proxy<T> value{ };
};
template <typename T> constexpr detail::type_tag_proxy<T> type_tag<T>::value;

    //
    // Generic type tag.
    //
template <typename T> using type = type_tag<T>;

template <typename T1, typename T2>
gsl_NODISCARD constexpr std::is_same<T1, T2>
operator ==(type<T1>, type<T2>) noexcept
{
    return { };
}
template <typename T1, typename T2>
gsl_NODISCARD constexpr std::integral_constant<bool, !std::is_same<T1, T2>::value>
operator !=(type<T1>, type<T2>) noexcept
{
    return { };
}

    //
    // Use `type_c<T>` as a value representation of `T` for tag dispatching.
    //
template <typename T> constexpr type<T> type_c{ };


    //
    // Type sequence, i.e. type list and tuple of `type<>` arguments.
    //
template <typename... Ts>
struct type_sequence : detail::constval_tag
{
    constexpr type_sequence(void) noexcept { }
    constexpr type_sequence(type<Ts>...) noexcept { }

    constexpr type_sequence operator ()(void) const noexcept
    {
        return *this;
    }
};
template <>
struct type_sequence<> : detail::constval_tag
{
    constexpr type_sequence(void) noexcept { }

    constexpr type_sequence operator ()(void) const noexcept
    {
        return *this;
    }
};
#if gsl_CPP17_OR_GREATER
template <typename... Ts>
type_sequence(type<Ts>...) -> type_sequence<Ts...>;
#endif // gsl_CPP17_OR_GREATER

    //
    // Type sequence, i.e. type list and tuple of `type<>` arguments.
    //
template <typename... Ts> constexpr type_sequence<Ts...> type_sequence_c{ };

    //
    // Returns a type sequence that represents the types of the given values.
    //
template <typename... Ts>
constexpr type_sequence<Ts...> make_type_sequence(type<Ts>...) noexcept
{
    return { };
}

    //
    // Returns the `I`-th element in the type sequence.
    //
template <std::size_t I, typename... Ts>
constexpr type<typename detail::nth_type_<I, Ts...>::type> get(type_sequence<Ts...> const&) noexcept
{
    static_assert(I < sizeof...(Ts), "tuple index out of range");
    return { };
}

    //
    // Returns the type sequence element of type `T`.
    //
template <typename T, typename... Ts>
constexpr type<T> get(type_sequence<Ts...> const&) noexcept
{
	constexpr std::size_t index = detail::try_index_of_type<T, Ts...>::value;
    static_assert(index != std::size_t(-1), "type T does not appear in type sequence");
    return { };
}


    //
    // Concatenates a sequence of type sequences.
    //
template <typename... Ts> struct type_sequence_cat : detail::type_sequence_cat_<type_sequence<>, Ts...> { };

    //
    // Concatenates a sequence of type sequences.
    //
template <typename... Ts> using type_sequence_cat_t = typename type_sequence_cat<Ts...>::type;


} // namespace makeshift


namespace makeshift
{

namespace detail
{


template <typename T>
struct type_tag_inst
{
    static constexpr type_tag<T> value{ };
};
template <typename T> constexpr type_tag<T> type_tag_inst<T>::value;
template <typename T>
constexpr type_tag_proxy<T>::operator type_tag<T> const&(void) const noexcept
{
    return type_tag_inst<T>::value;
}


} // namespace detail

} // namespace makeshift


namespace std
{


    // Specialize `tuple_size<>` and `tuple_element<>` for `type_sequence<>`.
template <typename... Ts> class tuple_size<makeshift::type_sequence<Ts...>> : public std::integral_constant<std::size_t, sizeof...(Ts)> { };
template <std::size_t I, typename... Ts> class tuple_element<I, makeshift::type_sequence<Ts...>> { public: using type = makeshift::type<typename makeshift::detail::nth_type_<I, Ts...>::type>; };


} // namespace std


#endif // INCLUDED_MAKESHIFT_UTILITY_HPP_
