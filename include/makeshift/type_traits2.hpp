
#ifndef INCLUDED_MAKESHIFT_TYPE_TRAITS2_HPP_
#define INCLUDED_MAKESHIFT_TYPE_TRAITS2_HPP_


#include <iterator>    // for begin(), end()
#include <type_traits> // for declval<>()

#include <makeshift/type_traits.hpp> // for nth_type<>, try_index_of_type<>, can_apply<>


namespace makeshift
{

namespace detail
{


    // needed because `type<>` cannot have a member `type`
template <typename T> struct type_t { using type = T; };


namespace is_iterable_ns
{


using std::begin;
using std::end;


template <typename T> using is_iterable_r = decltype(begin(std::declval<T&>()) != end(std::declval<T&>()));


} // is_iterable_ns


} // namespace detail

inline namespace types
{


    //ᅟ
    // `type<>` is a generic type tag.
    //ᅟ
    // Use `type_v<T>` as a value representation of `T`.
    //
template <typename T> using type = makeshift::detail::type_t<T>;


    //ᅟ
    // Use `type_v<T>` as a value representation of `T` for tag dispatching.
    //
template <typename T> constexpr inline type<T> type_v { };


    //ᅟ
    // Type sequence, i.e. tuple without runtime value representation.
    //
template <typename... Ts>
    struct type_sequence2
{
    constexpr type_sequence2(void) noexcept = default;
    constexpr type_sequence2(type<Ts>...) noexcept { }
};
template <>
    struct type_sequence2<>
{
    constexpr type_sequence2(void) noexcept = default;
};


    //ᅟ
    // Return a type sequence that represents the types of the given values.
    //
template <typename... Ts>
    constexpr type_sequence2<Ts...> make_type_sequence2(type<Ts>...) noexcept
{
    return { };
}


    //ᅟ
    // Returns the `I`-th element in the type sequence.
    //
template <std::size_t I, typename... Ts>
    constexpr type<nth_type_t<I, Ts...>> get(const type_sequence2<Ts...>&) noexcept
{
    static_assert(I < sizeof...(Ts), "tuple index out of range");
    return { };
}

    //ᅟ
    // Returns the type sequence element of type `T`.
    //
template <typename T, typename... Ts>
    constexpr type<T> get(const type_sequence2<Ts...>& ts) noexcept
{
	constexpr std::size_t index = try_index_of_type_v<T, Ts...>;
    static_assert(index != std::size_t(-1), "type T does not appear in type sequence");
    return { };
}


    //ᅟ
    // Determines whether the given type is iterable, i.e. functions `begin()` and `end()` are well-defined for arguments of type `T`.
    //
template <typename T> struct is_iterable : can_apply<makeshift::detail::is_iterable_ns::is_iterable_r, T> { };

    //ᅟ
    // Determines whether the given type is iterable, i.e. functions `begin()` and `end()` are well-defined for arguments of type `T`.
    //
template <typename T> constexpr bool is_iterable_v = is_iterable<T>::value;


} // inline namespace types

} // namespace makeshift


namespace std
{


    // Specialize `tuple_size<>` and `tuple_element<>` for `type_sequence<>`.
template <typename... Ts> class tuple_size<makeshift::type_sequence2<Ts...>> : public std::integral_constant<std::size_t, sizeof...(Ts)> { };
template <std::size_t I, typename... Ts> class tuple_element<I, makeshift::type_sequence2<Ts...>> { using type = makeshift::type<makeshift::nth_type_t<I, Ts...>>; };


} // namespace std


#endif // INCLUDED_MAKESHIFT_TYPE_TRAITS2_HPP_
