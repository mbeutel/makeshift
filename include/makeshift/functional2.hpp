
#ifndef INCLUDED_MAKESHIFT_FUNCTIONAL2_HPP_
#define INCLUDED_MAKESHIFT_FUNCTIONAL2_HPP_


#include <utility>     // for move()
#include <functional>  // for hash<>
#include <type_traits> // for decay<>

#include <makeshift/version.hpp>  // for MAKESHIFT_NODISCARD, MAKESHIFT_EMPTY_BASES


namespace makeshift
{


    //ᅟ
    // Constructs a functor wrapper that selects the matching overload among a number of given functors.
    //ᅟ
    //ᅟ    auto type_name_func = overload(
    //ᅟ        [](int)   { return "int"; },
    //ᅟ        [](float) { return "float"; },
    //ᅟ        [](auto)  { return "unknown"; }
    //ᅟ    );
    //
template <typename... Fs>
    struct MAKESHIFT_EMPTY_BASES overload2 : Fs...
{
    constexpr overload2(Fs... fs) : Fs(std::move(fs))... { }
    using Fs::operator ()...;
};
template <typename... Ts>
    overload2(Ts&&...) -> overload2<std::decay_t<Ts>...>;


    //ᅟ
    // Similar to `std::hash<>` but permits omitting the type argument and conditional specialization with `enable_if<>`.
    //
template <typename KeyT = void, typename = void>
    struct hash2 : std::hash<KeyT>
{
    using std::hash<KeyT>::hash;
};
template <>
    struct hash2<void>
{
        //ᅟ
        // Computes a hash value of the given argument.
        //
    template <typename T,
              typename = decltype(hash2<T>{ }(std::declval<const T&>()))>
        MAKESHIFT_NODISCARD constexpr std::size_t operator ()(const T& arg) const noexcept
    {
        return hash2<T>{ }(arg);
    }
};


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_FUNCTIONAL2_HPP_
