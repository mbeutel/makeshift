
#ifndef INCLUDED_MAKESHIFT_EXPAND_HPP_
#define INCLUDED_MAKESHIFT_EXPAND_HPP_


#include <variant>
#include <type_traits> // for enable_if<>, decay<>

#include <makeshift/type_traits.hpp> // for type_sequence<>, tag<>
#include <makeshift/tuple.hpp>       // for type_tuple<>, is_tuple_like<>
#include <makeshift/variant.hpp>     // for type_variant<>
#include <makeshift/metadata.hpp>    // for have_metadata<>, metadata_of<>


namespace makeshift
{

namespace detail
{


template <bool Raise, typename R, std::size_t I, typename T, typename TupleT>
    constexpr R expand_to_impl(T&& value, TupleT&& tuple);
template <typename R>
    constexpr R expand_to_impl_fail(std::true_type /* raise */)
{
    throw std::bad_variant_access(); // cannot happen, we just need to silence the compiler about not being able to formally return a value
}
template <typename R>
    constexpr R expand_to_impl_fail(std::false_type /* raise */)
{
    return std::monostate{ };
}
template <bool Raise, typename R, std::size_t I, typename T, typename TupleT>
    constexpr R expand_to_impl_recur(T&& value, TupleT&& tuple)
{
    if (value == get<I>(tuple))
        return { get<I>(std::forward<TupleT>(tuple)) };
    else
        return expand_to_impl<Raise, R, I + 1>(std::forward<T>(value), std::forward<TupleT>(tuple));
}
template <bool Raise, typename R, std::size_t I, typename T, typename TupleT>
    constexpr R expand_to_impl(T&& value, TupleT&& tuple)
{
    (void) value;
    (void) tuple;
    if constexpr (I == std::tuple_size<std::decay_t<TupleT>>::value)
        return expand_to_impl_fail<R>(std::integral_constant<bool, Raise>{ });
    else
        return expand_to_impl_recur<Raise, R, I>(std::forward<T>(value), std::forward<TupleT>(tuple));
}


template <bool Raise, template <typename...> class VariantT, typename TupleT> struct apply_variant_type;
template <template <typename...> class VariantT, template <typename...> class TupleT, typename... Ts> struct apply_variant_type<true, VariantT, TupleT<Ts...>> { using type = VariantT<Ts...>; };
template <template <typename...> class VariantT, template <typename...> class TupleT, typename... Ts> struct apply_variant_type<false, VariantT, TupleT<Ts...>> { using type = VariantT<std::monostate, Ts...>; };
template <bool Raise, template <typename...> class VariantT, typename TupleT> using apply_variant_type_t = typename apply_variant_type<Raise, VariantT, TupleT>::type;

template <bool Raise, typename TupleT> struct apply_default_variant_type : apply_variant_type<Raise, std::variant, TupleT> { };
template <bool Raise, typename... Ts> struct apply_default_variant_type<Raise, type_sequence<Ts...>> : apply_variant_type<Raise, type_variant, type_sequence<Ts...>> { };
template <bool Raise, typename... Ts> struct apply_default_variant_type<Raise, type_tuple<Ts...>> : apply_variant_type<Raise, type_variant, type_tuple<Ts...>> { };
template <bool Raise, typename TupleT> using apply_default_variant_type_t = typename apply_default_variant_type<Raise, TupleT>::type;

template <typename T> using is_value_metadata = is_same_template<T, value_metadata>;


} // namespace detail


inline namespace types
{


template <typename T, typename MetadataTagT = serialization_metadata_tag>
    constexpr auto values_from_metadata(tag_t<T> = { }, tag_t<MetadataTagT> = { })
{
    static_assert(have_metadata_v<T, MetadataTagT>, "no metadata available for given type and tag");
    static_assert(std::is_enum<T>::value || std::is_same<T, bool>::value, "values from metadata only supported for enum types and bool");

    auto values = metadata_of<T, MetadataTagT>.attributes
        | tuple_filter<makeshift::detail::is_value_metadata>()
        //| tuple_map([](const auto& v) { return c<std::decay_t<decltype(v)>::value_type::value>; });
        | tuple_map([](const auto& v) { return std::integral_constant<T, std::decay_t<decltype(v)>::value_type::value>{ }; }); // workaround for ICE in VC++
    using Values = decltype(values); // std::tuple<constant<Cs>...>
    return apply_t<type_tuple, Values>(std::move(values)); // type_tuple<constant<Cs>...>
}


template <template <typename...> class VariantT,
          typename T, typename TupleT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    makeshift::detail::apply_variant_type_t<true, VariantT, std::decay_t<TupleT>> expand_to(T&& value, TupleT&& tuple)
{
    using R = makeshift::detail::apply_variant_type_t<true, VariantT, std::decay_t<TupleT>>;
    return makeshift::detail::expand_to_impl<true, R, 0>(std::forward<T>(value), std::forward<TupleT>(tuple));
}


template <template <typename...> class VariantT,
          typename T, typename TupleT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    makeshift::detail::apply_variant_type_t<false, VariantT, std::decay_t<TupleT>> try_expand_to(T&& value, TupleT&& tuple)
{
    using R = makeshift::detail::apply_variant_type_t<false, VariantT, std::decay_t<TupleT>>;
    return makeshift::detail::expand_to_impl<false, R, 0>(std::forward<T>(value), std::forward<TupleT>(tuple));
}


template <typename T, typename TupleT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    makeshift::detail::apply_default_variant_type_t<true, std::decay_t<TupleT>> expand(T&& value, TupleT&& tuple)
{
    using R = makeshift::detail::apply_default_variant_type_t<true, std::decay_t<TupleT>>;
    return makeshift::detail::expand_to_impl<true, R, 0>(std::forward<T>(value), std::forward<TupleT>(tuple));
}


template <typename T, typename TupleT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    makeshift::detail::apply_default_variant_type_t<false, std::decay_t<TupleT>> try_expand(T&& value, TupleT&& tuple)
{
    using R = makeshift::detail::apply_default_variant_type_t<false, std::decay_t<TupleT>>;
    return makeshift::detail::expand_to_impl<false, R, 0>(std::forward<T>(value), std::forward<TupleT>(tuple));
}


template <typename T, typename MetadataTagT = serialization_metadata_tag>
    auto expand(T&& value, tag_t<MetadataTagT> = { })
{
    /*constexpr*/ auto tuple = values_from_metadata(tag<std::decay_t<T>>, tag<MetadataTagT>); // TODO: currently not constexpr due to VC++ ICE
    return expand(std::forward<T>(value), std::move(tuple));
}


template <typename T, typename MetadataTagT = serialization_metadata_tag>
    auto try_expand(T&& value, tag_t<MetadataTagT> = { })
{
    /*constexpr*/ auto tuple = values_from_metadata(tag<std::decay_t<T>>, tag<MetadataTagT>); // TODO: currently not constexpr due to VC++ ICE
    return try_expand(std::forward<T>(value), std::move(tuple));
}


template <template <typename...> class VariantT,
          typename T, typename MetadataTagT = serialization_metadata_tag>
    auto expand_to(T&& value, tag_t<MetadataTagT> = { })
{
    /*constexpr*/ auto tuple = values_from_metadata(tag<std::decay_t<T>>, tag<MetadataTagT>); // TODO: currently not constexpr due to VC++ ICE
    return expand_to<VariantT>(std::forward<T>(value), std::move(tuple));
}


template <template <typename...> class VariantT,
          typename T, typename MetadataTagT = serialization_metadata_tag>
    auto try_expand_to(T&& value, tag_t<MetadataTagT> = { })
{
    /*constexpr*/ auto tuple = values_from_metadata(tag<std::decay_t<T>>, tag<MetadataTagT>); // TODO: currently not constexpr due to VC++ ICE
    return try_expand_to<VariantT>(std::forward<T>(value), std::move(tuple));
}


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_EXPAND_HPP_
