
#ifndef INCLUDED_MAKESHIFT_EXPAND_HPP_
#define INCLUDED_MAKESHIFT_EXPAND_HPP_


#include <string>
#include <sstream>
#include <variant>
#include <stdexcept>
#include <type_traits> // for enable_if<>, decay<>

#include <makeshift/type_traits.hpp> // for type_sequence<>, tag<>, is_same_template<>
#include <makeshift/tuple.hpp>       // for type_tuple<>, is_tuple_like<>, tuple_foreach()
#include <makeshift/variant.hpp>     // for type_variant<>
#include <makeshift/metadata.hpp>    // for have_metadata<>, metadata_of<>
#include <makeshift/streamable.hpp>  // for stream_serializer


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


template <typename VariantT> struct variant_without_unknown_value;
template <template <typename...> class VariantT, typename T, typename... Ts> struct variant_without_unknown_value<VariantT<unknown_value<T>, Ts...>> { using type = VariantT<Ts...>; };
template <typename VariantT> using variant_without_unknown_value_t = typename variant_without_unknown_value<VariantT>::type;

struct identity_transform
{
    template <typename T>
        auto operator ()(T&& value)
    {
        return std::forward<T>(value);
    }
};

template <typename SerializerT>
    struct variant_fail_if_unknown_t : variant_stream_base<variant_fail_if_unknown_t<SerializerT>>
{
private:
    std::string errorMessage_;
    SerializerT serializer_;

public:
    variant_fail_if_unknown_t(std::string _errorMessage, SerializerT _serializer) : errorMessage_(std::move(_errorMessage)), serializer_(std::forward<SerializerT>(_serializer)) { }

private:
    template <typename VariantT>
        void fail(const VariantT& v) const
    {
        using T = typename std::variant_alternative_t<0, VariantT>::value_type;
        using ResultVariant = variant_without_unknown_value_t<std::decay_t<VariantT>>;
        using ResultTuple = apply_t<type_tuple, ResultVariant>;
        using std::get; // make std::get<>(std::pair<>&&) visible to enable ADL for template methods named get<>()

        std::ostringstream sstr;
        if (!errorMessage_.empty())
            sstr << errorMessage_;
        else
            sstr << "unsupported value";
        sstr << " (given value: " << streamable(get<0>(v).value, serializer_) << "; values supported: ";
        bool first = true;
        tuple_foreach(ResultTuple{ }, [&](auto element)
        {
            if (first)
                first = false;
            else
                sstr << ", ";
            T runtimeTypedElement = element;
            sstr << streamable(runtimeTypedElement, serializer_);
        });
        sstr << ")";
        throw std::runtime_error(sstr.str());
    }

public:
    template <typename VariantT,
              typename = std::enable_if_t<is_variant_like_v<std::decay_t<VariantT>> && is_same_template<std::variant_alternative_t<0, std::decay_t<VariantT>>, unknown_value>::value>>
        auto operator ()(VariantT&& variant) const
    {
        using ResultVariant = variant_without_unknown_value_t<std::decay_t<VariantT>>;
        if (variant.index() == 0) // unknown_value<>
            fail(variant);
        return variant_apply<ResultVariant, 1>(identity_transform{ }, std::forward<VariantT>(variant));
    }
};


template <bool Raise, typename R, std::size_t I, typename T, typename TupleT>
    constexpr R expand_impl(T&& value, TupleT&& tuple);
