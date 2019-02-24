
#ifndef INCLUDED_MAKESHIFT_FUNCTIONAL2_HPP_
#define INCLUDED_MAKESHIFT_FUNCTIONAL2_HPP_


#include <functional> // for hash<>

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


} // namespace detail


inline namespace types
{


    //ᅟ
    // Like `std::hash<>` but permits conditional specialization with `enable_if<>`.
    //
template <typename KeyT = void, typename = void>
    struct hash2 : std::hash<KeyT>
{
    using std::hash<KeyT>::hash;
};
template <>
    struct hash2<void, void>
{
        //ᅟ
        // Computes a hash value of the given argument.
        //
    template <typename T>
        constexpr std::size_t operator ()(const T& arg) const noexcept
    {
        return hash2<T>{ }(arg);
    }
};


    //ᅟ
    // Computes a hash value of the given argument.
    //
static constexpr inline hash2<> hash_of = { };





} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_FUNCTIONAL2_HPP_
