
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
struct chainable_default_root { };


} // namespace detail


inline namespace types
{


    //ᅟ
    // Base class for chainable classes with argument class `DataT`.
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
    struct define_chainable : BaseT
{
    using chainable_sequence = type_sequence_cat_t<type_sequence<makeshift::define_chainable<ChainableT, void, DataT, RootT, Ts...>>, typename BaseT::chainable_sequence>;
    using args_sequence = type_sequence_cat_t<type_sequence<DataT>, typename BaseT::args_sequence>;

private:
    DataT data_;

public:
    constexpr friend const DataT& data(const define_chainable& chainable) noexcept { return chainable.data_; }
    constexpr friend DataT& data(define_chainable& chainable) noexcept { return chainable.data_; }
    constexpr friend DataT&& data(define_chainable&& chainable) noexcept { return std::move(chainable.data_); }
    constexpr friend const DataT& data(const define_chainable& chainable, tag<DataT>) noexcept { return chainable.data_; }
    constexpr friend DataT& data(define_chainable& chainable, tag<DataT>) noexcept { return chainable.data_; }
    constexpr friend DataT&& data(define_chainable&& chainable, tag<DataT>) noexcept { return std::move(chainable.data_); }

    constexpr define_chainable(void) = default;
    template <typename... ArgsT>
        constexpr define_chainable(makeshift::detail::chainable_constructor_tag, std::tuple<ArgsT...>&& args)
            : BaseT(makeshift::detail::chainable_constructor_tag{ }, std::move(args)), data_(std::get<DataT>(std::move(args)))
    {
    }
};
template <template <typename...> class ChainableT, typename BaseT, typename RootT, typename... Ts>
    struct define_chainable<ChainableT, BaseT, void, RootT, Ts...> : BaseT
{
    using chainable_sequence = type_sequence_cat_t<type_sequence<makeshift::define_chainable<ChainableT, void, void, RootT, Ts...>>, typename BaseT::chainable_sequence>;

    constexpr define_chainable(void) = default;
    template <typename... ArgsT>
        constexpr define_chainable(makeshift::detail::chainable_constructor_tag, std::tuple<ArgsT...>&& args)
            : BaseT(makeshift::detail::chainable_constructor_tag{ }, std::move(args))
    {
    }
};
template <template <typename...> class ChainableT, typename DataT, typename RootT, typename... Ts>
    struct define_chainable<ChainableT, void, DataT, RootT, Ts...> : std::conditional_t<std::is_void<RootT>::value, makeshift::detail::chainable_default_root, RootT>
{
    using chainable_sequence = type_sequence<define_chainable>;
    using args_sequence = type_sequence<DataT>;

private:
    DataT data_;

public:
    constexpr friend const DataT& data(const define_chainable& chainable) noexcept { return chainable.data_; }
    constexpr friend DataT& data(define_chainable& chainable) noexcept { return chainable.data_; }
    constexpr friend DataT&& data(define_chainable&& chainable) noexcept { return std::move(chainable.data_); }
    constexpr friend const DataT& data(const define_chainable& chainable, tag<DataT>) noexcept { return chainable.data_; }
    constexpr friend DataT& data(define_chainable& chainable, tag<DataT>) noexcept { return chainable.data_; }
    constexpr friend DataT&& data(define_chainable&& chainable, tag<DataT>) noexcept { return std::move(chainable.data_); }

    constexpr define_chainable(void) = default;
    constexpr define_chainable(const DataT& _data) : data_(_data) { }
    constexpr define_chainable(DataT&& _data) : data_(std::move(_data)) { }
    template <typename... ArgsT>
        constexpr define_chainable(makeshift::detail::chainable_constructor_tag, std::tuple<ArgsT...>&& args)
            : data_(std::get<DataT>(std::move(args)))
    {
    }
};
template <template <typename...> class ChainableT, typename RootT, typename... Ts>
    struct define_chainable<ChainableT, void, void, RootT, Ts...> : std::conditional_t<std::is_void<RootT>::value, makeshift::detail::chainable_default_root, RootT>
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

template <typename... ArgsT, typename ChainableT>
    constexpr std::tuple<ArgsT...> get_args_tuple(type_sequence<ArgsT...>, ChainableT&& chainable)
{
    (void) chainable;
    return { data(std::forward<ChainableT>(chainable), tag_v<ArgsT>)... };
}


} // namespace detail


inline namespace types
{


    //ᅟ
    // Chain the given sequence of chainable classes.
    //ᅟ
    //ᅟ    auto serializer = chain(
    //ᅟ        bool_serializer(bool_serializer_options{ "yes", "no" }),
    //ᅟ        stream_serializer{ }
    //ᅟ    );
    //
template <typename... ChainablesT>
    constexpr typename makeshift::detail::chain_types_<typename type_sequence_cat<typename std::decay_t<ChainablesT>::chainable_sequence...>::type>::type
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
