
#ifndef INCLUDED_MAKESHIFT_FUNCTIONAL_HPP_
#define INCLUDED_MAKESHIFT_FUNCTIONAL_HPP_


#include <utility>     // for move(), forward<>()
#include <cstddef>     // for size_t
#include <functional>  // for hash<>
#include <type_traits> // for decay<>, declval<>()

#include <makeshift/macros.hpp>  // for MAKESHIFT_CXX, MAKESHIFT_NODISCARD, MAKESHIFT_DETAIL_EMPTY_BASES

#include <makeshift/detail/functional.hpp>


namespace makeshift
{


#if MAKESHIFT_CXXLEVEL >= 17
    //ᅟ
    // Constructs a functor wrapper that selects the matching overload among a number of given functors.
    //ᅟ
    //ᅟ    auto type_name_func = overloaded(
    //ᅟ        [](int)   { return "int"; },
    //ᅟ        [](float) { return "float"; },
    //ᅟ        [](auto)  { return "unknown"; }
    //ᅟ    );
    //
template <typename... Fs>
    struct MAKESHIFT_DETAIL_EMPTY_BASES overloaded : Fs...
{
    using Fs::operator ()...;
};
#else // MAKESHIFT_CXXLEVEL >= 17
    //ᅟ
    // Constructs a functor wrapper that selects the matching overload among a number of given functors.
    //ᅟ
    //ᅟ    auto type_name_func = overloaded(
    //ᅟ        [](int)   { return "int"; },
    //ᅟ        [](float) { return "float"; },
    //ᅟ        [](auto)  { return "unknown"; }
    //ᅟ    );
    //
template <typename... Fs>
    struct overloaded;
template <typename F0>
    struct MAKESHIFT_DETAIL_EMPTY_BASES overloaded<F0> : F0
{
    using F0::F0;
};
template <typename F0, typename... Fs>
    struct MAKESHIFT_DETAIL_EMPTY_BASES overloaded<F0, Fs...> : F0, overloaded<Fs...>
{
    constexpr overloaded(F0 f0, Fs... fs) : F0(std::move(f0)), overloaded<Fs...>(std::move(fs)...) { }
    using F0::operator ();
    using overloaded<Fs...>::operator ();
};
#endif // MAKESHIFT_CXXLEVEL >= 17

#if MAKESHIFT_CXX >= 17
template <typename... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;
#endif // MAKESHIFT_CXX >= 17

template <typename... Fs>
    constexpr overloaded<Fs...> make_overloaded(Fs... fs)
{
    return { std::move(fs)... };
}


    //ᅟ
    // Higher-order function for defining recursive lambda functions.
    // Note that the lambda function must explicitly declare a return type.
    //ᅟ
    //ᅟ    auto fac = y_combinator{ 
    //ᅟ        [](auto fac, int i) -> int
    //ᅟ        {
    //ᅟ            return i <= 1
    //ᅟ                ? 1
    //ᅟ                : i * fac(i - 1);
    //ᅟ        }
    //ᅟ    };
    //ᅟ    int i = fac(4); // returns 24
    //
template <typename F>
    class y_combinator
{
private:
    F func_;

public:
    constexpr explicit y_combinator(F&& _func)
        : func_(std::forward<F>(_func))
    {
    }

    template <typename... ArgsT>
        constexpr decltype(auto) operator()(ArgsT&&... args)
    {
        return func_(makeshift::detail::y_combinator_func_ref<F&>{ func_ }, std::forward<ArgsT>(args)...);
    }
    template <typename... ArgsT>
        constexpr decltype(auto) operator()(ArgsT&&... args) const
    {
        return func_(makeshift::detail::y_combinator_func_ref<F const&>{ func_ }, std::forward<ArgsT>(args)...);
    }
};
#if MAKESHIFT_CXX >= 17
template <typename F>
    y_combinator(F&& func) -> y_combinator<std::decay_t<F>>;
#endif // MAKESHIFT_CXX >= 17


    //ᅟ
    // Similar to `std::hash<>` but permits omitting the type argument and supports conditional specialization with `enable_if_t<>` or `void_t<>`.
    //
template <typename KeyT = void, typename = void>
    struct hash : std::hash<KeyT> // TODO: do we still need/want this?
{
    using std::hash<KeyT>::hash;
};
template <>
    struct hash<void>
{
        //ᅟ
        // Computes a hash value of the given argument.
        //
    template <typename T>
        MAKESHIFT_NODISCARD constexpr decltype(hash<T>{ }(std::declval<T const&>())) // SFINAE
        operator ()(T const& arg) const noexcept
    {
        return hash<T>{ }(arg);
    }
};


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_FUNCTIONAL_HPP_
