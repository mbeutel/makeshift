
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

template <keyword_crc Name>
    struct get_by_name_t : stream_base<get_by_name_t<Name>>
{
private:
    template <typename TupleT>
        static constexpr auto invoke(TupleT&& tuple)
    {
        using DTuple = std::decay_t<TupleT>;
        constexpr std::size_t matchIndex = makeshift::detail::tuple_kw_index<Name, DTuple>(std::make_index_sequence<std::tuple_size<DTuple>::value>{ });
        return contextual_value(std::get<matchIndex>(std::forward<TupleT>(tuple)));
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
    //ᅟ    auto tuple = std::make_tuple(name<"width"_kw> = 42);
    //ᅟ    int width = tuple
    //ᅟ        | get_by_name<"width"_kw>(); // returns 42
    //
template <makeshift::detail::keyword_crc Name>
    constexpr makeshift::detail::get_by_name_t<Name> get_by_name(void) noexcept
{
    return { };
}


    //ᅟ
    // Retrieves a tuple element by keyword name.
    //ᅟ
    //ᅟ    auto tuple = std::make_tuple(name<"width"_kw> = 42);
    //ᅟ    int width = get_by_name<"width"_kw>(tuple); // returns 42
    //
template <makeshift::detail::keyword_crc Name, typename TupleT,
          typename = std::enable_if_t<is_tuple_like_v<std::decay_t<TupleT>>>>
    constexpr decltype(auto) get_by_name(TupleT&& tuple) noexcept
{
    return get_by_name<Name>()(std::forward<TupleT>(tuple));
}


} // inline namespace types

} // namespace makeshift


#endif // MAKESHIFT_DETAIL_UTILITY_KEYWORD_TUPLE_HPP_
