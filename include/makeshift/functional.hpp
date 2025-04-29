
#ifndef INCLUDED_MAKESHIFT_FUNCTIONAL_HPP_
#define INCLUDED_MAKESHIFT_FUNCTIONAL_HPP_


#include <utility>  // for move(), forward<>()

#include <gsl-lite/gsl-lite.hpp>  // for gsl_CPP20_OR_GREATER

#if !gsl_CPP20_OR_GREATER
# error makeshift requires C++20 mode or higher
#endif // !gsl_CPP20_OR_GREATER

#include <makeshift/detail/macros.hpp>      // for MAKESHIFT_DETAIL_EMPTY_BASES
#include <makeshift/detail/functional.hpp>


namespace makeshift {

namespace gsl = ::gsl_lite;


    //
    // Constructs a functor wrapper that selects the matching overload among a number of given functors.
    //ᅟ
    //ᅟ    auto type_name_func = overload(
    //ᅟ        [](int)   { return "int"; },
    //ᅟ        [](float) { return "float"; },
    //ᅟ        [](auto)  { return "unknown"; });
    //
template <typename... Fs>
struct MAKESHIFT_DETAIL_EMPTY_BASES overload : Fs...
{
    using Fs::operator ()...;
};
template <typename... Ts>
overload(Ts...) -> overload<Ts...>;


    //
    // Higher-order function for defining recursive lambda functions.
    //ᅟ
    // Note that the lambda function must explicitly declare a return type.
    //ᅟ
    //ᅟ    auto fac = y_combinator( 
    //ᅟ        [](auto fac, int i) -> int
    //ᅟ        {
    //ᅟ            return i <= 1 ? 1 : i * fac(i - 1);
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
    constexpr decltype(auto)
    operator()(ArgsT&&... args)
    {
        return func_(detail::y_combinator_func_ref<F&>{ func_ }, std::forward<ArgsT>(args)...);
    }
    template <typename... ArgsT>
    constexpr decltype(auto)
    operator()(ArgsT&&... args) const
    {
        return func_(detail::y_combinator_func_ref<F const&>{ func_ }, std::forward<ArgsT>(args)...);
    }
};
template <typename F>
y_combinator(F) -> y_combinator<F>;

    //
    // Higher-order function for defining recursive lambda functions.
    //ᅟ
    // Note that the lambda function must explicitly declare a return type.
    //ᅟ
    //ᅟ    auto fac = make_y_combinator( 
    //ᅟ        [](auto fac, int i) -> int
    //ᅟ        {
    //ᅟ            return i <= 1 ? 1 : i * fac(i - 1);
    //ᅟ        });
    //ᅟ    int i = fac(4); // returns 24
    //
template <typename F>
[[nodiscard]] constexpr y_combinator<F>
make_y_combinator(F func)
{
    return y_combinator<F>{ std::move(func) };
}


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_FUNCTIONAL_HPP_
