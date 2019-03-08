
#ifndef INCLUDED_MAKESHIFT_FUNCTIONAL2_HPP_
#define INCLUDED_MAKESHIFT_FUNCTIONAL2_HPP_


#include <utility>     // for move()
#include <functional>  // for hash<>
#include <type_traits> // for decay<>

#include <makeshift/version.hpp>  // for MAKESHIFT_NODISCARD, MAKESHIFT_EMPTY_BASES

#include <makeshift/detail/functional2.hpp> // for hashable_base


namespace makeshift
{

inline namespace types
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


    //ᅟ
    // Invokes a callable object. (This is a constexpr version of `std::invoke()`.)
    //
template <typename F, typename... ArgsT>
    constexpr decltype(auto) invoke(F&& f, ArgsT&&... args)
{
    return std::forward<F>(f)(std::forward<ArgsT>(args)...);
}

    //ᅟ
    // Invokes a callable object. (This is a constexpr version of `std::invoke()`.)
    //
template <typename F, typename C, typename Arg0T, typename... ArgsT>
    constexpr decltype(auto) invoke(F C::* f, Arg0T&& arg0, ArgsT&&... args)
{
#ifdef MAKESHIFT_CXX17
    if constexpr (std::is_member_function_pointer<F C::*>::value)
    {
        if constexpr (std::is_base_of<C, std::decay_t<Arg0T>>::value)
            return (std::forward<Arg0T>(arg0).*f)(std::forward<ArgsT>(args)...);
        else if constexpr (makeshift::detail::is_reference_wrapper<std::decay_t<Arg0T>>::value)
            return (arg0.get().*f)(std::forward<ArgsT>(args)...);
        else
            return ((*std::forward<Arg0T>(arg0)).*f)(std::forward<ArgsT>(args)...);
    }
    else
    {
        static_assert(std::is_member_object_pointer<F C::*>::value);
        static_assert(sizeof...(ArgsT) == 0);
        if constexpr (std::is_base_of<C, std::decay_t<Arg0T>>::value)
            return std::forward<Arg0T>(arg0).*f;
        else if constexpr (makeshift::detail::is_reference_wrapper<std::decay_t<Arg0T>>::value)
            return arg0.get().*f;
        else
            return (*std::forward<Arg0T>(arg0)).*f;
    }
#else // MAKESHIFT_CXX17
    return makeshift::detail::invoke_impl(std::is_member_function_pointer<F C::*>{ }, f, std::forward<Arg0T>(arg0), std::forward<ArgsT>(args)...);
#endif // MAKESHIFT_CXX17
}


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_FUNCTIONAL2_HPP_