template <typename R, typename T>
    constexpr R expand_impl_fail(std::true_type /* raise */, T&&)
{
    throw std::runtime_error("invalid value"); // TODO: surely we can do better than that!
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
    //ᅟ    constexpr inline auto reflect(Color*, reflection_metadata_tag) noexcept
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
    // Raises std::runtime_error if the runtime value does not appear in the tuple of possible values.
    //ᅟ
    //ᅟ    int runtimeBits = ...;
    //ᅟ    auto bits = expand(runtimeBits, std::make_tuple(c<16>, c<32>, c<64>)); // returns std::variant<constant<16>, constant<32>, constant<64>>
    //
template <typename T, typename TupleT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    makeshift::detail::apply_variant_type_t<true, std::variant, std::decay_t<T>, std::decay_t<TupleT>> expand(T&& value, TupleT&& tuple)
{
    using R = makeshift::detail::apply_variant_type_t<true, std::variant, std::decay_t<T>, std::decay_t<TupleT>>;
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
    makeshift::detail::apply_variant_type_t<false, std::variant, std::decay_t<T>, std::decay_t<TupleT>> try_expand(T&& value, TupleT&& tuple)
{
    using R = makeshift::detail::apply_variant_type_t<false, std::variant, std::decay_t<T>, std::decay_t<TupleT>>;
    return makeshift::detail::expand_impl<false, R, 0>(std::forward<T>(value), std::forward<TupleT>(tuple));
}


    //ᅟ
    // Given a runtime value, returns a variant of the type-encoded possible values as defined by metadata.
    // Raises std::runtime_error if the runtime value does not appear in the tuple of possible values.
    //ᅟ
    //ᅟ    enum Color { red, green, blue };
    //ᅟ    constexpr inline auto reflect(Color*, serialization_metadata_tag) noexcept { ... }
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
    //ᅟ    constexpr inline auto reflect(Color*, serialization_metadata_tag) noexcept { ... }
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



    //ᅟ
    // Returns a functor that maps a variant with unknown value alternative (as returned by `try_expand()`)  to a variant without (as returned by `expand()`),
    // raising `std::runtime_error` with a detailed error error message if the variant holds an unknown value.
    //ᅟ
    //ᅟ    int runtimeBits = ...;
    //ᅟ    auto bits = try_expand(runtimeBits, std::make_tuple(c<16>, c<32>, c<64>))
    //                | variant_fail_if_unknown("unsupported number of bits", stream_serializer); // returns std::variant<constant<16>, constant<32>, constant<64>>
    //
template <typename SerializerT>
    makeshift::detail::variant_fail_if_unknown_t<remove_rvalue_reference_t<SerializerT>>
    variant_fail_if_unknown(std::string errorMessage, SerializerT&& serializer)
{
    return { std::move(errorMessage), std::forward<SerializerT>(serializer) };
}

    //ᅟ
    // Returns a functor that maps a variant with unknown value alternative (as returned by `try_expand()`)  to a variant without (as returned by `expand()`),
    // raising `std::runtime_error` with a detailed error error message if the variant holds an unknown value.
    //ᅟ
    //ᅟ    int runtimeBits = ...;
    //ᅟ    auto bits = try_expand(runtimeBits, std::make_tuple(c<16>, c<32>, c<64>))
    //                | variant_fail_if_unknown(stream_serializer); // returns std::variant<constant<16>, constant<32>, constant<64>>
    //
template <typename SerializerT,
          typename = std::enable_if_t<is_serializer_v<std::decay_t<SerializerT>>>>
    makeshift::detail::variant_fail_if_unknown_t<remove_rvalue_reference_t<SerializerT>>
    variant_fail_if_unknown(SerializerT&& serializer)
{
    return { { }, std::forward<SerializerT>(serializer) };
}

    //ᅟ
    // Returns a functor that maps a variant with unknown value alternative (as returned by `try_expand()`)  to a variant without (as returned by `expand()`),
    // raising `std::runtime_error` with a detailed error error message if the variant holds an unknown value.
    //ᅟ
    //ᅟ    int runtimeBits = ...;
    //ᅟ    auto bits = try_expand(runtimeBits, std::make_tuple(c<16>, c<32>, c<64>))
    //                | variant_fail_if_unknown("unsupported number of bits"); // returns std::variant<constant<16>, constant<32>, constant<64>>
    //
makeshift::detail::variant_fail_if_unknown_t<stream_serializer_t<>>
variant_fail_if_unknown(std::string errorMessage)
{
    return { std::move(errorMessage), stream_serializer };
}

    //ᅟ
    // Returns a functor that maps a variant with unknown value alternative (as returned by `try_expand()`)  to a variant without (as returned by `expand()`),
    // raising `std::runtime_error` with a detailed error error message if the variant holds an unknown value.
    //ᅟ
    //ᅟ    int runtimeBits = ...;
    //ᅟ    auto bits = try_expand(runtimeBits, std::make_tuple(c<16>, c<32>, c<64>))
    //                | variant_fail_if_unknown(); // returns std::variant<constant<16>, constant<32>, constant<64>>
    //
makeshift::detail::variant_fail_if_unknown_t<stream_serializer_t<>>
variant_fail_if_unknown(void)
{
    return { std::string{ }, stream_serializer };
}


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_EXPAND_HPP_
