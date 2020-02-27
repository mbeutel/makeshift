
#ifndef INCLUDED_MAKESHIFT_TUPLE_HPP_
#define INCLUDED_MAKESHIFT_TUPLE_HPP_


#include <array>
#include <tuple>       // for tuple<>, tuple_cat()
#include <cstddef>     // for size_t, ptrdiff_t
#include <utility>     // for forward<>()
#include <type_traits> // for decay<>

#include <gsl-lite/gsl-lite.hpp> // for gsl_NODISCARD

#include <makeshift/type_traits.hpp> // for can_instantiate<>, static_const<>, is_tuple_like<>

#include <makeshift/detail/tuple.hpp>
#include <makeshift/detail/macros.hpp> // for MAKESHIFT_DETAIL_FORCEINLINE


namespace makeshift {


namespace gsl = ::gsl_lite;


    //
    // Pass `tuple_index` to `array_transform()`, `template_for()`, or `tuple_transform()` to have the tuple element index passed as a functor argument.
    // The argument is of type `integral_constant<index, I>`.
    //ᅟ
    //ᅟ        // print all alternatives of a variant
    //ᅟ    constexpr auto numAlternatives = std::variant_size_v<MyVariant>;
    //ᅟ    template_for<numAlternatives>(
    //ᅟ        [](auto idxC)
    //ᅟ        {
    //ᅟ            using T = std::variant_alternative_t<idxC(), MyVariant>;
    //ᅟ            printTypename<T>();
    //ᅟ        });
    //
static constexpr detail::tuple_index_t const& tuple_index = static_const<detail::tuple_index_t>;


