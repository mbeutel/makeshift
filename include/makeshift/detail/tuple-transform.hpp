
#ifndef INCLUDED_MAKESHIFT_DETAIL_TUPLE_TRANSFORM_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_TUPLE_TRANSFORM_HPP_


#include <gsl-lite/gsl-lite.hpp> // for conjunction<>, disjunction<>, gsl_CPP17_OR_GREATER

#if !gsl_CPP17_OR_GREATER
# include <tuple>      // for tuple_size<>
#endif // !gsl_CPP17_OR_GREATER

#include <cstddef>     // for size_t, ptrdiff_t
#include <utility>     // for forward<>(), integer_sequence<>, tuple_size<>, get<>()
#include <type_traits> // for decay<>, integral_constant<>

#include <makeshift/type_traits.hpp>        // for can_instantiate<>, is_tuple_like<>

#include <makeshift/detail/macros.hpp>      // for MAKESHIFT_DETAIL_FORCEINLINE
#include <makeshift/detail/zip.hpp>         // for range_index_t, tuple_index_t


namespace makeshift {

namespace gsl = ::gsl_lite;

namespace detail {


template <std::size_t I, typename R>
struct MAKESHIFT_DETAIL_EMPTY_BASES tuple_zip_iterator_leaf
    : zip_iterator_leaf_base<I, R, tuple_iterator_leaf_mode_<std::decay_t<R>>::value>
{
    using leaf_base = zip_iterator_leaf_base<I, R, tuple_iterator_leaf_mode_<std::decay_t<R>>::value>;
    using leaf_base::leaf_base;
};
template <std::size_t I, typename R>
constexpr tuple_zip_iterator_leaf<I, R>& get_leaf(tuple_zip_iterator_leaf<I, R>& self)
{
    return self;
}


template <typename T> struct is_tuple_arg_0 : is_tuple_like<T> { };
template <> struct is_tuple_arg_0<range_index_t> : std::true_type { };
template <> struct is_tuple_arg_0<tuple_index_t> : std::true_type { };
template <typename T> using is_tuple_arg = is_tuple_arg_0<std::decay_t<T>>;
template <typename T> constexpr bool is_tuple_arg_v = is_tuple_arg<T>::value;

template <typename... Ts> struct are_tuple_args : gsl::conjunction<is_tuple_arg<Ts>...> { };
template <typename... Ts> constexpr bool are_tuple_args_v = are_tuple_args<Ts...>::value;

template <typename T> struct maybe_tuple_size_ : std::integral_constant<std::ptrdiff_t, std::tuple_size<T>::value> { };
template <> struct maybe_tuple_size_<range_index_t> : std::integral_constant<std::ptrdiff_t, -1> { };
template <> struct maybe_tuple_size_<tuple_index_t> : std::integral_constant<std::ptrdiff_t, -1> { };

template <bool Mismatch, std::ptrdiff_t N, typename... Ts> struct equal_sizes_0_;
template <std::ptrdiff_t N, typename... Ts> struct equal_sizes_0_<true, N, Ts...> : std::false_type { static constexpr std::ptrdiff_t size = -1; };
template <std::ptrdiff_t N> struct equal_sizes_0_<false, N> : std::true_type { static constexpr std::ptrdiff_t size = N; };
template <std::ptrdiff_t N, typename T0, typename... Ts>
struct equal_sizes_0_<false, N, T0, Ts...>
    : equal_sizes_0_<
        N != -1 && maybe_tuple_size_<T0>::value != -1 && N != maybe_tuple_size_<T0>::value,
        N != -1 ? N : maybe_tuple_size_<T0>::value,
        Ts...>
{
};
template <typename... Ts> struct equal_sizes_ : equal_sizes_0_<false, -1, Ts...> { };


template <std::size_t I>
constexpr MAKESHIFT_DETAIL_FORCEINLINE std::integral_constant<std::ptrdiff_t, I> get(tuple_index_t) noexcept
{
    return { };
}


template <std::size_t I, typename T>
constexpr MAKESHIFT_DETAIL_FORCEINLINE decltype(auto) get_element(T&& arg) noexcept
{
    using std::get; // make std::get<>() visible to enable ADL for template methods named get<>()
    return get<I>(std::forward<T>(arg));
}


template <std::size_t I, typename... Ts, typename F>
constexpr MAKESHIFT_DETAIL_FORCEINLINE auto
transform_element(F&& func, Ts&&... args)
{
    return func(detail::get_element<I>(std::forward<Ts>(args))...);
}

template <typename F, typename... Ts>
constexpr MAKESHIFT_DETAIL_FORCEINLINE void
template_for_impl(std::index_sequence<>, F&&, Ts&&...)
{
    // extra overload to avoid unused-parameter warning
}
template <std::size_t... Is, typename F, typename... Ts>
constexpr MAKESHIFT_DETAIL_FORCEINLINE void
template_for_impl(std::index_sequence<Is...>, F&& func, Ts&&... args)
{
    (void) func;
    using Swallow = int[];
    (void) Swallow{ 1,
        (detail::transform_element<Is>(func, std::forward<Ts>(args)...), void(), int{ })...
    };
}

template <std::ptrdiff_t N, typename... Ts>
constexpr std::size_t
tuple_transform_size(void)
{
    using Eq = equal_sizes_<std::decay_t<Ts>...>;
    static_assert(Eq::value, "sizes of tuple arguments do not match");
    static_assert(N != -1 || Eq::size != -1 || N == Eq::size, "given size argument does not match sizes of tuple arguments");
    static_assert(N != -1 || Eq::size != -1, "no tuple argument and no size given");
    return std::size_t(N != -1 ? N : Eq::size);
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_TUPLE_TRANSFORM_HPP_
