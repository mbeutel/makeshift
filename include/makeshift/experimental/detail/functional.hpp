
#ifndef INCLUDED_MAKESHIFT_EXPERIMENTAL_DETAIL_FUNCTIONAL_HPP_
#define INCLUDED_MAKESHIFT_EXPERIMENTAL_DETAIL_FUNCTIONAL_HPP_


#include <utility> // for forward<>()

#include <gsl-lite/gsl-lite.hpp> // for void_t<>


namespace makeshift
{


namespace gsl = ::gsl_lite;


namespace detail
{


struct lvalue_capture_forwarder
{
    template <typename T>
    constexpr T& operator ()(T& arg) const noexcept
    {
        return arg;
    }
};
struct rvalue_capture_forwarder
{
    template <typename T>
    constexpr T const& operator ()(T const& arg) const noexcept
    {
        return arg;
    }
    template <typename T>
    constexpr T&& operator ()(T& arg) const noexcept
    {
        return static_cast<T&&>(arg);
    }
};

template <typename F, typename = void>
struct forward_to_impl
{
    explicit constexpr forward_to_impl(F func)
        : F(std::move(func))
    {
    }

    template <typename... Ts>
    constexpr auto operator ()(Ts&&... args) &
    -> decltype(static_cast<F&>(*this)(lvalue_capture_forwarder{ })(std::forward<Ts>(args)...))
    {
        return static_cast<F&>(*this)(lvalue_capture_forwarder{ })(std::forward<Ts>(args)...);
    }
    template <typename... Ts>
    constexpr auto operator ()(Ts&&... args) const &
    -> decltype(static_cast<F const&>(*this)(lvalue_capture_forwarder{ })(std::forward<Ts>(args)...))
    {
        return static_cast<F const&>(*this)(lvalue_capture_forwarder{ })(std::forward<Ts>(args)...);
    }
    template <typename... Ts>
    constexpr auto operator ()(Ts&&... args) &&
    -> decltype(static_cast<F&&>(*this)(rvalue_capture_forwarder{ })(std::forward<Ts>(args)...))
    {
        return static_cast<F&&>(*this)(rvalue_capture_forwarder{ })(std::forward<Ts>(args)...);
    }
};
template <typename F>
struct forward_to_impl<F, gsl::void_t<std::declval<F>()()>> : F
{
    explicit constexpr forward_to_impl(F func)
        : F(std::move(func))
    {
    }

    template <typename... Ts>
    constexpr auto operator ()(Ts&&... args) &
    -> decltype(static_cast<F&>(*this)()(std::forward<Ts>(args)...))
    {
        return static_cast<F&>(*this)()(std::forward<Ts>(args)...);
    }
    template <typename... Ts>
    constexpr auto operator ()(Ts&&... args) const &
    -> decltype(static_cast<F const&>(*this)()(std::forward<Ts>(args)...))
    {
        return static_cast<F const&>(*this)()(std::forward<Ts>(args)...);
    }
    template <typename... Ts>
    constexpr auto operator ()(Ts&&... args) &&
    -> decltype(static_cast<F&&>(*this)()(std::forward<Ts>(args)...))
    {
        return static_cast<F&&>(*this)()(std::forward<Ts>(args)...);
    }
};


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_EXPERIMENTAL_DETAIL_FUNCTIONAL_HPP_