    //
    // Takes a scalar procedure (i.e. a function of non-tuple arguments which returns nothing) and calls the procedure for every element in the given tuples.
    //ᅟ
    //ᅟ    template_for(
    //ᅟ        [](auto name, auto elem) { std::cout << name << ": " << elem << '\n'; },
    //ᅟ        std::tuple{ "a", "b" }, std::tuple{ 1, 2.3f });
    //ᅟ    // prints "a: 1\nb: 2.3\n"
    //
template <typename F, typename... Ts>
constexpr MAKESHIFT_DETAIL_FORCEINLINE void
template_for(F&& func, Ts&&... args)
{
    static_assert(detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = detail::tuple_transform_size<-1, Ts...>();
    detail::template_for_impl(std::make_index_sequence<size>{ }, std::forward<F>(func), std::forward<Ts>(args)...);
}


    //
    // Takes a scalar procedure (i.e. a function of non-tuple arguments which returns nothing) and calls the procedure for every element in the given tuples.
    //ᅟ
    //ᅟ    template_for<3>(
    //ᅟ        [](index i) { std::cout << i << '\n'; },
    //ᅟ        range_index);
    //ᅟ    // prints "0\n1\n2\n"
    //
template <std::size_t N, typename F, typename... Ts>
constexpr MAKESHIFT_DETAIL_FORCEINLINE void
template_for(F&& func, Ts&&... args)
{
    static_assert(detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = detail::tuple_transform_size<N, Ts...>();
    return detail::template_for_impl(std::make_index_sequence<size>{ }, std::forward<F>(func), std::forward<Ts>(args)...);
}


    //
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns a tuple of the results of the function applied to the tuple elements.
    //ᅟ
    //ᅟ    auto squares = tuple_transform(
    //ᅟ        [](auto x) { return x*x; },
    //ᅟ        std::tuple{ 2, 3.0f });
    //ᅟ    // returns std::tuple{ 4, 9.0f }
    //
template <typename F, typename... Ts>
gsl_NODISCARD constexpr MAKESHIFT_DETAIL_FORCEINLINE auto
tuple_transform(F&& func, Ts&&... args)
{
    static_assert(detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = detail::tuple_transform_size<-1, Ts...>();
    return detail::tuple_transform_impl<std::tuple>(std::make_index_sequence<size>{ }, std::forward<F>(func), std::forward<Ts>(args)...);
}

    //
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns a tuple of the results of the function applied to the tuple elements.
    // The tuple is constructed using the given tuple template.
    //ᅟ
    //ᅟ    auto squares = tuple_transform<MyTuple>(
    //ᅟ        [](auto x) { return x*x; },
    //ᅟ        std::tuple{ 2, 3.0f });
    //ᅟ    // returns MyTuple<int, float>{ 4, 9.0f }
    //
template <template<typename...> class TupleT, typename F, typename... Ts>
gsl_NODISCARD constexpr MAKESHIFT_DETAIL_FORCEINLINE auto
tuple_transform(F&& func, Ts&&... args)
{
    static_assert(detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = detail::tuple_transform_size<-1, Ts...>();
    return detail::tuple_transform_impl<TupleT>(std::make_index_sequence<size>{ }, std::forward<F>(func), std::forward<Ts>(args)...);
}


    //
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns a tuple of the results of the function applied to the tuple elements.
    //ᅟ
    //ᅟ    auto indices = tuple_transform_n<3>(
    //ᅟ        [](index i) { return i; },
    //ᅟ        range_index);
    //ᅟ    // returns std::tuple{ 0, 1, 2 }
    //
template <std::size_t N, typename F, typename... Ts>
gsl_NODISCARD constexpr MAKESHIFT_DETAIL_FORCEINLINE auto
tuple_transform_n(F&& func, Ts&&... args)
{
    static_assert(detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = detail::tuple_transform_size<N, Ts...>();
    return detail::tuple_transform_impl<std::tuple>(std::make_index_sequence<size>{ }, std::forward<F>(func), std::forward<Ts>(args)...);
}

    //
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns a tuple of the results of the function applied to the tuple elements.
    // The tuple is constructed using the given tuple template.
    //ᅟ
    //ᅟ    auto indices = tuple_transform_n<MyTuple, 3>(
    //ᅟ        [](index i) { return i; },
    //ᅟ        range_index);
    //ᅟ    // returns MyTuple<index, index, index>{ 0, 1, 2 }
    //
template <template <typename...> class TupleT, std::size_t N, typename F, typename... Ts>
gsl_NODISCARD constexpr MAKESHIFT_DETAIL_FORCEINLINE auto
tuple_transform_n(F&& func, Ts&&... args)
{
    static_assert(detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = detail::tuple_transform_size<N, Ts...>();
    return detail::tuple_transform_impl<TupleT>(std::make_index_sequence<size>{ }, std::forward<F>(func), std::forward<Ts>(args)...);
}


    //
    // Takes an initial value, a reducer, a transformer, and a list of tuples and reduces them to a scalar value.
    //ᅟ
    //ᅟ    template_transform_reduce(
    //ᅟ        std::size_t(0),
    //ᅟ        std::plus<>{ },
    //ᅟ        [](auto&& str) { return str.length(); },
    //ᅟ        std::tuple{ "Hello, "s, "World!"sv });
    //ᅟ    // returns 13
    //
template <typename InitialValueT, typename ReduceFuncT, typename TransformFuncT, typename... Ts>
gsl_NODISCARD constexpr MAKESHIFT_DETAIL_FORCEINLINE auto
template_transform_reduce(InitialValueT&& initialValue, ReduceFuncT&& reduce, TransformFuncT&& transform, Ts&&... args)
{
    static_assert(detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = detail::tuple_transform_size<-1, Ts...>();
    return detail::transform_reduce_fn<true, size, ReduceFuncT, TransformFuncT>{
        std::forward<ReduceFuncT>(reduce), std::forward<TransformFuncT>(transform)
    }(std::integral_constant<std::size_t, 0>{ }, std::forward<InitialValueT>(initialValue), std::forward<Ts>(args)...);
}


    //
    // Takes an initial value, a reducer, and a tuple and reduces them to a scalar value.
    //ᅟ
    //ᅟ    template_reduce(
    //ᅟ        std::string{ },
    //ᅟ        std::plus<>{ },
    //ᅟ        std::tuple{ "Hello, ", "World!" });
    //ᅟ    // returns "Hello, World!"s;
    //
template <typename InitialValueT, typename ReduceFuncT, typename T>
gsl_NODISCARD constexpr MAKESHIFT_DETAIL_FORCEINLINE auto
template_reduce(InitialValueT&& initialValue, ReduceFuncT&& reduce, T&& arg)
{
    static_assert(detail::are_tuple_args_v<T>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = detail::tuple_transform_size<-1, T>();
    return detail::transform_reduce_fn<true, size, ReduceFuncT, gsl::identity>{
        std::forward<ReduceFuncT>(reduce), { },
    }(std::integral_constant<std::size_t, 0>{ }, std::forward<InitialValueT>(initialValue), std::forward<T>(arg));
}


    //
    // Takes a predicate and a list of tuples and returns whether the predicate is satisfied for all sets of tuple elements.
    //ᅟ
    //ᅟ    template_all_of(
    //ᅟ        [](auto&& str) { return str.empty(); },
    //ᅟ        std::tuple{ "Hello, "s, "World!"sv });
    //ᅟ    // returns false
    //
template <typename PredicateT, typename... Ts>
gsl_NODISCARD constexpr MAKESHIFT_DETAIL_FORCEINLINE bool
template_all_of(PredicateT&& predicate, Ts&&... args)
{
    static_assert(detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = detail::tuple_transform_size<-1, Ts...>();
    return detail::conjunction_fn<size, gsl::identity, PredicateT>{
        std::forward<PredicateT>(predicate)
    }(std::integral_constant<std::size_t, 0>{ }, std::forward<Ts>(args)...);
}


    //
    // Takes a predicate and a list of tuples and returns whether the predicate is satisfied for any set of tuple elements.
    //ᅟ
    //ᅟ    template_any_of(
    //ᅟ        [](auto&& str) { return str.empty(); },
    //ᅟ        std::tuple{ "Hello, "s, "World!"sv });
    //ᅟ    // returns false
    //
template <typename PredicateT, typename... Ts>
gsl_NODISCARD constexpr MAKESHIFT_DETAIL_FORCEINLINE bool
template_any_of(PredicateT&& predicate, Ts&&... args)
{
    static_assert(detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = detail::tuple_transform_size<-1, Ts...>();
    return !detail::conjunction_fn<size, detail::negation_fn, PredicateT>{
        std::forward<PredicateT>(predicate)
    }(std::integral_constant<std::size_t, 0>{ }, std::forward<Ts>(args)...);
}


    //
    // Takes a predicate and a list of tuples and returns whether the predicate is satisfied for no set of tuple elements.
    //ᅟ
    //ᅟ    template_none_of(
    //ᅟ        [](auto&& str) { return str.empty(); },
    //ᅟ        std::tuple{ "Hello, "s, "World!"sv });
    //ᅟ    // returns true
    //
template <typename PredicateT, typename... Ts>
gsl_NODISCARD constexpr MAKESHIFT_DETAIL_FORCEINLINE bool
template_none_of(PredicateT&& predicate, Ts&&... args)
{
    static_assert(detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = detail::tuple_transform_size<-1, Ts...>();
    return detail::conjunction_fn<size, detail::negation_fn, PredicateT>{
        std::forward<PredicateT>(predicate)
    }(std::integral_constant<std::size_t, 0>{ }, std::forward<Ts>(args)...);
}


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_TUPLE_HPP_
