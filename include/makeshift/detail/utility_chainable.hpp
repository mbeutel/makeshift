
#ifndef INCLUDED_MAKESHIFT_DETAIL_UTILITY_CHAINABLE_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_UTILITY_CHAINABLE_HPP_


#include <tuple>
#include <type_traits> // for decay<>
#include <utility>     // for move(), forward<>()

#include <makeshift/type_traits.hpp> // for type_sequence<>


namespace makeshift
{

namespace detail
{


struct chainable_constructor_tag { };


} // namespace detail


inline namespace types
{


    //ᅟ
    // Base class for chainable classes with argument class `ArgsT`.
    //ᅟ
    //ᅟ    struct MySerializerData { ... };
    //ᅟ    template <typename BaseT = void>
    //ᅟ        struct MySerializer : define_chainable<MySerializer, BaseT, MySerializerData>
    //ᅟ    {
    //ᅟ        using base = define_chainable<MySerializer, BaseT, MySerializerData>;
    //ᅟ        using base::base;
    //ᅟ        ...
    //ᅟ    };
    //
template <template <typename...> class ChainableT, typename BaseT, typename DataT = void, typename RootT = void, typename... Ts>
    struct define_chainable : DataT, BaseT
{
    using chainable_sequence = type_sequence_cat_t<type_sequence<define_chainable<ChainableT, void, DataT, RootT, Ts...>>, typename BaseT::chainable_sequence>;
    using args_sequence = type_sequence_cat_t<type_sequence<DataT>, typename BaseT::args_sequence>;

    constexpr define_chainable(void) = default;
    template <typename... ArgsT>
        constexpr define_chainable(makeshift::detail::chainable_constructor_tag, std::tuple<ArgsT...>&& args)
            : DataT(std::get<DataT>(std::move(args))), BaseT(makeshift::detail::chainable_constructor_tag{ }, std::move(args))
    {
    }
};
template <template <typename...> class ChainableT, typename BaseT, typename RootT, typename... Ts>
    struct define_chainable<ChainableT, BaseT, void, RootT, Ts...> : BaseT
{
    using chainable_sequence = type_sequence_cat_t<type_sequence<define_chainable<ChainableT, void, void, RootT, Ts...>>, typename BaseT::chainable_sequence>;

    constexpr define_chainable(void) = default;
    using BaseT::BaseT;
};
template <template <typename...> class ChainableT, typename DataT, typename RootT, typename... Ts>
    struct define_chainable<ChainableT, void, DataT, RootT, Ts...> : DataT, RootT
{
    using chainable_sequence = type_sequence<define_chainable>;
    using args_sequence = type_sequence<DataT>;

    constexpr define_chainable(void) = default;
    using DataT::DataT;
    template <typename... ArgsT>
        constexpr define_chainable(makeshift::detail::chainable_constructor_tag, std::tuple<ArgsT...>&& args)
            : DataT(std::get<DataT>(std::move(args)))
    {
    }
};
template <template <typename...> class ChainableT, typename DataT, typename... Ts>
    struct define_chainable<ChainableT, void, DataT, void, Ts...> : DataT
{
    using chainable_sequence = type_sequence<define_chainable>;
    using args_sequence = type_sequence<DataT>;

    constexpr define_chainable(void) = default;
    using DataT::DataT;
    template <typename... ArgsT>
        constexpr define_chainable(makeshift::detail::chainable_constructor_tag, std::tuple<ArgsT...>&& args)
            : DataT(std::get<DataT>(std::move(args)))
    {
    }
};
template <template <typename...> class ChainableT, typename RootT, typename... Ts>
    struct define_chainable<ChainableT, void, void, RootT, Ts...> : RootT
{
    using chainable_sequence = type_sequence<define_chainable>;
    using args_sequence = type_sequence<>;

    constexpr define_chainable(void) = default;
    template <typename... ArgsT> constexpr define_chainable(makeshift::detail::chainable_constructor_tag, std::tuple<ArgsT...>&&) { }
};
template <template <typename...> class ChainableT, typename... Ts>
    struct define_chainable<ChainableT, void, void, void, Ts...>
{
    using chainable_sequence = type_sequence<define_chainable>;
    using args_sequence = type_sequence<>;

    constexpr define_chainable(void) = default;
    template <typename... ArgsT> constexpr define_chainable(makeshift::detail::chainable_constructor_tag, std::tuple<ArgsT...>&&) { }
};


} // inline namespace types


namespace detail
{


template <typename... ChainablesT> struct chain_types_0_;
template <> struct chain_types_0_<> { using type = void; };
template <template <typename...> class ChainableT, typename DataT, typename RootT, typename... Ts, typename... ChainablesT>
    struct chain_types_0_<define_chainable<ChainableT, void, DataT, RootT, Ts...>, ChainablesT...>
{
    using type = ChainableT<Ts..., typename chain_types_0_<ChainablesT...>::type>;
};
template <typename ChainablesT> struct chain_types_;
template <typename... ChainablesT> struct chain_types_<type_sequence<ChainablesT...>> : chain_types_0_<ChainablesT...> { };

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
template <typename... ArgsT, typename ChainableT>
    constexpr std::tuple<ArgsT...> get_args_tuple(type_sequence<ArgsT...>, ChainableT&& chainable)
{
    (void) chainable;
    return { as_base<ArgsT, ChainableT>(std::forward<ChainableT>(chainable))... };
}


} // namespace detail


inline namespace types
{


    //ᅟ
    // Chain the given sequence of chainable classes.
    //ᅟ
    //ᅟ    auto serializer = chain(
    //ᅟ        string_quoting_serializer, // hypothetical serializer which encloses strings in quotes and passes them on
    //ᅟ        stream_serializer // serializes strings (and more)
    //ᅟ    );
    //
template <typename... ChainablesT>
    typename makeshift::detail::chain_types_<type_sequence_cat_t<typename std::decay_t<ChainablesT>::chainable_sequence...>>::type
    chain(ChainablesT&&... chainables)
{
    return {
        makeshift::detail::chainable_constructor_tag{ },
        std::tuple_cat(
            makeshift::detail::get_args_tuple(typename std::decay_t<ChainablesT>::args_sequence{ }, std::forward<ChainablesT>(chainables))...
        )
    };
}


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_UTILITY_CHAINABLE_HPP_
