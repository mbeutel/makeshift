
#ifndef INCLUDED_MAKESHIFT_TUPLE_HPP_
#define INCLUDED_MAKESHIFT_TUPLE_HPP_


#include <array>
#include <tuple>        // for tuple<>, tuple_cat()
#include <cstddef>      // for size_t, ptrdiff_t
#include <utility>      // for forward<>(), index_sequence_for<>
#include <type_traits>  // for decay<>

#include <gsl-lite/gsl-lite.hpp>  // for identity, gsl_constexpr20, gsl_CPP17_OR_GREATER

#if !gsl_CPP17_OR_GREATER
# error makeshift requires C++17 mode or higher
#endif // !gsl_CPP17_OR_GREATER

#include <makeshift/type_traits.hpp>  // for can_instantiate<>, static_const<>, is_tuple_like<>, nth_type<>

#include <makeshift/concepts.hpp>  // for tuple_like<>
#include <makeshift/metadata.hpp>  // for all_members()

#include <makeshift/detail/constval.hpp>  // for MAKESHIFT_CONSTVAL_()
#include <makeshift/detail/tuple.hpp>
#include <makeshift/detail/macros.hpp>    // for MAKESHIFT_DETAIL_FORCEINLINE, MAKESHIFT_DETAIL_EMPTY_BASES
#include <makeshift/detail/metadata.hpp>  // for apply_impl()


namespace makeshift {

namespace gsl = ::gsl_lite;


    //
    // Given a tuple-like type, returns a `type_sequence<>` of the element types.
    //
template <typename T>
struct tuple_elements : detail::tuple_elements_<T, std::tuple_size_v<T>> { };

    //
    // Given a tuple-like type, returns a `type_sequence<>` of the element types.
    //
template <typename T>
using tuple_elements_t = typename detail::tuple_elements_<T, std::tuple_size_v<T>>::type;


    //
    // Minimalistic tuple type for increased compile-time throughput.
    //
template <typename... Ts>
struct MAKESHIFT_DETAIL_EMPTY_BASES value_tuple : detail::value_tuple_base<std::index_sequence_for<Ts...>, Ts...>
{
    using _base = detail::value_tuple_base<std::index_sequence_for<Ts...>, Ts...>;
    using _base::_base;
};
template <typename... Ts>
value_tuple(Ts...) -> value_tuple<Ts...>;
template <std::size_t I, typename... Ts>
[[nodiscard]] constexpr auto&
get(value_tuple<Ts...>& tuple) noexcept
{
    static_assert(I < sizeof...(Ts), "tuple index out of range");
    return _tuple_get(tuple, detail::tuple_index_tag<I>{ });
}
template <std::size_t I, typename... Ts>
[[nodiscard]] constexpr auto const&
get(value_tuple<Ts...> const& tuple) noexcept
{
    static_assert(I < sizeof...(Ts), "tuple index out of range");
    return _tuple_get(tuple, detail::tuple_index_tag<I>{ });
}

template <typename... Ts>
[[nodiscard]] constexpr value_tuple<std::decay_t<Ts>...>
make_value_tuple(Ts&&... args)
{
    return { std::forward<Ts>(args)... };
}


