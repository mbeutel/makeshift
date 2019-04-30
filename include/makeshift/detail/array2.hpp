
#ifndef INCLUDED_MAKESHIFT_DETAIL_ARRAY2_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_ARRAY2_HPP_


#include <array>
#include <tuple>
#include <cstddef>     // for size_t
#include <utility>     // for integer_sequence<>, get<>()

#include <makeshift/detail/workaround.hpp>       // for cadd<>()
#include <makeshift/detail/tuple2-transform.hpp> // for tuple_transform_impl2()


namespace makeshift
{

namespace detail
{


template <typename T> struct is_std_array_ : std::false_type { };
template <typename T, std::size_t N> struct is_std_array_<std::array<T, N>> : std::true_type { };


template <typename F, std::size_t I, typename... Ts> struct result_type_ { using type = decltype(std::declval<F>()(makeshift::detail::get_element<I>(std::declval<Ts>())...)); };

template <typename F, typename Is, typename... Ts>
    struct result_types_;
template <typename F, std::size_t... Is, typename... Ts>
    struct result_types_<F, std::index_sequence<Is...>, Ts...>
{
    using type = type_sequence<typename result_type_<F, Is, Ts...>::type...>;
};

template <typename R, typename T> struct transfer_ref_ { using type = T; };
template <typename R, typename T> struct transfer_ref_<R&, T> { using type = T&; };
template <typename R, typename T> struct transfer_ref_<const R&, T> { using type = const T&; };
template <typename R, typename T> struct transfer_ref_<R&&, T> { using type = T; };

template <std::ptrdiff_t N, bool HomogeneousArgs, typename F, typename... Ts>
    struct homogeneous_result_;
template <std::ptrdiff_t N, typename F, typename... Ts>
    struct homogeneous_result_<N, true, F, Ts...>
{
        // all arguments are array types, so we can just extract their value types
    using type = decltype(std::declval<F>()(std::declval<typename transfer_ref_<Ts, typename std::decay_t<Ts>::value_type>::type>()...));
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


template <std::size_t... Is, typename F, typename... Ts>
    constexpr auto
    tuple_transform_impl1(transform_to_array_tag, std::index_sequence<Is...>, F&& func, Ts&&... args)
{
    (void) func;
    using R = typename homogeneous_result_<sizeof...(Is), cand(is_std_array_<std::decay_t<Ts>>::value...), F, Ts...>::type;
    return std::array<R, sizeof...(Is)>{ makeshift::detail::tuple_transform_impl2<Is>(func, std::forward<Ts>(args)...)... };
}
template <typename R, std::size_t... Is, typename F, typename... Ts>
    constexpr auto
    tuple_transform_impl1(transform_to_array_of_tag<R>, std::index_sequence<Is...>, F&& func, Ts&&... args)
{
    (void) func;
    return std::array<R, sizeof...(Is)>{ makeshift::detail::tuple_transform_impl2<Is>(func, std::forward<Ts>(args)...)... };
}


    // borrowing the 2-d indexing technique that first appeared in the `tuple_cat()` implementation of Microsoft's STL
template <std::size_t... Ns>
    struct indices_2d_
{
    static constexpr std::size_t size = cadd<std::size_t>(Ns...);
    static constexpr std::size_t row(std::size_t i) noexcept
    {
        std::size_t sizes[] = { Ns... };
        std::size_t r = 0;
        while (i >= sizes[r]) // compiler error if sizeof...(Ns) == 0 or i >= (Ns + ... + 0)
        {
            ++r;
            i -= sizes[r];
        }
        return r;
    }
    static constexpr std::size_t col(std::size_t i) noexcept
    {
        std::size_t sizes[] = { Ns... };
        std::size_t r = 0;
        while (i >= sizes[r]) // compiler error if sizeof...(Ns) == 0 or i >= (Ns + ... + 0)
        {
            ++r;
            i -= sizes[r];
        }
        return i;
    }
};

template <typename T, std::size_t... Is, typename IndicesT, typename... Ts>
    constexpr std::array<T, IndicesT::size> array_cat_impl(std::index_sequence<Is...>, IndicesT, std::tuple<Ts...> tupleOfTuples)
{
    using std::get;
    return { get<IndicesT::col(Is)>(std::get<IndicesT::row(Is)>(tupleOfTuples))... };
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_ARRAY2_HPP_
