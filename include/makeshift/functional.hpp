
#ifndef INCLUDED_MAKESHIFT_FUNCTIONAL_HPP_
#define INCLUDED_MAKESHIFT_FUNCTIONAL_HPP_


#include <utility> // for move(), forward<>()

#include <makeshift/macros.hpp>  // for MAKESHIFT_CXX, MAKESHIFT_DETAIL_EMPTY_BASES

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
    //ᅟ        [](auto)  { return "unknown"; });
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
    //ᅟ        [](auto)  { return "unknown"; });
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
    //ᅟ    auto fac = y_combinator( 
    //ᅟ        [](auto fac, int i) -> int
    //ᅟ        {
    //ᅟ            return i <= 1
    //ᅟ                ? 1
    //ᅟ                : i * fac(i - 1);
    //ᅟ        });
    //ᅟ    int i = fac(4); // returns 24
    //
template <typename F>
    class y_combinator
{
private:
    F func_;

public:
    constexpr explicit y_combinator(F _func)
        : func_(std::move(_func))
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
    y_combinator(F) -> y_combinator<F>;
#endif // MAKESHIFT_CXX >= 17


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_FUNCTIONAL_HPP_
