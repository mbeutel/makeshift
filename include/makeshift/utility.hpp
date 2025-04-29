
#ifndef INCLUDED_MAKESHIFT_UTILITY_HPP_
#define INCLUDED_MAKESHIFT_UTILITY_HPP_


#include <cstddef>     // for size_t
#include <utility>     // for tuple_size<>, tuple_element<>
#include <type_traits> // for integral_constant<>

#include <gsl-lite/gsl-lite.hpp>  // for type_identity<>, gsl_CPP20_OR_GREATER

#if !gsl_CPP20_OR_GREATER
# error makeshift requires C++20 mode or higher
#endif // !gsl_CPP20_OR_GREATER

#include <makeshift/detail/type_traits.hpp>  // for constval_tag, type_sequence_cat_<>


namespace makeshift {

namespace gsl = ::gsl_lite;


    //
    // Type sequence, i.e. type list and tuple of `gsl::type_identity<>` arguments.
    //
template <typename... Ts>
struct type_sequence : detail::constval_tag
{
    constexpr type_sequence(void) noexcept { }
    constexpr type_sequence(gsl::type_identity<Ts>...) noexcept { }

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
template <typename... Ts>
type_sequence(gsl::type_identity<Ts>...) -> type_sequence<Ts...>;

    //
    // Type sequence, i.e. type list and tuple of `gsl::type_identity<>` arguments.
    //
template <typename... Ts>
constexpr type_sequence<Ts...>
type_sequence_c{ };

    //
    // Returns a type sequence that represents the types of the given values.
    //
template <typename... Ts>
constexpr type_sequence<Ts...>
make_type_sequence(gsl::type_identity<Ts>...) noexcept
{
    return { };
}

    //
    // Returns the `I`-th element in the type sequence.
    //
template <std::size_t I, typename... Ts>
constexpr gsl::type_identity<typename detail::nth_type_<I, Ts...>::type>
get(type_sequence<Ts...> const&) noexcept
{
    static_assert(I < sizeof...(Ts), "tuple index out of range");
    return { };
}

    //
    // Returns the type sequence element of type `T`.
    //
template <typename T, typename... Ts>
constexpr gsl::type_identity<T>
get(type_sequence<Ts...> const&) noexcept
{
    constexpr std::size_t index = detail::search_type_pack_index<T, Ts...>::value;
    static_assert(index != std::size_t(-1), "type T does not appear in type sequence");
    return { };
}


    //
    // Concatenates a sequence of type sequences.
    //
template <typename... Ts>
struct type_sequence_cat : detail::type_sequence_cat_<type_sequence<>, Ts...> { };

    //
    // Concatenates a sequence of type sequences.
    //
template <typename... Ts>
using type_sequence_cat_t = typename type_sequence_cat<Ts...>::type;


    //
    // Class that inherits from all its template arguments.
    //
template <typename... Ts>
struct composition // TODO: is this really needed?
{
};
template <typename... Ts>
composition(Ts...) -> composition<Ts...>;


template <typename T>
[[nodiscard]] consteval T
as_consteval(T value)
{
    return std::move(value);
}


} // namespace makeshift


    // Specialize `tuple_size<>` and `tuple_element<>` for `type_sequence<>`.
template <typename... Ts> class std::tuple_size<makeshift::type_sequence<Ts...>> : public std::integral_constant<std::size_t, sizeof...(Ts)> { };
template <std::size_t I, typename... Ts> class std::tuple_element<I, makeshift::type_sequence<Ts...>> { public: using type = typename makeshift::detail::nth_type_<I, Ts...>::type; };


#endif // INCLUDED_MAKESHIFT_UTILITY_HPP_
