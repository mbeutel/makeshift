
#ifndef INCLUDED_MAKESHIFT_FUNCTIONAL_HPP_
#define INCLUDED_MAKESHIFT_FUNCTIONAL_HPP_


#include <utility> // for move(), forward<>()

#include <gsl/gsl-lite.hpp> // for gsl_CPP17_OR_GREATER

#include <makeshift/detail/macros.hpp>     // for MAKESHIFT_DETAIL_CXXLEVEL, MAKESHIFT_DETAIL_EMPTY_BASES
#include <makeshift/detail/functional.hpp>


namespace makeshift
{


#if MAKESHIFT_DETAIL_CXXLEVEL >= 17
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
#else // ^^^ MAKESHIFT_DETAIL_CXXLEVEL >= 17 ^^^ / vvv MAKESHIFT_DETAIL_CXXLEVEL < 17 vvv
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
#endif // MAKESHIFT_DETAIL_CXXLEVEL >= 17

#if gsl_CPP17_OR_GREATER
template <typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;
#endif // gsl_CPP17_OR_GREATER

template <typename... Fs>
constexpr overloaded<Fs...> make_overloaded(Fs... fs)
{
    return { std::move(fs)... };
}


template <typename F>
struct rvalue_ref_fn : F
{
    constexpr rvalue_ref_fn(F func) : F(std::move(func)) { }
    template <typename... Ts>
    constexpr auto operator ()(Ts&&... args) &&
        -> decltype(static_cast<F&&>(*this)(std::forward<Ts>(args)...))
    {
        return static_cast<F&&>(*this)(std::forward<Ts>(args)...);
    }
};
#if gsl_CPP17_OR_GREATER
template <typename F>
rvalue_ref_fn(F) -> rvalue_ref_fn<F>;
#endif // gsl_CPP17_OR_GREATER

template <typename F>
constexpr rvalue_ref_fn<F> make_rvalue_ref_fn(F func)
{
    return { std::move(func) };
}


template <typename F>
struct lvalue_ref_fn : F
{
    constexpr lvalue_ref_fn(F func) : F(std::move(func)) { }
    template <typename... Ts>
    constexpr auto operator ()(Ts&&... args) &
        -> decltype(static_cast<F&>(*this)(std::forward<Ts>(args)...))
    {
        return static_cast<F&>(*this)(std::forward<Ts>(args)...);
    }
};
#if gsl_CPP17_OR_GREATER
template <typename F>
lvalue_ref_fn(F) -> lvalue_ref_fn<F>;
#endif // gsl_CPP17_OR_GREATER

template <typename F>
constexpr lvalue_ref_fn<F> make_lvalue_ref_fn(F func)
{
    return { std::move(func) };
}


template <typename F>
struct lvalue_const_ref_fn : F
{
    constexpr lvalue_const_ref_fn(F func) : F(std::move(func)) { }
    template <typename... Ts>
    constexpr auto operator ()(Ts&&... args) const&
        -> decltype(static_cast<F const&>(*this)(std::forward<Ts>(args)...))
    {
        return static_cast<F const&>(*this)(std::forward<Ts>(args)...);
    }
};
#if gsl_CPP17_OR_GREATER
template <typename F>
lvalue_const_ref_fn(F) -> lvalue_const_ref_fn<F>;
#endif // gsl_CPP17_OR_GREATER

template <typename F>
constexpr lvalue_const_ref_fn<F> make_lvalue_const_ref_fn(F func)
{
    return { std::move(func) };
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
        return func_(detail::y_combinator_func_ref<F&>{ func_ }, std::forward<ArgsT>(args)...);
    }
    template <typename... ArgsT>
    constexpr decltype(auto) operator()(ArgsT&&... args) const
    {
        return func_(detail::y_combinator_func_ref<F const&>{ func_ }, std::forward<ArgsT>(args)...);
    }
};
#if gsl_CPP17_OR_GREATER
template <typename F>
y_combinator(F) -> y_combinator<F>;
#endif // gsl_CPP17_OR_GREATER

template <typename F>
constexpr y_combinator<F> make_y_combinator(F func)
{
    return y_combinator<F>{ std::move(func) };
}


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_FUNCTIONAL_HPP_
