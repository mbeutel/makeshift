
#ifndef MAKESHIFT_DETAIL_UTILITY_KEYWORD_TUPLE_HPP_
#define MAKESHIFT_DETAIL_UTILITY_KEYWORD_TUPLE_HPP_


#include <makeshift/detail/utility_keyword.hpp>
#include <makeshift/tuple.hpp>


namespace makeshift
{

namespace detail
{

template <makeshift::detail::keyword_crc Name, typename TupleT, std::size_t... Is>
    constexpr std::size_t tuple_kw_index(std::index_sequence<Is...>) noexcept
{
    constexpr int numMatches = (0 + ... + ((name_of_v<std::tuple_element_t<Is, TupleT>> == Name) ? 1 : 0));
    constexpr std::size_t matchIndex = (0 + ... + ((name_of_v<std::tuple_element_t<Is, TupleT>> == Name) ? Is : 0));
    static_assert(numMatches >= 1, "argument with given keyword not found in tuple");
    static_assert(numMatches <= 1, "more than one keyword arguments match the given keyword");
    return matchIndex;
}

} // namespace detail

inline namespace types
{

template <makeshift::detail::keyword_crc Name, typename TupleT>
    constexpr decltype(auto) get(TupleT&& tuple) noexcept
{
    using DTuple = std::decay_t<TupleT>;
    constexpr std::size_t matchIndex = makeshift::detail::tuple_kw_index<Name, DTuple>(std::make_index_sequence<std::tuple_size<DTuple>::value>{ });
    return contextual_value(std::get<matchIndex>(std::forward<TupleT>(tuple)));
}

} // inline namespace types

} // namespace makeshift

#endif // MAKESHIFT_DETAIL_UTILITY_KEYWORD_TUPLE_HPP_
