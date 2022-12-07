
#ifndef INCLUDED_MAKESHIFT_DETAIL_TUPLE_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_TUPLE_HPP_


#include <tuple>        // for make_tuple()
#include <cstddef>      // for size_t
#include <utility>      // for forward<>(), integer_sequence<>, tuple_size<>, get<>()
#include <functional>   // for invoke()
#include <type_traits>  // for decay<>

#include <gsl-lite/gsl-lite.hpp>  // for gsl_constexpr20

#include <makeshift/metadata.hpp>  // for metadata_v<>, bases(), members()

#include <makeshift/detail/indices-2d.hpp>       // for indices_2d_
#include <makeshift/detail/macros.hpp>           // for MAKESHIFT_DETAIL_FORCEINLINE, MAKESHIFT_DETAIL_EMPTY_BASES
#include <makeshift/detail/tuple-transform.hpp>
#include <makeshift/detail/constval.hpp>


namespace makeshift {


    // Defined in utility.hpp.
template <typename... Ts>
struct type_sequence;


namespace detail {


template <typename TupleT, typename Is>
struct tuple_elements_0_;
template <typename TupleT, std::size_t... Is>
struct tuple_elements_0_<TupleT, std::index_sequence<Is...>>
{
    using type = type_sequence<std::tuple_element_t<Is, TupleT>...>;
};
template <typename TupleT, std::size_t N>
struct tuple_elements_ : tuple_elements_0_<TupleT, std::make_index_sequence<N>>
{
};


template <std::size_t I> struct tuple_index_tag { };
template <std::size_t I, typename T>
struct MAKESHIFT_DETAIL_EMPTY_BASES value_tuple_leaf
{
    T value_;

