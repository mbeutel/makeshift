
#ifndef INCLUDED_MAKESHIFT_DETAIL_CONSTEXPR_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_CONSTEXPR_HPP_


#include <utility>     // for forward<>()
#include <type_traits> // for is_empty<>, is_default_constructible<>


namespace makeshift
{

namespace detail
{


    // Workaround for non-default-constructible lambdas in C++17.
    // Does not rely on UB (as far as I can tell). Works with GCC 8.2, Clang 7.0, MSVC 19.20, and ICC 19.0 (also `constexpr` evaluation).
    // Idea taken from http://pfultz2.com/blog/2014/09/02/static-lambda/ and modified to avoid casts.
template <typename F>
    struct stateless_lambda
{
    static_assert(std::is_empty<F>::value, "lambda must be empty");

private:
    union
    {
        F obj;
        char dummy;
    };

public:
    constexpr stateless_lambda(void) noexcept : dummy(0) { }
    template <typename... Ts>
        constexpr decltype(auto) operator()(Ts&&... args) const
    {
        return obj(std::forward<Ts>(args)...); // we can legally use `obj` even if it wasn't initialized because it is empty
    }
};

template <typename F, bool IsDefaultConstructible> struct stateless_functor_0_;
template <typename F> struct stateless_functor_0_<F, true> { using type = F; };
template <typename F> struct stateless_functor_0_<F, false> { using type = stateless_lambda<F>; };
template <typename F> using stateless_functor = typename stateless_functor_0_<F, std::is_default_constructible_v<F>>::type;

template <typename F, typename... Rs>
    struct constexpr_transform_functor
{
    constexpr auto operator ()(void) const
    {
        return stateless_functor<F>{ }(stateless_functor<Rs>{ }()...);
    }
};


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_CONSTEXPR_HPP_
