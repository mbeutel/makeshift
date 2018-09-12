
#ifndef INCLUDED_MAKESHIFT_EXPAND_HPP_
#define INCLUDED_MAKESHIFT_EXPAND_HPP_


#include <string>
#include <sstream>
#include <variant>
#include <type_traits> // for enable_if<>, decay<>

#include <gsl/gsl_assert> // for Expects()

#include <makeshift/type_traits.hpp> // for type_sequence<>, tag<>, is_same_template<>
#include <makeshift/tuple.hpp>       // for type_tuple<>, is_tuple_like<>, tuple_foreach()
#include <makeshift/metadata.hpp>    // for have_metadata<>, metadata_of<>


namespace makeshift
{

inline namespace types
{


template <typename T>
    struct unknown_value
{
    using value_type = T;

    T value;
};
template <typename T>
    unknown_value(T&&) -> unknown_value<std::decay_t<T>>;


} // inline namespace types


namespace detail
{


template <bool Raise, typename R, std::size_t I, typename T, typename TupleT>
    constexpr R expand_impl(T&& value, TupleT&& tuple);
template <typename R, typename T>
    constexpr R expand_impl_fail(std::true_type /* raise */, T&&)
{
    Expects(!"value not supported"); // we end up here if the value was not found, which is a programming error
}
template <typename R, typename T>
    constexpr R expand_impl_fail(std::false_type /* raise */, T&& value)
{
    return unknown_value{ std::forward<T>(value) };
}
template <bool Raise, typename R, std::size_t I, typename T, typename TupleT>
    constexpr R expand_impl_recur(T&& value, TupleT&& tuple)
{
    if (value == get<I>(tuple))
        return { get<I>(std::forward<TupleT>(tuple)) };
    else
        return expand_impl<Raise, R, I + 1>(std::forward<T>(value), std::forward<TupleT>(tuple));
}
template <bool Raise, typename R, std::size_t I, typename T, typename TupleT>
    constexpr R expand_impl(T&& value, TupleT&& tuple)
{
    (void) value;
    (void) tuple;
    if constexpr (I == std::tuple_size<std::decay_t<TupleT>>::value)
        return expand_impl_fail<R>(std::integral_constant<bool, Raise>{ }, std::forward<T>(value));
    else
        return expand_impl_recur<Raise, R, I>(std::forward<T>(value), std::forward<TupleT>(tuple));
}


template <bool Raise, template <typename...> class VariantT, typename T, typename TupleT> struct apply_variant_type;
template <template <typename...> class VariantT, typename T, template <typename...> class TupleT, typename... Ts> struct apply_variant_type<true, VariantT, T, TupleT<Ts...>> { using type = VariantT<Ts...>; };
template <template <typename...> class VariantT, typename T, template <typename...> class TupleT, typename... Ts> struct apply_variant_type<false, VariantT, T, TupleT<Ts...>> { using type = VariantT<unknown_value<T>, Ts...>; };
template <bool Raise, template <typename...> class VariantT, typename T, typename TupleT> using apply_variant_type_t = typename apply_variant_type<Raise, VariantT, T, TupleT>::type;

template <typename T> using is_value_metadata = is_same_template<T, value_metadata>;


template <typename T, T... Vs>
    constexpr std::tuple<std::integral_constant<T, Vs>...> sequence_to_tuple(sequence<T, Vs...>) noexcept
{
    return { };
}


} // namespace detail


inline namespace types
{


    //ᅟ
    // For enum types with metadata, returns a tuple of type-encoded possible values.
    //ᅟ
    //ᅟ    enum Color { red, green, blue };
    //ᅟ    constexpr inline auto reflect(tag<Color>, reflection_metadata_tag) noexcept
    //ᅟ    {
    //ᅟ        return type<Color>(value<red>(), value<green>(), value<blue>());
    //ᅟ    }
    //ᅟ    auto colorValues = values_from_metadata<Color>(); // returns std::tuple<constant<red>, constant<green>, constant<blue>>
    //
template <typename T, typename MetadataTagT = reflection_metadata_tag>
    constexpr auto values_from_metadata(tag<T> = { }, MetadataTagT = { })
{
    static_assert(have_metadata_v<T, MetadataTagT>, "no metadata available for given type and tag");
    static_assert(std::is_enum<T>::value, "values from metadata only supported for enum types");

    auto values = metadata_of<T, MetadataTagT>.attributes
        | tuple_filter<makeshift::detail::is_value_metadata>()
        //| tuple_map([](const auto& v) { return c<std::decay_t<decltype(v)>::value_type::value>; });
        | tuple_map([](const auto& v) { return std::integral_constant<T, std::decay_t<decltype(v)>::value_type::value>{ }; }); // workaround for ICE in VC++
    using Values = decltype(values); // std::tuple<constant<Cs>...>
    return apply_t<type_tuple, Values>(std::move(values)); // type_tuple<constant<Cs>...>
}


    //ᅟ
    // For bool and for constrained integer types, returns a tuple of type-encoded possible values.
    //ᅟ
    //ᅟ    auto boolValues = values_from_type<bool>(); // returns std::tuple<constant<false>, constant<true>>
    //
template <typename T>
    constexpr auto values_from_type(tag<T> = { })
{
    static_assert(std::is_same<T, bool>::value || is_constrained_integer_v<T>, "values from type only supported for bool and constrained integer types");

    if constexpr (std::is_same<T, bool>::value)
        return std::tuple<constant<false>, constant<true>>{ };
    else // is_constrained_integer_v<T>
        return makeshift::detail::sequence_to_tuple(T::verifier::get_valid_values(typename T::constraint{ }));
}


    //ᅟ
    // For bool, constrained integer types, and for enum types with metadata, returns a tuple of type-encoded possible values.
    //ᅟ
template <typename T, typename MetadataTagT = reflection_metadata_tag>
    constexpr auto values_from_type_or_metadata(tag<T> = { }, MetadataTagT = { })
{
    if constexpr (std::is_same<T, bool>::value || is_constrained_integer_v<T>)
        return values_from_type<T>();
    else
        return values_from_metadata<T, MetadataTagT>();
}


    //ᅟ
    // Given a runtime value and a tuple of type-encoded possible values, returns a variant of the type-encoded possible values.
    //ᅟ
    //ᅟ    int runtimeBits = ...;
    //ᅟ    auto bits = expand(runtimeBits, std::make_tuple(c<16>, c<32>, c<64>)); // returns std::variant<constant<16>, constant<32>, constant<64>>
    //
template <typename T, typename TupleT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    typename apply<std::variant, std::decay_t<TupleT>>::type expand(T&& value, TupleT&& tuple)
{
    using R = typename apply<std::variant, std::decay_t<TupleT>>::type;
    return makeshift::detail::expand_impl<true, R, 0>(std::forward<T>(value), std::forward<TupleT>(tuple));
}


    //ᅟ
    // Given a runtime value and a tuple of type-encoded possible values, returns a variant of `unknown_value<>` and the type-encoded possible values.
    // The variant holds `unknown_value<>` if the runtime value does not appear in the tuple of possible values.
    //ᅟ
    //ᅟ    int runtimeBits = ...;
    //ᅟ    auto bits = try_expand(runtimeBits, std::make_tuple(c<16>, c<32>, c<64>)); // returns std::variant<unknown_value<int>, constant<16>, constant<32>, constant<64>>
    //
template <typename T, typename TupleT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    typename makeshift::detail::apply_variant_type<false, std::variant, std::decay_t<T>, std::decay_t<TupleT>>::type try_expand(T&& value, TupleT&& tuple)
{
    using R = typename makeshift::detail::apply_variant_type<false, std::variant, std::decay_t<T>, std::decay_t<TupleT>>::type;
    return makeshift::detail::expand_impl<false, R, 0>(std::forward<T>(value), std::forward<TupleT>(tuple));
}


    //ᅟ
    // Given a runtime value, returns a variant of the type-encoded possible values as defined by metadata.
    //ᅟ
    //ᅟ    enum Color { red, green, blue };
    //ᅟ    constexpr inline auto reflect(tag<Color>, serialization_metadata_tag) noexcept { ... }
    //ᅟ
    //ᅟ    Color runtimeColor = ...;
    //ᅟ    auto color = expand(runtimeColor); // returns std::variant<constant<red>, constant<green>, constant<blue>>
    //
template <typename T, typename MetadataTagT = serialization_metadata_tag>
    auto expand(T&& value, tag<MetadataTagT> = { })
{
    /*constexpr*/ auto tuple = values_from_type_or_metadata(tag_v<std::decay_t<T>>, MetadataTagT{ }); // TODO: currently not constexpr due to VC++ ICE
    return expand(std::forward<T>(value), std::move(tuple));
}


    //ᅟ
    // Given a runtime value, returns a variant of `unknown_value<>` and the type-encoded possible values as defined by metadata.
    // The variant holds `unknown_value<>` if the runtime value does not appear in the tuple of possible values.
    //ᅟ
    //ᅟ    enum Color { red, green, blue };
    //ᅟ    constexpr inline auto reflect(tag<Color>, serialization_metadata_tag) noexcept { ... }
    //ᅟ
    //ᅟ    Color runtimeColor = ...;
    //ᅟ    auto color = try_expand(runtimeColor); // returns std::variant<unknown_value<Color>, constant<red>, constant<green>, constant<blue>>
    //
template <typename T, typename MetadataTagT = serialization_metadata_tag>
    auto try_expand(T&& value, tag<MetadataTagT> = { })
{
    /*constexpr*/ auto tuple = values_from_type_or_metadata(tag_v<std::decay_t<T>>, MetadataTagT{ }); // TODO: currently not constexpr due to VC++ ICE
    return try_expand(std::forward<T>(value), std::move(tuple));
}


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_EXPAND_HPP_