    friend constexpr T&
    _tuple_get(value_tuple_leaf<I, T>& self, tuple_index_tag<I>) noexcept
    {
        return self.value_;
    }
    friend constexpr T const&
    _tuple_get(value_tuple_leaf<I, T> const& self, tuple_index_tag<I>) noexcept
    {
        return self.value_;
    }
};
template <typename Is, typename... Ts>
struct MAKESHIFT_DETAIL_EMPTY_BASES value_tuple_base;
template <std::size_t... Is, typename... Ts>
struct MAKESHIFT_DETAIL_EMPTY_BASES value_tuple_base<std::index_sequence<Is...>, Ts...> : value_tuple_leaf<Is, Ts>...
{
    constexpr value_tuple_base(Ts... values)
        : value_tuple_leaf<Is, Ts>{ values }...
    {
    }
};
template <std::size_t... Is, typename... Ts>
constexpr auto
operator ==(value_tuple_base<std::index_sequence<Is...>, Ts...> const& lhs, value_tuple_base<std::index_sequence<Is...>, Ts...> const& rhs)
-> decltype(((_tuple_get(lhs, tuple_index_tag<Is>{ }) == _tuple_get(rhs, tuple_index_tag<Is>{ })) && ...))
{
    return ((_tuple_get(lhs, tuple_index_tag<Is>{ }) == _tuple_get(rhs, tuple_index_tag<Is>{ })) && ...);
}
template <std::size_t... Is, typename... Ts>
constexpr auto
operator !=(value_tuple_base<std::index_sequence<Is...>, Ts...> const& lhs, value_tuple_base<std::index_sequence<Is...>, Ts...> const& rhs)
-> decltype(((_tuple_get(lhs, tuple_index_tag<Is>{ }) != _tuple_get(rhs, tuple_index_tag<Is>{ })) && ...))
{
    return ((_tuple_get(lhs, tuple_index_tag<Is>{ }) != _tuple_get(rhs, tuple_index_tag<Is>{ })) && ...);
}
template <std::size_t... Is, typename... Ts>
constexpr auto
operator <(value_tuple_base<std::index_sequence<Is...>, Ts...> const& lhs, value_tuple_base<std::index_sequence<Is...>, Ts...> const& rhs)
-> decltype(((_tuple_get(lhs, tuple_index_tag<Is>{ }) < _tuple_get(rhs, tuple_index_tag<Is>{ })) && ...))
{
    bool result = false;
    auto apply = [&result](auto const& a, auto const& b)
    {
        if (a < b)
        {
            result = true;
            return true;
        }
        else if (b < a)
        {
            result = false;
            return true;
        }
        return false;
    };
    (apply(_tuple_get(lhs, tuple_index_tag<Is>{ }), _tuple_get(rhs, tuple_index_tag<Is>{ })) || ...);
    return result;
}
template <std::size_t... Is, typename... Ts>
constexpr auto
operator >(value_tuple_base<std::index_sequence<Is...>, Ts...> const& lhs, value_tuple_base<std::index_sequence<Is...>, Ts...> const& rhs)
-> decltype(((_tuple_get(lhs, tuple_index_tag<Is>{ }) < _tuple_get(rhs, tuple_index_tag<Is>{ })) && ...))
{
    return rhs < lhs;
}
template <std::size_t... Is, typename... Ts>
constexpr auto
operator <=(value_tuple_base<std::index_sequence<Is...>, Ts...> const& lhs, value_tuple_base<std::index_sequence<Is...>, Ts...> const& rhs)
-> decltype(((_tuple_get(lhs, tuple_index_tag<Is>{ }) < _tuple_get(rhs, tuple_index_tag<Is>{ })) && ...))
{
    return !(rhs < lhs);
}
template <std::size_t... Is, typename... Ts>
constexpr auto
operator >=(value_tuple_base<std::index_sequence<Is...>, Ts...> const& lhs, value_tuple_base<std::index_sequence<Is...>, Ts...> const& rhs)
-> decltype(((_tuple_get(lhs, tuple_index_tag<Is>{ }) < _tuple_get(rhs, tuple_index_tag<Is>{ })) && ...))
{
    return !(lhs < rhs);
}


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


template <typename FuncT, typename TupleT, std::size_t... Is>
gsl_constexpr20 decltype(auto)
apply_impl_1(FuncT&& f, TupleT&& t, std::index_sequence<Is...>)
{
    using std::get;
    return std::invoke(std::forward<FuncT>(f), get<Is>(std::forward<TupleT>(t))...);
}
template <typename FuncT, typename TupleT, std::size_t... Is>
gsl_constexpr20 decltype(auto)
apply_impl(FuncT&& f, TupleT&& t)
{
    return detail::apply_impl_1(std::forward<FuncT>(f), std::forward<TupleT>(t),
        std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<TupleT>>>{ });
}


template <template <typename...> class TupleT, typename IndicesT, std::size_t... Is, typename... Ts>
constexpr TupleT<std::tuple_element_t<IndicesT::col(Is), std::remove_cv_t<std::remove_reference_t<std::tuple_element_t<IndicesT::row(Is), std::tuple<Ts...>>>>>...>
tuple_cat_impl_1(std::index_sequence<Is...>, std::tuple<Ts...> tupleOfTuples)
{
    using std::get;
    return { get<IndicesT::col(Is)>(get<IndicesT::row(Is)>(std::move(tupleOfTuples)))... };
}
template <template <typename...> class TupleT, typename... Ts>
constexpr auto
tuple_cat_impl(Ts&&... tuples)
{
    using Indices = detail::indices_2d_<std::tuple_size<std::decay_t<Ts>>::value...>;
    return detail::tuple_cat_impl_1<TupleT, Indices>(std::make_index_sequence<Indices::size>{ }, std::tuple<Ts&&...>{ std::forward<Ts>(tuples)... });
}


template <typename T, template <typename...> class TupleT, typename ReflectorT>
constexpr auto
all_members(void)
{
    if constexpr (metadata::is_available(metadata::members<T, ReflectorT>()))
    {
        auto baseMemberTupleTuple = detail::apply_impl(
            [](auto&&... bases)
            {
                return TupleT<decltype(detail::all_members<typename std::remove_cv_t<std::remove_reference_t<decltype(bases)>>::type, TupleT, ReflectorT>())...>{
                    detail::all_members<typename std::remove_cv_t<std::remove_reference_t<decltype(bases)>>::type, TupleT, ReflectorT>()...
                };
            },
            metadata::bases<T, ReflectorT>());
        return detail::apply_impl(
            [](auto&&... baseMemberTuples)
            {
                return detail::tuple_cat_impl<TupleT>(
                    std::forward<decltype(baseMemberTuples)>(baseMemberTuples)...,  // base class members
                    metadata::members<T, ReflectorT>());  // direct members
            },
            baseMemberTupleTuple);
    }
    else return std::nullopt;
}

template <template <typename...> class TupleT, typename T>
struct tie_members_functor
{
    T& x;

    template <typename... MembersC>
    constexpr auto
    operator ()(MembersC...)
    {
        return TupleT<decltype(x.*MembersC{ }())...>{ x.*MembersC{ }()... };
    }
};


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_TUPLE_HPP_
