
#ifndef INCLUDED_MAKESHIFT_DETAIL_METADATA_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_METADATA_HPP_


#include <array>
#include <tuple>
#include <cstddef>      // for size_t
#include <utility>      // for index_sequence<>, tuple_size<>, forward<>()
#include <optional>
#include <functional>   // for invoke()
#include <string_view>
#include <type_traits>  // for void_t<>, index_sequence<>, make_index_sequence<>

#include <gsl-lite/gsl-lite.hpp>  // for type_identity<>, gsl_constexpr20

#include <makeshift/utility.hpp>  // for type_sequence<>

#include <makeshift/detail/array.hpp>  // for array_cat_impl()
#include <makeshift/detail/tuple.hpp>  // for tuple_cat_impl()


namespace makeshift {

namespace gsl = gsl_lite;

namespace detail {


struct convertible_from_type_identity
{
    template <typename T>
    constexpr convertible_from_type_identity(gsl::type_identity<T>)
    {
    }
};

constexpr std::nullopt_t
reflect(convertible_from_type_identity)
{
    return std::nullopt;
}
constexpr auto
reflect(gsl::type_identity<bool>)
{
    return std::array{ false, true };  // TODO: extend?
}

struct reflector
{
    template <typename T>
    constexpr auto
    operator ()(gsl::type_identity<T>) const
    {
        return reflect(gsl::type_identity<T>{ });
    }
};


template <template <typename...> class PredT>
struct predicate_adaptor
{
    template <typename, typename T, typename... ArgsT>
    struct type : PredT<T, ArgsT...>
    {
    };
};


template <typename T>
constexpr std::negation<std::is_same<T, std::nullopt_t>>
is_available(T const&)
{
    return { };
}


template <typename, typename M> struct is_string_like : std::is_convertible<M, std::string_view> { };
template <typename M> struct is_string_like_2 : std::is_convertible<M, std::string_view> { };

template <typename T, typename M> struct is_value_array : std::false_type { };
template <typename T, std::size_t N> struct is_value_array<T, std::array<T, N>> : std::true_type { };

template <typename T, typename M> struct is_member_pointer : std::false_type { };
template <typename T, typename DT> struct is_member_pointer<T, DT T::*> : std::true_type { };

template <typename T, typename R> struct is_base : std::false_type { };
template <typename T, typename U> struct is_base<T, gsl::type_identity<U>> : std::is_base_of<U, T> { };

template <typename T, typename M, bool IsNonEmptyTuple> struct is_base_record_0_;
template <typename T, typename M> struct is_base_record_0_<T, M, true> : is_base<T, std::tuple_element_t<0, M>> { };
template <typename T, typename M> struct is_base_record_0_<T, M, false> : std::false_type { };
template <typename T, typename M, typename = void> struct is_base_record : std::false_type { };
template <typename T, typename M> struct is_base_record<T, M, std::void_t<typename std::tuple_size<M>::type>> : is_base_record_0_<T, M, (std::tuple_size_v<M> > 0)> { };

template <typename T, typename M, typename> struct is_base_record_tuple_0_;
template <typename T, typename M, std::size_t... Is> struct is_base_record_tuple_0_<T, M, std::index_sequence<Is...>> : std::conjunction<is_base_record<T, std::tuple_element_t<Is, M>>...> { };
template <typename T, typename M, typename = void> struct is_base_record_tuple : std::false_type { };
template <typename T, typename M> struct is_base_record_tuple<T, M, std::void_t<typename std::tuple_size<M>::type>> : is_base_record_tuple_0_<T, M, std::make_index_sequence<std::tuple_size_v<M>>> { };

template <typename T, typename M, typename> struct is_base_tuple_0_;
template <typename T, typename M, std::size_t... Is> struct is_base_tuple_0_<T, M, std::index_sequence<Is...>> : std::conjunction<is_base<T, std::tuple_element_t<Is, M>>...> { };
template <typename T, typename M, typename = void> struct is_base_tuple : std::false_type { };
template <typename T, typename M> struct is_base_tuple<T, M, std::void_t<typename std::tuple_size<M>::type>> : is_base_tuple_0_<T, M, std::make_index_sequence<std::tuple_size_v<M>>> { };

template <typename T, typename R> struct is_value : std::false_type { };
template <typename T> struct is_value<T, T> : std::true_type { };

template <typename T, typename M, bool IsNonEmptyTuple> struct is_value_record_0_;
template <typename T, typename M> struct is_value_record_0_<T, M, true> : is_value<T, std::tuple_element_t<0, M>> { };
template <typename T, typename M> struct is_value_record_0_<T, M, false> : std::false_type { };
template <typename T, typename M, typename = void> struct is_value_record : std::false_type { };
template <typename T, typename M> struct is_value_record<T, M, std::void_t<typename std::tuple_size<M>::type>> : is_value_record_0_<T, M, (std::tuple_size_v<M> > 0)> { };

template <typename T, typename M, typename Is> struct is_value_record_tuple_0_;
template <typename T, typename M, std::size_t... Is> struct is_value_record_tuple_0_<T, M, std::index_sequence<Is...>> : std::conjunction<is_value_record<T, std::tuple_element_t<Is, M>>...> { };
template <typename T, typename M, typename = void> struct is_value_record_tuple : std::false_type { };
template <typename T, typename R, std::size_t N> struct is_value_record_tuple<T, std::array<R, N>, std::void_t<R>> : is_value_record<T, R> { };
template <typename T, typename M> struct is_value_record_tuple<T, M, std::void_t<typename std::tuple_size<M>::type>> : is_value_record_tuple_0_<T, M, std::make_index_sequence<std::tuple_size_v<M>>> { };

template <typename T, typename R> struct is_member : std::false_type { };
template <typename T, typename DT> struct is_member<T, DT T::*> : std::true_type { };

template <typename T, typename M, bool IsNonEmptyTuple> struct is_member_record_0_;
template <typename T, typename M> struct is_member_record_0_<T, M, true> : is_member<T, std::tuple_element_t<0, M>> { };
template <typename T, typename M> struct is_member_record_0_<T, M, false> : std::false_type { };
template <typename T, typename M, typename = void> struct is_member_record : std::false_type { };
template <typename T, typename M> struct is_member_record<T, M, std::void_t<typename std::tuple_size<M>::type>> : is_member_record_0_<T, M, (std::tuple_size_v<M> > 0)> { };

template <typename T, typename M, typename> struct is_member_record_tuple_0_;
template <typename T, typename M, std::size_t... Is> struct is_member_record_tuple_0_<T, M, std::index_sequence<Is...>> : std::conjunction<is_member_record<T, std::tuple_element_t<Is, M>>...> { };
template <typename T, typename M, typename = void> struct is_member_record_tuple : std::false_type { };
template <typename T, typename M> struct is_member_record_tuple<T, M, std::void_t<typename std::tuple_size<M>::type>> : is_member_record_tuple_0_<T, M, std::make_index_sequence<std::tuple_size_v<M>>> { };

template <typename T, typename M, typename> struct is_member_tuple_0_;
template <typename T, typename M, std::size_t... Is> struct is_member_tuple_0_<T, M, std::index_sequence<Is...>> : std::conjunction<is_member<T, std::tuple_element_t<Is, M>>...> { };
template <typename T, typename M, typename = void> struct is_member_tuple : std::false_type { };
template <typename T, typename M> struct is_member_tuple<T, M, std::void_t<typename std::tuple_size<M>::type>> : is_member_tuple_0_<T, M, std::make_index_sequence<std::tuple_size_v<M>>> { };

template <typename T, typename M, template <typename...> class PredT, std::size_t N, std::size_t I, int Occurrence, typename ArgsT>
struct record_index_0_;
template <typename T, typename M, template <typename...> class PredT, std::size_t N, std::size_t I, int Occurrence, bool Match, typename ArgsT>
struct record_index_1_;
template <typename T, typename M, template <typename...> class PredT, std::size_t N, std::size_t I, typename ArgsT>
struct record_index_1_<T, M, PredT, N, I, 0, true, ArgsT> : std::integral_constant<std::size_t, I> { };
template <typename T, typename M, template <typename...> class PredT, std::size_t N, std::size_t I, int Occurrence, typename ArgsT>
struct record_index_1_<T, M, PredT, N, I, Occurrence, true, ArgsT> : record_index_0_<T, M, PredT, N, I + 1, Occurrence - 1, ArgsT> { };
template <typename T, typename M, template <typename...> class PredT, std::size_t N, std::size_t I, int Occurrence, typename ArgsT>
struct record_index_1_<T, M, PredT, N, I, Occurrence, false, ArgsT> : record_index_0_<T, M, PredT, N, I + 1, Occurrence, ArgsT> { };
template <typename T, typename M, template <typename...> class PredT, std::size_t N, std::size_t I, int Occurrence, typename... ArgsT>
struct record_index_0_<T, M, PredT, N, I, Occurrence, type_sequence<ArgsT...>> : record_index_1_<T, M, PredT, N, I, Occurrence, PredT<T, std::tuple_element_t<I, M>, ArgsT...>::value, type_sequence<ArgsT...>> { };
template <typename T, typename M, template <typename...> class PredT, std::size_t N, int Occurrence, typename... ArgsT>
struct record_index_0_<T, M, PredT, N, N, Occurrence, type_sequence<ArgsT...>> : std::integral_constant<std::size_t, std::size_t(-1)> { };
template <typename T, typename M, template <typename...> class PredT, int Occurrence, typename, typename ArgsT>
struct record_index : std::integral_constant<std::size_t, std::size_t(-1)> { };
template <typename T, typename M, template <typename...> class PredT, int Occurrence, typename ArgsT>
struct record_index<T, M, PredT, Occurrence, std::void_t<typename std::tuple_size<M>::type>, ArgsT> : record_index_0_<T, M, PredT, std::tuple_size_v<M>, 0, Occurrence, ArgsT> { };

template <typename T, typename M, template <typename...> class PredT, int Occurrence, typename Is, typename ArgsT>
struct record_indices_0_;
template <typename T, typename M, template <typename...> class PredT, int Occurrence, std::size_t... Is, typename ArgsT>
struct record_indices_0_<T, M, PredT, Occurrence, std::index_sequence<Is...>, ArgsT> : std::index_sequence<record_index<T, std::tuple_element_t<Is, M>, PredT, Occurrence, void, ArgsT>::value...> { };
template <typename T, typename M, template <typename...> class PredT, int Occurrence, typename ArgsT>
struct record_indices : record_indices_0_<T, M, PredT, Occurrence, std::make_index_sequence<std::tuple_size_v<M>>, ArgsT> { };

template <typename T, typename ReflectorT>
constexpr decltype(auto)
get_metadata(ReflectorT = { })
{
    static_assert(std::is_invocable_v<ReflectorT const&, gsl::type_identity<T>>);
    return ReflectorT{ }(gsl::type_identity<T>{ });
}

template <typename T>
constexpr decltype(auto)
promote(T const& arg)
{
    if constexpr (std::is_same_v<T, char const*>) return std::string_view(arg);
    else return arg;
}

template <typename V, std::size_t I, typename TupleT, std::size_t... Js>
constexpr auto
extract_column_0(TupleT const& arg, std::index_sequence<Js...>)
{
    using std::get;
    return std::array<V, sizeof...(Js)>{ detail::promote(get<I>(get<Js>(arg)))... };
}
template <typename V, std::size_t I, typename TupleT>
constexpr auto
extract_column(TupleT const& arg)
{
    return detail::extract_column_0<V, I>(arg, std::make_index_sequence<std::tuple_size_v<TupleT>>{ });
}

template <typename V, typename TupleT, std::size_t... Is, std::size_t... Js>
constexpr auto
extract_column_0(TupleT const& arg, std::index_sequence<Is...>, std::index_sequence<Js...>)
{
    using std::get;
    return std::array<V, sizeof...(Js)>{ detail::promote(get<Is>(get<Js>(arg)))... };
}
template <typename V, typename TupleT, std::size_t... Is>
constexpr auto
extract_column(TupleT const& arg, std::index_sequence<Is...>)
{
    return detail::extract_column_0<V>(arg, std::index_sequence<Is...>{ }, std::make_index_sequence<std::tuple_size_v<TupleT>>{ });
}

template <std::size_t I, template <typename...> class TupleT, typename... Ts, std::size_t... Js>
constexpr auto
extract_heterogeneous_column_0(TupleT<Ts...> const& tuple, std::index_sequence<Js...>)
{
    using std::get;
    return TupleT<std::tuple_element_t<0, Ts>...>{ detail::promote(get<I>(get<Js>(tuple)))... };
}
template <std::size_t I, typename TupleT>
constexpr auto
extract_heterogeneous_column(TupleT const& tuple)
{
    return detail::extract_heterogeneous_column_0<I>(tuple, std::make_index_sequence<std::tuple_size_v<TupleT>>{ });
}

template <template <typename...> class TupleT, typename... Ts, std::size_t... Is, std::size_t... Js>
constexpr auto
extract_heterogeneous_column_0(TupleT<Ts...> const& tuple, std::index_sequence<Is...>, std::index_sequence<Js...>)
{
    using std::get;
    return TupleT<std::tuple_element_t<0, Ts>...>{ detail::promote(get<Is>(get<Js>(tuple)))... };
}
template <typename TupleT, std::size_t... Is>
constexpr auto
extract_heterogeneous_column(TupleT const& tuple, std::index_sequence<Is...>)
{
    return detail::extract_heterogeneous_column_0(tuple, std::index_sequence<Is...>{ }, std::make_index_sequence<std::tuple_size_v<TupleT>>{ });
}

template <std::size_t I>
struct extract_metadata_
{
    template <typename R>
    static constexpr auto
    invoke(R const& arg)
    {
        using std::get;
        return detail::promote(get<I>(arg));
    }
};
template <>
struct extract_metadata_<std::size_t(-1)>
{
    template <typename R>
    static constexpr std::nullopt_t
    invoke(R const&)
    {
        return std::nullopt;
    }
};

template <typename T, template <typename...> class PredT, int Occurrence, typename M, typename... ArgsT>
constexpr decltype(auto)
extract_metadata([[maybe_unused]] M const& md, type_sequence<ArgsT...> = { })
{
    if constexpr (!std::is_same_v<M, std::nullopt_t>)
    {
        if constexpr (Occurrence == 0 && PredT<T, M, ArgsT...>::value)
        {
            return detail::promote(md);
        }
        else
        {
            return extract_metadata_<record_index<T, M, PredT, Occurrence, void, type_sequence<ArgsT...>>::value>::invoke(md);
        }
    }
    else return std::nullopt;
}

template <typename T, typename M>
constexpr decltype(auto)
extract_bases([[maybe_unused]] M const& md)
{
    if constexpr (!std::is_same_v<std::decay_t<decltype(detail::extract_metadata<T, is_base_record_tuple, 0>(md))>, std::nullopt_t>)
    {
        return detail::extract_heterogeneous_column<0>(detail::extract_metadata<T, is_base_record_tuple, 0>(md));
    }
    else if constexpr (!std::is_same_v<std::decay_t<decltype(detail::extract_metadata<T, is_base_tuple, 0>(md))>, std::nullopt_t>)
    {
        return detail::extract_metadata<T, is_base_tuple, 0>(md);
    }
    else if constexpr (!std::is_same_v<std::decay_t<decltype(detail::extract_metadata<T, is_base_record, 0>(md))>, std::nullopt_t>)
    {
        using std::get;
        return std::tuple{ get<0>(detail::extract_metadata<T, is_base_record, 0>(md)) };
    }
    else if constexpr (!std::is_same_v<std::decay_t<decltype(detail::extract_metadata<T, is_base, 0>(md))>, std::nullopt_t>)
    {
        return std::tuple{ detail::extract_metadata<T, is_base, 0>(md) };
    }
    else if constexpr (is_base<T, M>::value)
    {
        return std::tuple{ md };
    }
    else if constexpr (!std::is_same_v<M, std::nullopt_t> && std::is_class_v<T>)
    {
            // The type is a class and metadata has been defined for it, but no base classes are listed; hence we assume that no
            // bases exist.
        return std::tuple{ };
    }
    else return std::nullopt;
}

template <typename T, typename M>
constexpr decltype(auto)
extract_values([[maybe_unused]] M const& md)
{
    if constexpr (!std::is_same_v<std::decay_t<decltype(detail::extract_metadata<T, is_value_record_tuple, 0>(md))>, std::nullopt_t>)
    {
        return detail::extract_column<T, 0>(detail::extract_metadata<T, is_value_record_tuple, 0>(md));
    }
    else if constexpr (!std::is_same_v<std::decay_t<decltype(detail::extract_metadata<T, is_value_array, 0>(md))>, std::nullopt_t>)
    {
        return detail::extract_metadata<T, is_value_array, 0>(md);
    }
    else if constexpr (!std::is_same_v<std::decay_t<decltype(detail::extract_metadata<T, is_value_record, 0>(md))>, std::nullopt_t>)
    {
        using std::get;
        return std::array<T, 1>{ get<0>(detail::extract_metadata<T, is_value_record, 0>(md)) };
    }
    else if constexpr (!std::is_same_v<std::decay_t<decltype(detail::extract_metadata<T, is_value, 0>(md))>, std::nullopt_t>)
    {
        return std::array<T, 1>{ detail::extract_metadata<T, is_value, 0>(md) };
    }
    else if constexpr (is_value<T, M>::value)
    {
        return std::array<T, 1>{ md };
    }
    else if constexpr (!std::is_same_v<M, std::nullopt_t>)  // Note that we don't confine `values()` to enums and bools.
    {
            // Metadata has been defined for the type, but no values are listed; hence we assume that no values exist.
        return std::array<T, 0>{ };
    }
    else return std::nullopt;
}

template <typename T, typename M>
constexpr decltype(auto)
extract_members([[maybe_unused]] M const& md)
{
    if constexpr (!std::is_same_v<std::decay_t<decltype(detail::extract_metadata<T, is_member_record_tuple, 0>(md))>, std::nullopt_t>)
    {
        return detail::extract_heterogeneous_column<0>(detail::extract_metadata<T, is_member_record_tuple, 0>(md));
    }
    else if constexpr (!std::is_same_v<std::decay_t<decltype(detail::extract_metadata<T, is_member_tuple, 0>(md))>, std::nullopt_t>)
    {
        return detail::extract_metadata<T, is_member_tuple, 0>(md);
    }
    else if constexpr (!std::is_same_v<std::decay_t<decltype(detail::extract_metadata<T, is_member_record, 0>(md))>, std::nullopt_t>)
    {
        using std::get;
        return std::tuple{ get<0>(detail::extract_metadata<T, is_member_record, 0>(md)) };
    }
    else if constexpr (!std::is_same_v<std::decay_t<decltype(detail::extract_metadata<T, is_member, 0>(md))>, std::nullopt_t>)
    {
        return std::tuple{ detail::extract_metadata<T, is_member, 0>(md) };
    }
    else if constexpr (is_member<T, M>::value)
    {
        return std::tuple{ md };
    }
    else if constexpr (!std::is_same_v<M, std::nullopt_t> && std::is_class_v<T>)
    {
            // The type is a class and metadata has been defined for it, but no memberes are listed; hence we assume that no
            // members exist.
        return std::tuple{ };
    }
    else return std::nullopt;
}

template <typename T, typename V, template <typename...> class PredT, int Occurrence, typename ArgsT, typename R, std::size_t N>
constexpr decltype(auto)
try_extract_column([[maybe_unused]] std::array<R, N> const& arg)
{
    constexpr std::size_t j = record_index<T, R, PredT, Occurrence, void, ArgsT>::value;
    if constexpr (j != std::size_t(-1))
    {
        return detail::extract_column<V, j>(arg);
    }
    else return std::nullopt;
}

template <std::size_t... Is>
constexpr bool
any_index_invalid(std::index_sequence<Is...>)
{
    return ((Is == std::size_t(-1)) || ...);
}

template <typename T, typename V, template <typename...> class PredT, int Occurrence, typename ArgsT, typename TupleT>
constexpr decltype(auto)
try_extract_column([[maybe_unused]] TupleT const& arg)
{
    constexpr auto js = record_indices<T, TupleT, PredT, Occurrence, ArgsT>{ };
    if constexpr (!detail::any_index_invalid(js))
    {
        return detail::extract_column<V>(arg, js);
    }
    else return std::nullopt;
}

template <typename T, template <typename...> class PredT, int Occurrence, typename ArgsT, typename TupleT>
constexpr decltype(auto)
try_extract_heterogeneous_column([[maybe_unused]] TupleT const& arg)
{
    constexpr auto js = record_indices<T, TupleT, PredT, Occurrence, ArgsT>{ };
    if constexpr (!detail::any_index_invalid(js))
    {
        return detail::extract_heterogeneous_column(arg, js);
    }
    else return std::nullopt;
}

template <typename T, typename V, template <typename...> class PredT, int Occurrence, typename ArgsT, typename M>
constexpr decltype(auto)
extract_value_metadata([[maybe_unused]] M const& md)
{
    if constexpr (!std::is_same_v<std::decay_t<decltype(detail::extract_metadata<T, is_value_record_tuple, 0>(md))>, std::nullopt_t>)
    {
        return detail::try_extract_column<T, V, PredT, Occurrence, ArgsT>(detail::extract_metadata<T, is_value_record_tuple, 0>(md));
    }
    //else if constexpr (!std::is_same_v<std::decay_t<decltype(detail::extract_metadata<T, is_value_record, 0>(md))>, std::nullopt_t>)
    //{
    //    using std::get;
    //    return std::array<V, 1>{ get<Occurrence>(detail::extract_metadata<T, is_value_record, 0>(md)) };
    //}
    else if constexpr (!std::is_same_v<M, std::nullopt_t>)  // Note that we don't confine `values()` to enums and bools.
    {
        if constexpr (std::is_same_v<decltype(detail::extract_values<T>(md)), std::nullopt_t>)
        {
                // Metadata has been defined for the type, but no values are listed; hence we assume that no values exist.
            return std::array<V, 0>{ };
        }
        else
        {
                // Values without the desired metadata have been defined for the type.
            return std::nullopt;
        }
    }
    else return std::nullopt;
}

template <typename T, typename V, template <typename...> class PredT, int Occurrence, typename ArgsT, typename M>
constexpr decltype(auto)
extract_member_metadata([[maybe_unused]] M const& md)
{
    if constexpr (!std::is_same_v<std::decay_t<decltype(detail::extract_metadata<T, is_member_record_tuple, 0>(md))>, std::nullopt_t>)
    {
        return detail::try_extract_column<T, V, PredT, Occurrence, ArgsT>(detail::extract_metadata<T, is_member_record_tuple, 0>(md));
    }
    //else if constexpr (!std::is_same_v<std::decay_t<decltype(detail::extract_metadata<T, is_member_record, 0>(md))>, std::nullopt_t>)
    //{
    //    using std::get;
    //    return std::array<V, 1>{ get<Occurrence>(detail::extract_metadata<T, is_member_record, 0>(md)) };
    //}
    else if constexpr (!std::is_same_v<M, std::nullopt_t> && std::is_class_v<T>)
    {
        if constexpr (std::is_same_v<decltype(detail::extract_members<T>(md)), std::nullopt_t>)
        {
                // The type is a class and metadata has been defined for it, but no memberes are listed; hence we assume that no
                // members exist.
            return std::array<V, 0>{ };
        }
        else
        {
                // Members without the desired metadata have been defined for the type.
            return std::nullopt;
        }
    }
    else return std::nullopt;
}


template <typename T, std::size_t N>
constexpr gsl::index
search_index(T value, std::array<T, N> values)
{
    // TODO: this should be optimized to a direct index lookup for the case of consecutive values

    for (std::size_t i = 0; i != N; ++i)
    {
        if (values[i] == value) return gsl::index(i);
    }
    return -1;
}


template <typename T, template <typename...> class TupleT, typename ReflectorT>
constexpr auto
all_members()
{
    auto directMembers = detail::extract_members<T>(detail::get_metadata<T, ReflectorT>());
    if constexpr (detail::is_available(directMembers))
    {
        auto baseMemberTupleTuple = detail::apply_impl(
            [](auto&&... bases)
            {
                return TupleT<decltype(detail::all_members<typename std::remove_cv_t<std::remove_reference_t<decltype(bases)>>::type, TupleT, ReflectorT>())...>{
                    detail::all_members<typename std::remove_cv_t<std::remove_reference_t<decltype(bases)>>::type, TupleT, ReflectorT>()...
                };
            },
            detail::extract_bases<T>(detail::get_metadata<T, ReflectorT>()));
        return detail::apply_impl(
            [&directMembers](auto&&... baseMemberTuples)
            {
                return detail::tuple_cat_impl<TupleT>(
                    std::forward<decltype(baseMemberTuples)>(baseMemberTuples)...,
                    directMembers);
            },
            baseMemberTupleTuple);
    }
    else return std::nullopt;
}
template <typename T, typename V, template <typename...> class PredT, int Occurrence, typename ArgsT, typename ReflectorT>
constexpr decltype(auto)
extract_all_member_metadata()
{
    constexpr auto directMemberMetadata = detail::extract_member_metadata<T, V, PredT, Occurrence, ArgsT>(
        detail::get_metadata<T, ReflectorT>());
    if constexpr (detail::is_available(directMemberMetadata))
    {
        auto baseMemberMetadataArrayTuple = detail::apply_impl(
            [](auto&&... bases)
            {
                return std::make_tuple(
                    detail::extract_member_metadata<typename std::remove_cv_t<std::remove_reference_t<decltype(bases)>>::type, V, PredT, Occurrence, ArgsT>(
                        detail::get_metadata<typename std::remove_cv_t<std::remove_reference_t<decltype(bases)>>::type, ReflectorT>())...);
            },
            detail::extract_bases<T>(detail::get_metadata<T, ReflectorT>()));
        return detail::apply_impl(
            [&directMemberMetadata](auto&&... baseMemberMetadataArrays)
            {
                return detail::array_cat_impl<std::array, V>(
                    std::forward<decltype(baseMemberMetadataArrays)>(baseMemberMetadataArrays)...,
                    directMemberMetadata);
            },
            baseMemberMetadataArrayTuple);
    }
    else return std::nullopt;
}

template <typename T, typename ReflectorT>
struct value_store
{
    static constexpr inline auto value = detail::extract_values<T>(detail::get_metadata<T, ReflectorT>());
};
template <typename T, typename ReflectorT>
struct value_name_store
{
    static constexpr inline auto value = detail::extract_value_metadata<T, std::string_view,
        detail::predicate_adaptor<std::is_convertible>::template type, 0, type_sequence<std::string_view>>(
            detail::get_metadata<T, ReflectorT>());
};
template <typename T, typename ReflectorT>
struct exclusive_member_store
{
    static constexpr inline auto value = detail::extract_members<T>(detail::get_metadata<T, ReflectorT>());
};
template <typename T, typename ReflectorT>
struct exclusive_member_name_store
{
    static constexpr inline auto value = detail::extract_member_metadata<T, std::string_view,
        detail::predicate_adaptor<std::is_convertible>::template type, 0, type_sequence<std::string_view>>(
            detail::get_metadata<T, ReflectorT>());
};
template <template <typename...> class TupleT, typename T, typename ReflectorT>
struct member_store
{
    static constexpr inline auto value = detail::all_members<T, TupleT, ReflectorT>();
};
template <typename T, typename ReflectorT>
struct member_name_store
{
    static constexpr inline auto value = detail::extract_all_member_metadata<T, std::string_view,
        detail::predicate_adaptor<std::is_convertible>::template type, 0, type_sequence<std::string_view>, ReflectorT>();
};


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_METADATA_HPP_
