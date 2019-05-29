
#ifndef INCLUDED_MAKESHIFT_DETAIL_TUPLE2_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_TUPLE2_HPP_


#include <tuple>       // for make_tuple()
#include <cstddef>     // for size_t
#include <utility>     // for forward<>(), integer_sequence<>, tuple_size<>, get<>()
#include <type_traits> // for decay<>

#include <makeshift/detail/tuple2-transform.hpp>


namespace makeshift
{

namespace detail
{


template <typename F, typename... Ts>
    constexpr std::tuple<>
    tuple_transform_impl1(transform_to_tuple_tag, std::index_sequence<>, F&&, Ts&&...)
{
    return { }; // extra overload to avoid unused-parameter warning
}
template <std::size_t... Is, typename F, typename... Ts>
    constexpr auto
    tuple_transform_impl1(transform_to_tuple_tag, std::index_sequence<Is...>, F&& func, Ts&&... args)
{
    (void) func;
    return std::make_tuple(makeshift::detail::tuple_transform_impl2<Is>(func, std::forward<Ts>(args)...)...);
}


struct left_fold { };
struct right_fold { };
struct all_fold { };
struct any_fold { };

template <typename FoldT, typename TupleT, typename T, typename F>
    constexpr auto fold_impl(std::index_sequence<>, FoldT, TupleT&&, T&& initialValue, F&&)
{
    return std::forward<T>(initialValue);
}
template <std::size_t I0, std::size_t... Is, typename TupleT, typename T, typename F>
    constexpr auto
    fold_impl(std::index_sequence<I0, Is...>, left_fold, TupleT&& tuple, T&& initialValue, F&& func)
{
    return makeshift::detail::fold_impl(std::index_sequence<Is...>{ }, left_fold{ }, std::forward<TupleT>(tuple),
        func(std::forward<T>(initialValue), std::get<I0>(std::forward<TupleT>(tuple))),
        std::forward<F>(func));
}
template <std::size_t I0, std::size_t... Is, typename TupleT, typename T, typename F>
    constexpr auto
    fold_impl(std::index_sequence<I0, Is...>, right_fold, TupleT&& tuple, T&& initialValue, F&& func)
{
    return makeshift::detail::fold_impl(std::index_sequence<Is...>{ }, right_fold{ }, std::forward<TupleT>(tuple),
        func(std::get<std::tuple_size<std::decay_t<TupleT>>::value - 1 - I0>(std::forward<TupleT>(tuple)), std::forward<T>(initialValue)),
        std::forward<F>(func));
}
template <std::size_t I0, std::size_t... Is, typename TupleT, typename F>
    constexpr auto
    fold_impl(std::index_sequence<I0, Is...>, all_fold, TupleT&& tuple, bool, F&& func)
{
    return func(std::get<I0>(std::forward<TupleT>(tuple)))
        && makeshift::detail::fold_impl(std::index_sequence<Is...>{ }, all_fold{ }, std::forward<TupleT>(tuple), true, std::forward<F>(func));
}
template <std::size_t I0, std::size_t... Is, typename TupleT, typename F>
    constexpr auto
    fold_impl(std::index_sequence<I0, Is...>, any_fold, TupleT&& tuple, bool, F&& func)
{
    return func(std::get<I0>(std::forward<TupleT>(tuple)))
        || makeshift::detail::fold_impl(std::index_sequence<Is...>{ }, any_fold{ }, std::forward<TupleT>(tuple), false, std::forward<F>(func));
}
template <typename FoldT, typename TupleT, typename T, typename F>
    constexpr auto
    fold_impl(FoldT, TupleT&& tuple, T&& initialValue, F&& func)
{
    return makeshift::detail::fold_impl(std::make_index_sequence<std::tuple_size<std::decay_t<TupleT>>::value>{ }, FoldT{ },
        std::forward<TupleT>(tuple), std::forward<T>(initialValue), std::forward<F>(func));
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_TUPLE2_HPP_
