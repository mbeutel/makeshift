
#ifndef INCLUDED_MAKESHIFT_DETAIL_UTILITY_KEYWORD_TUPLE_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_UTILITY_KEYWORD_TUPLE_HPP_


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

template <keyword_crc Name>
    struct get_by_key_t : stream_base<get_by_key_t<Name>>
{
private:
    template <typename TupleT>
        static constexpr auto invoke(TupleT&& tuple)
    {
        using DTuple = std::decay_t<TupleT>;
        constexpr std::size_t matchIndex = makeshift::detail::tuple_kw_index<Name, DTuple>(std::make_index_sequence<std::tuple_size<DTuple>::value>{ });
        using std::get; // make std::get<>(std::pair<>&&) visible to enable ADL for template methods named get<>()
        return contextual_value(get<matchIndex>(std::forward<TupleT>(tuple)));
    }
public:
    template <typename TupleT,
              typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
        constexpr auto operator ()(TupleT&& tuple) const
    {
        return invoke(std::forward<TupleT>(tuple));
    }
};


} // namespace detail


inline namespace types
{


    //ᅟ
    // Returns a functor which retrieves the tuple element with the given keyword name.
    //ᅟ
    //ᅟ    auto tuple = std::tuple{ name<"width"_k> = 42 };
    //ᅟ    int width = tuple
    //ᅟ        | get_by_name(name<"width"_k>); // returns 42
    //
template <makeshift::detail::keyword_crc Name>
    constexpr makeshift::detail::get_by_key_t<Name> get_by_key(name_<Name> = { }) noexcept
{
    return { };
}


    //ᅟ
    // Retrieves a tuple element by keyword name.
    //ᅟ
    //ᅟ    auto tuple = std::tuple{ name<"width"_k> = 42 };
    //ᅟ    int width = get_by_name(tuple, name<"width"_k>); // returns 42
    //
template <makeshift::detail::keyword_crc Name, typename TupleT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr decltype(auto) get_by_key(TupleT&& tuple, name_<Name> = { }) noexcept
{
    return get_by_key<Name>()(std::forward<TupleT>(tuple));
}


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_UTILITY_KEYWORD_TUPLE_HPP_
