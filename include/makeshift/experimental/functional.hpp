
#ifndef INCLUDED_MAKESHIFT_EXPERIMENTAL_FUNCTIONAL_HPP_
#define INCLUDED_MAKESHIFT_EXPERIMENTAL_FUNCTIONAL_HPP_


#include <utility> // for move(), forward<>()

#include <gsl-lite/gsl-lite.hpp> // for gsl_CPP17_OR_GREATER


namespace makeshift {


namespace gsl = ::gsl_lite;


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


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_EXPERIMENTAL_FUNCTIONAL_HPP_
