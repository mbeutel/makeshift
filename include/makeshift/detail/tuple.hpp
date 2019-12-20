
#ifndef INCLUDED_MAKESHIFT_DETAIL_TUPLE_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_TUPLE_HPP_


#include <tuple>       // for make_tuple()
#include <cstddef>     // for size_t
#include <utility>     // for forward<>(), integer_sequence<>, tuple_size<>, get<>()
#include <type_traits> // for decay<>

#include <makeshift/detail/macros.hpp>          // for MAKESHIFT_DETAIL_FORCEINLINE
#include <makeshift/detail/tuple-transform.hpp>


namespace makeshift
{

namespace detail
{


template <template <typename...> class TupleT, typename F, typename... Ts>
constexpr MAKESHIFT_DETAIL_FORCEINLINE TupleT<>
tuple_transform_impl(std::index_sequence<>, F&&, Ts&&...)
{
    return { }; // extra overload to avoid unused-parameter warning
}
template <template <typename...> class TupleT, std::size_t... Is, typename F, typename... Ts>
constexpr MAKESHIFT_DETAIL_FORCEINLINE auto
tuple_transform_impl(std::index_sequence<Is...>, F&& func, Ts&&... args)
{
    (void) func;
    using RTuple = TupleT<std::decay_t<decltype(makeshift::detail::transform_element<Is>(func, std::forward<Ts>(args)...))>...>;
    return RTuple(makeshift::detail::transform_element<Is>(func, std::forward<Ts>(args)...)...);
}


template <bool LReduce, std::size_t N, typename ReduceFuncT, typename TransformFuncT>
struct transform_reduce_fn
{
    ReduceFuncT reduceFunc_;
    TransformFuncT transformFunc_;

    template <std::size_t I, typename V, typename... Ts>
    constexpr MAKESHIFT_DETAIL_FORCEINLINE auto operator ()(std::integral_constant<std::size_t, I>, V&& value, Ts&&... args) const
    {
        constexpr std::size_t elementIdx = LReduce ? I : N - 1 - I;
        return (*this)(std::integral_constant<std::size_t, I + 1>{ },
            reduceFunc_(std::forward<V>(value), detail::transform_element<elementIdx>(transformFunc_, std::forward<Ts>(args)...)),
            std::forward<Ts>(args)...);
    }
    template <typename V, typename... Ts>
    constexpr MAKESHIFT_DETAIL_FORCEINLINE auto operator ()(std::integral_constant<std::size_t, N>, V&& value, Ts&&...) const
    {
        return std::forward<V>(value);
    }
};
template <std::size_t N, typename PredFuncT, typename TransformFuncT>
struct conjunction_fn
{
    TransformFuncT transformFunc_;

    template <std::size_t I, typename... Ts>
    constexpr MAKESHIFT_DETAIL_FORCEINLINE bool operator ()(std::integral_constant<std::size_t, I>, Ts&&... args) const
    {
        if (!PredFuncT{ }(detail::transform_element<I>(transformFunc_, std::forward<Ts>(args)...))) return false;
        return (*this)(std::integral_constant<std::size_t, I + 1>{ }, std::forward<Ts>(args)...);
    }
    template <typename... Ts>
    constexpr MAKESHIFT_DETAIL_FORCEINLINE bool operator ()(std::integral_constant<std::size_t, N>, Ts&&...) const
    {
        return true;
    }
};


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_TUPLE_HPP_
