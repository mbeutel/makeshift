
#ifndef INCLUDED_MAKESHIFT_DETAIL_UTILITY_TYPES_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_UTILITY_TYPES_HPP_


#include <cstddef>     // for size_t, ptrdiff_t
#include <type_traits> // for integral_constant<>

#include <makeshift/type_traits.hpp> // for sequence<>

#include <makeshift/detail/workaround.hpp> // for csum<>(), cand()


namespace makeshift
{

inline namespace types
{


    //ᅟ
    // Represents an index value.
    //
using index_t = std::ptrdiff_t;


    //ᅟ
    // Represents a difference value.
    //
using diff_t = std::ptrdiff_t;


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
template <index_t... Is> struct index_constant : sequence<index_t, Is...> { };

    //ᅟ
    // Encodes a sequence of index values in the type of the expression.
    //
template <index_t... Is> constexpr index_constant<Is...> index_c{ };


    //ᅟ
    // Encodes a sequence of dimension values in a type.
    //
template <dim_t... Ds> struct shape_constant : sequence<dim_t, Ds...> { };

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
template <typename T, typename Is, char... Cs>
    struct make_constant_;
template <typename T, std::size_t... Is, char... Cs>
    struct make_constant_<T, std::index_sequence<Is...>, Cs...>
{
    static_assert(cand(Cs >= '0' && Cs <= '9'...), "invalid character: index must be an integral value");
    static constexpr T value = csum<T>((Cs - '0') * cpow10(Is, sizeof...(Cs))...);
};
template <typename T, char... Cs> struct make_constant : make_constant_<T, std::make_index_sequence<sizeof...(Cs)>, Cs...> { };
template <typename T, char... Cs> constexpr std::ptrdiff_t make_constant_v = make_constant<T, Cs...>::value;


template <auto I, auto V, typename = decltype(V)> using substitute = std::integral_constant<decltype(V), V>;
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
    // Encodes a value given as numeric literal in the type of the expression using `integral_constant<>`.
    //ᅟ
    //ᅟ    auto i = 42_c; // decltype(i) is integral_constant<long long, 42>
    //
template <char... Cs>
    constexpr inline std::integral_constant<long long, makeshift::detail::make_constant<long long, Cs...>::value>
    operator "" _c(void) noexcept
{
    return { };
}


    //ᅟ
    // Encodes an index value given as numeric literal in the type of the expression using `index_constant<>`.
    //ᅟ
    //ᅟ    auto i = 42_idx; // decltype(i) is index_constant<42>
    //
template <char... Cs>
    constexpr inline index_constant<makeshift::detail::make_constant<index_t, Cs...>::value>
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
    constexpr inline dim_constant<makeshift::detail::make_constant<dim_t, Cs...>::value>
    operator "" _dim(void) noexcept
{
    return { };
}


} // inline namespace literals

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_UTILITY_TYPES_HPP_
