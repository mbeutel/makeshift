
#ifndef INCLUDED_MAKESHIFT_DETAIL_FUNCTIONAL2_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_FUNCTIONAL2_HPP_


#include <utility>     // for move()
#include <type_traits> // for enable_if<>, conjunction<>, is_default_constructible<>

#include <makeshift/version.hpp>  // for MAKESHIFT_EMPTY_BASES


namespace makeshift
{

namespace detail
{


template <typename... Ts>
    struct MAKESHIFT_EMPTY_BASES adapter_base : Ts...
{
    template <typename = std::enable_if_t<std::conjunction_v<std::is_default_constructible<Ts>...>>>
        constexpr adapter_base(void)
    {
    }
    constexpr adapter_base(Ts... args) : Ts(std::move(args))... { }
    constexpr adapter_base(const adapter_base&) = default;
    constexpr adapter_base(adapter_base&&) = default;
    constexpr adapter_base& operator =(const adapter_base&) = default;
    constexpr adapter_base& operator =(adapter_base&&) = default;
};


struct hashable_base { };


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_FUNCTIONAL2_HPP_
