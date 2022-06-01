
#ifndef INCLUDED_MAKESHIFT_EXPERIMENTAL_FUNCTIONAL_HPP_
#define INCLUDED_MAKESHIFT_EXPERIMENTAL_FUNCTIONAL_HPP_


#include <utility>      // for move(), forward<>()
#include <type_traits>  // for move(), forward<>()

#include <gsl-lite/gsl-lite.hpp>  // for gsl_CPP17_OR_GREATER

#if !gsl_CPP17_OR_GREATER
# error makeshift requires C++17 mode or higher
#endif // !gsl_CPP17_OR_GREATER

#include <makeshift/experimental/detail/functional.hpp>


namespace makeshift {


    //
    // Forwards 
    //ᅟ
    //ᅟ    auto v = std::vector<int>{ ... };
    //ᅟ    auto f = forward_to(
    //ᅟ        [context = ...]()
    //ᅟ        {
    //ᅟ            return [&](auto forward_capture){
    //ᅟ            {
    //ᅟ                g(forward_capture(context));
    //ᅟ            };
    //ᅟ        });
    //
template <typename F>
detail::forward_to_impl<F>
forward_to(F func)
{
    return detail::forward_to_impl<F>{ std::move(func) };
}

// Usage:
//
//     auto f = forward_to(
//         [shared_state]
//         {
//             return overload(
//                 [&](int i) { g(i, shared_state); },
//                 [&](float f) { h(f, shared_state); });
//         });


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
