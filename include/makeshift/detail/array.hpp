
#ifndef INCLUDED_MAKESHIFT_DETAIL_ARRAY_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_ARRAY_HPP_


#include <array>
#include <tuple>
#include <cstddef>      // for size_t, ptrdiff_t
#include <utility>      // for integer_sequence<>, get<>()
#include <type_traits>  // for conjunction<>

#include <makeshift/utility.hpp>  // for type_sequence<>

#include <makeshift/detail/indices-2d.hpp>       // for indices_2d_
#include <makeshift/detail/ranges.hpp>           // for range_index_t
#include <makeshift/detail/type_traits.hpp>      // for can_instantiate_<>
#include <makeshift/detail/tuple-transform.hpp>  // for transform_element()


namespace makeshift {

namespace detail {


template <typename T> using is_array_like_r = decltype(std::declval<T>()[std::declval<std::ptrdiff_t>()]);
template <typename T> struct is_array_like_ : can_instantiate_<is_array_like_r, void, T> { };

template <typename T> struct is_homogeneous_arg_ : is_array_like_<T> { };
template <> struct is_homogeneous_arg_<range_index_t> : std::true_type { };

template <typename T> struct homogeneous_arg_type_ { using type = std::decay_t<is_array_like_r<T>>; };
template <> struct homogeneous_arg_type_<range_index_t> { using type = std::ptrdiff_t; };


template <typename F, std::size_t I, typename... Ts> struct result_type_ { using type = decltype(std::declval<F>()(detail::get_element<I>(std::declval<Ts>())...)); };

template <typename F, typename Is, typename... Ts>
struct result_types_;
template <typename F, std::size_t... Is, typename... Ts>
struct result_types_<F, std::index_sequence<Is...>, Ts...>
{
    using type = type_sequence<typename result_type_<F, Is, Ts...>::type...>;
};

template <typename R, typename T> struct transfer_ref_ { using type = T; };
template <typename R, typename T> struct transfer_ref_<R&, T> { using type = T&; };
template <typename R, typename T> struct transfer_ref_<R const&, T> { using type = T const&; };
template <typename R, typename T> struct transfer_ref_<R&&, T> { using type = T; };

template <std::ptrdiff_t N, bool HomogeneousArgs, typename F, typename... Ts>
struct homogeneous_result_;
template <std::ptrdiff_t N, typename F, typename... Ts>
struct homogeneous_result_<N, true, F, Ts...>
{
        // all arguments are array types or array indices, so we can just extract their value types
    using type = std::decay_t<decltype(std::declval<F>()(std::declval<typename transfer_ref_<Ts, typename homogeneous_arg_type_<std::decay_t<Ts>>::type>::type>()...))>;
};
template <typename F, typename Rs, typename... Ts>
struct check_homogeneous_result_;
template <typename F, typename... Rs, typename... Ts>
struct check_homogeneous_result_<F, type_sequence<Rs...>, Ts...>
{
    using Eq = equal_types_<Rs...>;
    static_assert(sizeof...(Rs) == 0 || Eq::value, "result types of functor must be identical for all sets of tuple elements");
    using type = typename Eq::common_type; // exists only if types are identical
};
template <std::ptrdiff_t N, typename F, typename... Ts>
struct homogeneous_result_<N, false, F, Ts...>
    : check_homogeneous_result_<F, typename result_types_<F, std::make_index_sequence<N>, Ts...>::type, Ts...>
{
        // some arguments are non-homogeneous, so we need to compute return types for all combinations and verify that they match exactly
};


template <std::size_t N, bool IsHomogeneous> struct array_for_index_arg;
template <std::size_t N> struct array_for_index_arg<N, true> : std::integral_constant<std::size_t, N> { };
template <std::size_t N> struct array_for_index_arg<N, false> : std::make_index_sequence<N> { };

template <template <typename, std::size_t> class ArrayT, typename F, typename... Ts>
constexpr auto
array_transform_impl(std::index_sequence<>, F&&, Ts&&...)
{
    // extra overload to avoid unused-parameter warning

    static_assert(std::conjunction<is_homogeneous_arg_<std::decay_t<Ts>>...>::value, "cannot infer array element type from empty tuple arguments");
    using R = typename homogeneous_result_<0, true, F, Ts...>::type;
    return ArrayT<R, 0>{ };
}
template <template <typename, std::size_t> class ArrayT, std::size_t... Is, typename F, typename... Ts>
constexpr auto
array_transform_impl(std::index_sequence<Is...>, F&& func, Ts&&... args)
{
    (void) func;
    using R = typename homogeneous_result_<sizeof...(Is), std::conjunction<is_homogeneous_arg_<std::decay_t<Ts>>...>::value, F, Ts...>::type;
    return ArrayT<R, sizeof...(Is)>{ detail::transform_element<Is>(func, std::forward<Ts>(args)...)... };
}
template <template <typename, std::size_t> class ArrayT, typename R, typename F, typename... Ts>
constexpr auto
array_transform_to_impl(std::index_sequence<>, F&&, Ts&&...)
{
    return ArrayT<R, 0>{ }; // extra overload to avoid unused-parameter warning
}
template <template <typename, std::size_t> class ArrayT, typename R, std::size_t... Is, typename F, typename... Ts>
constexpr auto
array_transform_to_impl(std::index_sequence<Is...>, F&& func, Ts&&... args)
{
    (void) func;
    return ArrayT<R, sizeof...(Is)>{ detail::transform_element<Is>(func, std::forward<Ts>(args)...)... };
}


template <template <typename, std::size_t> class ArrayT, typename T, typename IndicesT, std::size_t... Is, typename... Ts>
constexpr ArrayT<T, IndicesT::size>
array_cat_impl_1(std::index_sequence<Is...>, std::tuple<Ts...> tupleOfTuples)
{
    using std::get;
    return { get<IndicesT::col(Is)>(get<IndicesT::row(Is)>(std::move(tupleOfTuples)))... };
}
template <template <typename, std::size_t> class ArrayT, typename T, typename... TuplesT>
constexpr auto
array_cat_impl(TuplesT&&... tuples)
{
    using Indices = detail::indices_2d_<std::tuple_size<std::decay_t<TuplesT>>::value...>;
    return detail::array_cat_impl_1<ArrayT, T, Indices>(std::make_index_sequence<Indices::size>{ }, std::tuple<TuplesT&&...>{ std::forward<TuplesT>(tuples)... });
}


template <typename T, std::size_t... Dims> struct mdarray_;
template <typename T> struct mdarray_<T> { using type = T; };
template <typename T, std::size_t Dim0, std::size_t... Dims> struct mdarray_<T, Dim0, Dims...> { using type = std::array<typename mdarray_<T, Dims...>::type, Dim0>; };


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_ARRAY_HPP_
