
#ifndef INCLUDED_MAKESHIFT_EXPERIMENTAL_DETAIL_UTILITY_HPP_
#define INCLUDED_MAKESHIFT_EXPERIMENTAL_DETAIL_UTILITY_HPP_


#include <makeshift/detail/type_traits.hpp>  // for search_type_pack_index<>, any_sink


namespace makeshift {


template <typename T>
struct type_tag;


namespace detail {


template <typename T, typename TypeSeqT> struct search_type_pack_index_in;
template <typename T, template <typename...> class TypeSeqT, typename... Ts> struct search_type_pack_index_in<T, TypeSeqT<Ts...>> : search_type_pack_index<T, Ts...> { };


template <typename T>
struct type_base : constval_tag
{
    using type = T;
};


template <typename T>
struct type_tag_proxy
{
    constexpr operator type_tag<T> const&(void) const noexcept;
    constexpr type_tag<T> const* operator &(void) const noexcept
    {
        return &static_cast<type_tag<T> const&>(*this);
    }
};


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_EXPERIMENTAL_DETAIL_UTILITY_HPP_
