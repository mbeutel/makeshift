
#ifndef MAKESHIFT_DETAIL_UTILITY_TYPES_HPP_
#define MAKESHIFT_DETAIL_UTILITY_TYPES_HPP_


#include <cstddef>     // for ptrdiff_t
#include <type_traits> // for integral_constant<>
#include <utility>     // for integer_sequence<>


namespace makeshift
{

inline namespace types
{


    //ᅟ
    // Represents an index value.
    //
using index_t = std::ptrdiff_t;


    //ᅟ
    // Represents an array stride.
    //
using stride_t = std::ptrdiff_t;


    //ᅟ
    // Represents a dimension value.
    //
using dim_t = std::ptrdiff_t;


    //ᅟ
    // Encodes a sequence of index values in a type.
    //
template <index_t... Is> struct index_constant : std::integer_sequence<index_t, Is...> { };


    //ᅟ
    // Encodes a sequence of index values in the type of the expression.
    //
template <index_t... Is> constexpr index_constant<Is...> index_c{ };


    //ᅟ
    // Encodes a sequence of dimension values in a type.
    //
template <dim_t... Ds> struct shape_constant : std::integer_sequence<dim_t, Ds...> { };


    //ᅟ
    // Encodes a sequence of dimension values in the type of the expression.
    //
template <dim_t... Ds> constexpr shape_constant<Ds...> shape_c{ };


    //ᅟ
    // Encodes a dimension value in a type.
    //
template <dim_t D> using dim_constant = shape_constant<D>;


    //ᅟ
    // Encodes a dimension value in the type of the expression.
    //
template <dim_t D> constexpr dim_constant<D> dim_c{ };


} // inline namespace types


namespace detail
{


constexpr inline std::ptrdiff_t cpow10(int I, int N) noexcept
{
    std::ptrdiff_t result = 1;
    for (int n = 0; n != N - 1 - I; ++n)
        result *= 10;
    return result;
}
template <typename... Ts>
    constexpr inline std::ptrdiff_t csum(Ts... vs) noexcept // workaround to make VC++ accept non-trivial fold expressions
{
    auto term = std::ptrdiff_t(0);
    return (vs + ... + term);
}
template <typename... Ts>
    constexpr inline bool cand(Ts... vs) noexcept // workaround to make VC++ accept non-trivial fold expressions
{
    return (vs && ...);
}
template <typename Is, char... Cs>
    struct make_index_constant_;
template <std::size_t... Is, char... Cs>
    struct make_index_constant_<std::index_sequence<Is...>, Cs...>
{
    static_assert(cand(Cs >= '0' && Cs <= '9'...), "invalid character: index must be an integral value");
    static constexpr std::ptrdiff_t value = csum((Cs - '0') * cpow10(Is, sizeof...(Cs))...);
};
template <char... Cs> struct make_index_constant : make_index_constant_<std::make_index_sequence<sizeof...(Cs)>, Cs...> { };
template <char... Cs> constexpr std::ptrdiff_t make_index_constant_v = make_index_constant<Cs...>::value;


template <auto I, auto V> using substitute = std::integral_constant<decltype(V), V>;
template <typename Is> struct zero_index_0_;
template <std::size_t... Is> struct zero_index_0_<std::index_sequence<Is...>> { using type = index_constant<substitute<Is, std::ptrdiff_t(0)>::value...>; };
template <dim_t Dim> struct zero_index_ : zero_index_0_<std::make_index_sequence<Dim>> { };


} // namespace detail


inline namespace types
{


    //ᅟ
    // A multi-index of dimension `Dim` with zero-valued entries.
    //ᅟ
    //ᅟ    using I0 = zero_index_t<3>; // I0 is index_constant<0, 0, 0>
    //
template <dim_t Dim> using zero_index_t = typename makeshift::detail::zero_index_<Dim>::type;

    //ᅟ
    // Constructs a multi-index of dimension `Dim` with zero-valued entries.
    //ᅟ
    //ᅟ    auto i0 = zero_index_c<3>; // decltype(i0) is index_constant<0, 0, 0>
    //
template <dim_t Dim> constexpr zero_index_t<Dim> zero_index_c{ };


} // inline namespace types


inline namespace literals
{


    //ᅟ
    // Encodes an index value given as numeric literal in the type of the expression using `index_constant<>`.
    //ᅟ
    //ᅟ    auto i = 42_idx; // decltype(i) is index_constant<42>
    //
template <char... Cs>
    constexpr inline index_constant<makeshift::detail::make_index_constant<Cs...>::value>
    operator "" _idx(void) noexcept
{
    return { };
}


    //ᅟ
    // Encodes a dimension value given as numeric literal in the type of the expression using `dim_constant<>`.
    //ᅟ
    //ᅟ    auto d = 3_dim; // decltype(i) is dim_constant<3>
    //
template <char... Cs>
    constexpr inline dim_constant<makeshift::detail::make_index_constant<Cs...>::value>
    operator "" _dim(void) noexcept
{
    return { };
}


} // inline namespace literals

} // namespace makeshift


#endif // MAKESHIFT_DETAIL_UTILITY_TYPES_HPP_