    //
    // Takes a scalar procedure (i.e. a function of non-tuple arguments which returns nothing) and calls the procedure for every
    // element in the given tuples.
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
    // Takes a scalar procedure (i.e. a function of non-tuple arguments which returns nothing) and calls the procedure for every
    // element in the given tuples.
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
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns a tuple of the results of the function applied
    // to the tuple elements.
    //ᅟ
    //ᅟ    auto squares = tuple_transform(
    //ᅟ        [](auto x) { return x*x; },
    //ᅟ        std::tuple{ 2, 3.0f });
    //ᅟ    // returns std::tuple{ 4, 9.0f }
    //
template <typename F, typename... Ts>
[[nodiscard]] constexpr MAKESHIFT_DETAIL_FORCEINLINE auto
tuple_transform(F&& func, Ts&&... args)
{
    static_assert(detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = detail::tuple_transform_size<-1, Ts...>();
    return detail::tuple_transform_impl<std::tuple>(std::make_index_sequence<size>{ }, std::forward<F>(func), std::forward<Ts>(args)...);
}

    //
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns a tuple of the results of the function applied
    // to the tuple elements. The tuple is constructed using the given tuple template.
    //ᅟ
    //ᅟ    auto squares = tuple_transform<MyTuple>(
    //ᅟ        [](auto x) { return x*x; },
    //ᅟ        std::tuple{ 2, 3.0f });
    //ᅟ    // returns MyTuple<int, float>{ 4, 9.0f }
    //
template <template<typename...> class TupleT, typename F, typename... Ts>
[[nodiscard]] constexpr MAKESHIFT_DETAIL_FORCEINLINE auto
tuple_transform(F&& func, Ts&&... args)
{
    static_assert(detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = detail::tuple_transform_size<-1, Ts...>();
    return detail::tuple_transform_impl<TupleT>(std::make_index_sequence<size>{ }, std::forward<F>(func), std::forward<Ts>(args)...);
}


    //
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns a tuple of the results of the function applied
    // to the tuple elements.
    //ᅟ
    //ᅟ    auto indices = tuple_transform<3>(
    //ᅟ        [](index i) { return i; },
    //ᅟ        range_index);
    //ᅟ    // returns std::tuple{ 0, 1, 2 }
    //
template <std::size_t N, typename F, typename... Ts>
[[nodiscard]] constexpr MAKESHIFT_DETAIL_FORCEINLINE auto
tuple_transform(F&& func, Ts&&... args)
{
    static_assert(detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = detail::tuple_transform_size<N, Ts...>();
    return detail::tuple_transform_impl<std::tuple>(std::make_index_sequence<size>{ }, std::forward<F>(func), std::forward<Ts>(args)...);
}

    //
    // Takes a scalar function (i.e. a function of non-tuple arguments) and returns a tuple of the results of the function applied
    // to the tuple elements. The tuple is constructed using the given tuple template.
    //ᅟ
    //ᅟ    auto indices = tuple_transform<MyTuple, 3>(
    //ᅟ        [](index i) { return i; },
    //ᅟ        range_index);
    //ᅟ    // returns MyTuple<index, index, index>{ 0, 1, 2 }
    //
template <template <typename...> class TupleT, std::size_t N, typename F, typename... Ts>
[[nodiscard]] constexpr MAKESHIFT_DETAIL_FORCEINLINE auto
tuple_transform(F&& func, Ts&&... args)
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
[[nodiscard]] constexpr MAKESHIFT_DETAIL_FORCEINLINE auto
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
[[nodiscard]] constexpr MAKESHIFT_DETAIL_FORCEINLINE auto
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
[[nodiscard]] constexpr MAKESHIFT_DETAIL_FORCEINLINE bool
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
[[nodiscard]] constexpr MAKESHIFT_DETAIL_FORCEINLINE bool
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
[[nodiscard]] constexpr MAKESHIFT_DETAIL_FORCEINLINE bool
template_none_of(PredicateT&& predicate, Ts&&... args)
{
    static_assert(detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");
    constexpr std::size_t size = detail::tuple_transform_size<-1, Ts...>();
    return detail::conjunction_fn<size, detail::negation_fn, PredicateT>{
        std::forward<PredicateT>(predicate)
    }(std::integral_constant<std::size_t, 0>{ }, std::forward<Ts>(args)...);
}


template <typename FuncT, typename TupleT, std::size_t... Is>
gsl_constexpr20 decltype(auto)
apply(FuncT&& f, TupleT&& t)
{
    return detail::apply_impl(std::forward<FuncT>(f), std::forward<TupleT>(t));
}


    //
    // Takes a list of tuples and returns a tuple of concatenated elements.
    //ᅟ
    //ᅟ    auto numbers = std::tuple{ 2, 3 };
    //ᅟ    auto moreNumbers = std::array{ 6., 8. };
    //ᅟ    auto allNumbers = tuple_cat(numbers, moreNumbers);
    //ᅟ    // returns std::tuple{ 2, 3, 6., 8. }
    //
template <typename... Ts>
[[nodiscard]] constexpr auto
tuple_cat(Ts&&... tuples)
{
    static_assert(detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");

    return detail::tuple_cat_impl<std::tuple>(std::forward<Ts>(tuples)...);
}

    //
    // Takes a list of tuples and returns a tuple of concatenated elements.
    // The tuple is constructed using the given tuple template.
    //ᅟ
    //ᅟ    auto numbers = std::tuple{ 2, 3 };
    //ᅟ    auto moreNumbers = std::array{ 6., 8. };
    //ᅟ    auto allNumbers = tuple_cat<MyTuple>(numbers, moreNumbers);
    //ᅟ    // returns MyTuple{ 2, 3, 6., 8. }
    //
template <template <typename...> class TupleT, typename... Ts>
[[nodiscard]] constexpr auto
tuple_cat(Ts&&... tuples)
{
    static_assert(detail::are_tuple_args_v<Ts...>, "arguments must be tuples or tuple-like types");

    return detail::tuple_cat_impl<TupleT>(std::forward<Ts>(tuples)...);
}


#if gsl_CPP20_OR_GREATER  // need constexpr `std::invoke()`
template <typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr auto
tie_members(T& x, ReflectorT = { })
requires std::invocable<ReflectorT const&, gsl::type_identity<T>>
{
    auto allMembersC = MAKESHIFT_CONSTVAL_(metadata::members<value_tuple, T, ReflectorT>());
    static_assert(metadata::is_available(allMembersC()), "no member metadata was defined for type T");

    return makeshift::apply(
        detail::tie_members_functor<std::tuple, T>{ x },
        allMembersC);
}
template <template <typename...> class TupleT, typename T, typename ReflectorT = reflector>
[[nodiscard]] constexpr auto
tie_members(T& x, ReflectorT = { })
requires std::invocable<ReflectorT const&, gsl::type_identity<T>>
{
    auto allMembersC = MAKESHIFT_CONSTVAL_(metadata::members<value_tuple, T, ReflectorT>());
    static_assert(metadata::is_available(allMembersC()), "no member metadata was defined for type T");

    return makeshift::apply(
        detail::tie_members_functor<TupleT, T>{ x },
        allMembersC);
}
template <typename T, typename MembersC>
[[nodiscard]] constexpr auto
tie_members(T& x, MembersC const& membersC)
requires tuple_like<decltype(membersC())>
{
    return makeshift::apply(
        detail::tie_members_functor<std::tuple, T>{ x },
        membersC);
}
template <template <typename...> class TupleT, typename T, typename MembersC>
[[nodiscard]] constexpr auto
tie_members(T& x, MembersC const& membersC)
requires tuple_like<decltype(membersC())>
{
    return makeshift::apply(
        detail::tie_members_functor<TupleT, T>{ x },
        membersC);
}
#endif // gsl_CPP20_OR_GREATER


} // namespace makeshift


    // Implement tuple-like protocol for `value_tuple<>`.
template <typename... Ts> class std::tuple_size<makeshift::value_tuple<Ts...>> : public std::integral_constant<std::size_t, sizeof...(Ts)> { };
template <std::size_t I, typename... Ts> class std::tuple_element<I, makeshift::value_tuple<Ts...>> : public makeshift::nth_type<I, Ts...> { };


#endif // INCLUDED_MAKESHIFT_TUPLE_HPP_
