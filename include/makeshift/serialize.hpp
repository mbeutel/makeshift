
#ifndef MAKESHIFT_SERIALIZE_EHPP_
#define MAKESHIFT_SERIALIZE_EHPP_


#include <tuple>
#include <utility>     // for move(), forward<>()
#include <type_traits> // for decay<>

#include <makeshift/type_traits.hpp> // for tag<>, type_sequence<>, type_sequence_cat<>


namespace makeshift
{

inline namespace serialize
{


    //ᅟ
    // Base class for serializers with argument class `SerializerArgsT`. Permits chaining of serializers.
    //ᅟ
    //ᅟ    struct MySerializerArgs { ... };
    //ᅟ    template <typename BaseT = void>
    //ᅟ        struct MySerializer : define_serializer<MySerializer, BaseT, MySerializerArgs>
    //ᅟ    {
    //ᅟ        using base = define_serializer<MySerializer, BaseT, MySerializerArgs>;
    //ᅟ        using base::base;
    //ᅟ        ...
    //ᅟ    };
    //
template <template <typename...> class SerializerT, typename BaseT = void, typename SerializerArgsT = void, typename... Ts>
    struct define_serializer : SerializerArgsT, BaseT
{
    using args_sequence = type_sequence_cat_t<type_sequence<SerializerArgsT>, typename BaseT::args_sequence>;

    constexpr define_serializer(void) = default;
    template <typename... ArgsT>
        constexpr define_serializer(std::tuple<ArgsT...>&& args)
            : SerializerArgsT(std::get<SerializerArgsT>(std::move(args))), BaseT(std::move(args))
    {
    }
};
template <template <typename...> class SerializerT, typename BaseT, typename... Ts>
    struct define_serializer<SerializerT, BaseT, void, Ts...> : BaseT
{
    constexpr define_serializer(void) = default;
    using BaseT::BaseT;
};
template <template <typename...> class SerializerT, typename SerializerArgsT, typename... Ts>
    struct define_serializer<SerializerT, void, SerializerArgsT, Ts...> : SerializerArgsT
{
    using args_sequence = type_sequence<SerializerArgsT>;

    constexpr define_serializer(void) = default;
    using SerializerArgsT::SerializerArgsT;
    template <typename... ArgsT>
        constexpr define_serializer(std::tuple<ArgsT...>&& args)
            : SerializerArgsT(std::get<SerializerArgsT>(std::move(args)))
    {
    }
};
template <template <typename...> class SerializerT, typename... Ts>
    struct define_serializer<SerializerT, void, void, Ts...>
{
    using args_sequence = type_sequence<>;
    constexpr define_serializer(void) = default;
    template <typename... ArgsT> constexpr define_serializer(std::tuple<ArgsT...>&&) { }
};


} // inline namespace serialize


namespace detail
{


template <template <typename...> class SerializerT, typename SerializerArgsT, typename... Ts>
    constexpr tag_t<define_serializer<SerializerT, void, SerializerArgsT, Ts...>> get_serializer_definition(const define_serializer<SerializerT, void, SerializerArgsT, Ts...>&)
{
    return { };
}

template <typename... DefsT> struct chain_serializer_types;
template <> struct chain_serializer_types<> { using type = void; };
template <template <typename...> class SerializerT, typename SerializerArgsT, typename... Ts, typename... DefsT>
    struct chain_serializer_types<tag_t<define_serializer<SerializerT, void, SerializerArgsT, Ts...>>, DefsT...>
{
    using type = SerializerT<Ts..., typename chain_serializer_types<DefsT...>::type>;
};

template <typename BaseT, typename DerivedT>
    constexpr BaseT&& as_base(std::decay_t<DerivedT>&& derived)
{
    return static_cast<BaseT&&>(derived);
}
template <typename BaseT, typename DerivedT>
    constexpr const BaseT& as_base(const std::decay_t<DerivedT>& derived)
{
    return derived;
}
template <typename BaseT, typename DerivedT>
    constexpr BaseT& as_base(std::decay_t<DerivedT>& derived)
{
    return derived;
}
template <typename... ArgsT, typename SerializerT>
    constexpr std::tuple<ArgsT...> get_args_tuple(type_sequence<ArgsT...>, SerializerT&& serializer)
{
    return { as_base<ArgsT, SerializerT>(std::forward<SerializerT>(serializer))... };
}


} // namespace detail


inline namespace serialize
{


    //ᅟ
    // Chain the given sequence of serializers. Unlike combining unrelated serializers with `combine()`, chaining permits that a serializer
    // partially overrides the behavior of a serializer further down the list.
    //ᅟ
    //ᅟ    auto serializer = chain_serializers(
    //ᅟ        string_quoting_serializer, // hypothetical serializer which encloses strings in quotes and passes them on
    //ᅟ        stream_serializer // serializes strings (and more)
    //ᅟ    );
    //
template <typename... SerializersT>
    auto chain_serializers(SerializersT&&... serializers)
{
    auto argsTuple = std::tuple_cat(
        makeshift::detail::get_args_tuple(typename std::decay_t<SerializersT>::args_sequence{ }, std::forward<SerializersT>(serializers))...
    );
    using ChainedSerializer = typename makeshift::detail::chain_serializer_types<decltype(makeshift::detail::get_serializer_definition(serializers))...>::type;
    return ChainedSerializer{ std::move(argsTuple) };
}


} // inline namespace serialize

} // namespace makeshift


#endif // MAKESHIFT_SERIALIZE_EHPP_
