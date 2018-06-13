
#ifndef MAKESHIFT_FUNCTIONAL_HPP_
#define MAKESHIFT_FUNCTIONAL_HPP_


#include <utility>     // for forward<>()
#include <type_traits> // for is_function<>, is_member_function_pointer<>, is_member_object_pointer<>, integral_constant<>

#include <makeshift/type_traits.hpp> // for can_apply<>


namespace makeshift
{

namespace detail
{


template <typename F> using is_functor_r = decltype(&F::operator ());

template <typename F> struct functor_sig_0_;
template <typename R, typename... ArgsT> struct functor_sig_0_<R(ArgsT...)> { using type = R(ArgsT...); };
template <typename R, typename C, typename... ArgsT> struct functor_sig_0_<R (C::*)(ArgsT...)> : functor_sig_0_<R(ArgsT...)> { };
template <typename R, typename C, typename... ArgsT> struct functor_sig_0_<R (C::*)(ArgsT...) const> : functor_sig_0_<R(ArgsT...)> { };


} // namespace detail


inline namespace types
{


    // Determines whether the given type is a functor (i.e. a class with non-ambiguous `operator ()`).
template <typename F> using is_functor = can_apply<makeshift::detail::is_functor_r, F>;
template <typename F> constexpr bool is_functor_v = is_functor<F>::value;


    // Determines whether the given type is a function pointer.
template <typename F> struct is_function_pointer : std::false_type { };
template <typename F> struct is_function_pointer<F*> : std::is_function<F> { };
template <typename F> constexpr bool is_function_pointer_v = is_function_pointer<F>::value;


    // Retrieves the signature of a callable object.
template <typename F> struct callable_sig : makeshift::detail::functor_sig_0_<decltype(&F::operator ())> { };
template <typename R, typename... ArgsT> struct callable_sig<R (*)(ArgsT...)> { using type = R(ArgsT...); };
template <typename R, typename... ArgsT> struct callable_sig<R (*)(ArgsT...) noexcept> { using type = R(ArgsT...); };
template <typename R, typename C, typename... ArgsT> struct callable_sig<R (C::*)(ArgsT...)> { using type = R(C&, ArgsT...); };
template <typename R, typename C, typename... ArgsT> struct callable_sig<R (C::*)(ArgsT...) noexcept> { using type = R(C&, ArgsT...); };
template <typename R, typename C, typename... ArgsT> struct callable_sig<R (C::*)(ArgsT...) const> { using type = R(const C&, ArgsT...); };
template <typename R, typename C, typename... ArgsT> struct callable_sig<R (C::*)(ArgsT...) const noexcept> { using type = R(const C&, ArgsT...); };
template <typename F> using callable_sig_t = typename callable_sig<F>::type;


    // Determines whether the object is callable.
template <typename F> struct is_callable : std::integral_constant<bool, is_functor_v<F> || is_function_pointer_v<F> || std::is_member_function_pointer<F>::value> { };
template <typename F> constexpr bool is_callable_v = is_callable<F>::value;


    // Retrieves the return type of a function signature.
template <typename SigT> struct sig_return_type;
template <typename R, typename... ArgsT> struct sig_return_type<R(ArgsT...)> { using type = R; };
template <typename SigT> using sig_return_type_t = typename sig_return_type<SigT>::type;


    // Retrieves the `I`-th argument type of a function signature.
template <std::size_t I, typename SigT> struct sig_arg_type;
template <std::size_t I, typename R, typename Arg0T, typename... ArgsT> struct sig_arg_type<I, R(Arg0T, ArgsT...)> : sig_arg_type<I - 1, R(ArgsT...)> { };
template <typename R, typename Arg0T, typename... ArgsT> struct sig_arg_type<0, R(Arg0T, ArgsT...)> { using type = Arg0T; };
template <std::size_t I, typename SigT> using sig_arg_type_t = typename sig_arg_type<I, SigT>::type;


    // Calls a callable object. (This is a less general but `constexpr` version of `std::invoke()`.)
template <typename F, typename... ArgsT>
    constexpr decltype(auto) call(F&& f, ArgsT&&... args)
{
    return std::forward<F>(f)(std::forward<ArgsT>(args)...);
}
template <typename F, typename C, typename Arg0T, typename... ArgsT>
    constexpr decltype(auto) call(F C::* f, Arg0T&& arg0, ArgsT&&... args)
{
    if constexpr (std::is_member_function_pointer<F C::*>::value)
    {
        if constexpr (std::is_base_of<C, std::decay_t<Arg0T>>::value)
            return (std::forward<Arg0T>(arg0).*f)(std::forward<ArgsT>(args)...);
        //else if constexpr (std::is_reference_wrapper<std::decay_t<Arg0T>>::value)
        //    return (arg0.get().*f)(std::forward<ArgsT>(args)...);
        else
            return ((*std::forward<Arg0T>(arg0)).*f)(std::forward<ArgsT>(args)...);
    }
    else
    {
        static_assert(std::is_member_object_pointer<F C::*>::value);
        static_assert(sizeof...(ArgsT) == 0);
        //static_assert(sizeof...(ArgsT) <= 1);
        //if constexpr (sizeof...(ArgsT) == 1) // setter invocation
        //    call(f, std::forward<Arg0T>(arg0)) = (std::forward<ArgsT>(args), ...));
        //else // getter invocation
        //{
            if constexpr (std::is_base_of<C, std::decay_t<Arg0T>>::value)
                return std::forward<Arg0T>(arg0).*f;
            //else if constexpr (std::is_reference_wrapper<std::decay_t<Arg0T>>::value)
            //    return arg0.get().*f;
            else
                return (*std::forward<Arg0T>(arg0)).*f;
        //}
    }
}


} // inline namespace types

} // namespace makeshift


#endif // MAKESHIFT_FUNCTIONAL_HPP_
