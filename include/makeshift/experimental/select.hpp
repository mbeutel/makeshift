
#ifndef INCLUDED_MAKESHIFT_EXPERIMENTAL_SELECT_HPP_
#define INCLUDED_MAKESHIFT_EXPERIMENTAL_SELECT_HPP_


#include <type_traits> // for integral_constant<>, is_same<>, decay<>
#include <utility>     // for forward<>(), pair<>
#include <tuple>       // for apply()

#include <makeshift/type_traits.hpp> // for c<>, tag<>, sequence<>
#include <makeshift/tuple.hpp>
#include <makeshift/utility.hpp>     // for ??
#include <makeshift/metadata.hpp>    // for have_metadata<>, metadata_of<>


namespace makeshift
{

namespace detail
{


struct accumulate_sequence_t
{
    template <typename T, T... Ls, T R>
        constexpr sequence<T, Ls..., R> operator ()(sequence<T, Ls...>, std::integral_constant<T, R>) const noexcept
    {
        return { };
    }
};

template <typename T> using is_value_metadata = is_same_template<T, value_metadata>;
template <typename T, typename MetadataTagT = serialization_metadata_tag>
    auto static_get_values(tag_t<T> = { }, tag_t<MetadataTagT> = { })
{
    if constexpr (std::is_same<T, bool>::value)
        return sequence<bool, false, true>{ };
    else if constexpr (std::is_enum<T>::value && have_metadata_v<T, MetadataTagT>)
    {
        return metadata_of<T, MetadataTagT>.attributes
            | tuple_filter<is_value_metadata>()
            | tuple_map([](const auto& v) { return std::integral_constant<T, std::decay_t<decltype(v)>::value>{ }; })
            | tuple_reduce(sequence<T>{ }, accumulate_sequence_t{ });
    }
}

template <typename T, T Key, typename ValT, typename F>
    bool static_map_value(const T& runtimeKey, constant<Key>, ValT&& value, const F& func)
{
    if (runtimeKey == Key)
    {
        func(std::forward<ValT>(value));
        return true;
    }
    return false;
}


template <typename R>
    R static_map_impl_throw(void)
{
    throw std::runtime_error("unknown key"); // TODO: we can do better than this!
}
template <typename R, typename T, typename TupleT, std::size_t I, typename F>
    auto static_map_impl(const T& runtimeKey, TupleT&& map, constant<I>, F&& func);
template <typename R, typename T, typename TupleT, std::size_t I, typename F>
    auto static_map_impl_recur(const T& runtimeKey, TupleT&& map, constant<I>, F&& func)
{
    if (runtimeKey == std::get<I>(map).first)
        return func(std::get<I>(map).second);
    else
        return static_map_impl<decltype(func(std::get<I>(map).second))>(runtimeKey, std::forward<TupleT>(map), c<I + 1>, std::forward<F>(func));
}
template <typename R, typename T, typename TupleT, std::size_t I, typename F>
    auto static_map_impl(const T& runtimeKey, TupleT&& map, constant<I>, F&& func)
{
    (void) runtimeKey;
    (void) map;
    (void) func;
    constexpr std::size_t numElements = std::tuple_size<std::decay_t<TupleT>>::value;
    if constexpr (I == numElements)
        return static_map_impl_throw<R>();
    else
        return static_map_impl_recur<R>(runtimeKey, std::forward<TupleT>(map), c<I>, std::forward<F>(func));
}


} // namespace detail


inline namespace types
{


template <typename T, T... Vs, typename F>
    auto static_select(const T&, sequence<T>, F&& func) -> decltype(func(c<T{ }>))
{
    throw std::runtime_error("unknown value"); // TODO: we can do better than this!
}
template <typename T, T V0, T... Vs, typename F>
    auto static_select(const T& value, sequence<T, V0, Vs...>, F&& func) -> decltype(func(c<T{ }>))
{
    if (value == V0)
        return func(c<V0>);
    else
        return static_select(value, sequence<T, Vs...>{ }, std::forward<F>(func));
}
template <typename T, typename MetadataTagT, typename F>
    auto static_select(const T& value, tag_t<MetadataTagT>, F&& func)
{
    return static_select(value, makeshift::detail::static_get_values(tag<T>, tag<MetadataTagT>), std::forward<F>(func));
}
template <typename T, typename F>
    auto static_select(const T& value, F&& func)
{
    return static_select(value, makeshift::detail::static_get_values(tag<T>), std::forward<F>(func));
}


template <typename T, typename TupleT, typename F,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    auto static_map(const T& key, TupleT&& map, F&& func)
{
    return makeshift::detail::static_map_impl<void>(key, std::forward<TupleT>(map), c<std::size_t(0)>, std::forward<F>(func));
}


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_EXPERIMENTAL_SELECT_HPP_
