
#ifndef MAKESHIFT_TYPE_TRAITS_HPP_
#define MAKESHIFT_TYPE_TRAITS_HPP_


#include <type_traits> // for integral_constant<>


namespace makeshift
{

namespace detail
{

    // this is an abomination; it is used only to avoid errors about incompatible types when another, more helpful error message will follow.
struct universally_convertible
{
    template <typename T> operator T(void) const;
};

template <typename...> using void_t = void; // ICC doesn't have std::void_t<> yet
template <template <typename...> class, typename, typename...> struct can_apply_1_ : std::false_type { };
template <template <typename...> class Z, typename... Ts> struct can_apply_1_<Z, void_t<Z<Ts...>>, Ts...> : std::true_type { };

} // namespace detail


inline namespace types
{

    // Determines whether the template instantiation Z<Ts...> would be valid. Useful for expression SFINAE.
template <template <typename...> class Z, typename... Ts> using can_apply = makeshift::detail::can_apply_1_<Z, void, Ts...>;
template <template <typename...> class Z, typename... Ts> constexpr bool can_apply_v = can_apply<Z, Ts...>::value;


    // Type sequence (strictly for compile-time purposes).
template <typename... Ts> struct type_sequence { };


    // Helper for type dispatching.
template <typename T = void> struct tag_t { using type = T; };
template <typename T = void> constexpr tag_t<T> tag { };


    // Unity type for tuple functions.
    // TODO: how to bridge to std::optional<>?
struct none_t { };
constexpr none_t none { };

} // inline namespace types

} // namespace makeshift


namespace makeshift
{

namespace detail
{

template <typename T, template <typename...> class U> struct is_same_template_ : std::false_type { };
template <template <typename...> class U, typename... Ts> struct is_same_template_<U<Ts...>, U> : std::true_type { };

template <typename F> using is_functor_r = decltype(&T::operator ());

template <typename F> struct functor_sig_0_;
template <typename R, typename... ArgsT> struct functor_sig_0_<R(ArgsT...)> { using type = R(ArgsT...); };
template <typename R, typename C, typename... ArgsT> struct functor_sig_0_<R (C::*)(ArgsT...)> : functor_sig_0_<R(ArgsT...)> { };
template <typename R, typename C, typename... ArgsT> struct functor_sig_0_<R (C::*)(ArgsT...) const> : functor_sig_0_<R(ArgsT...)> { };

} // namespace detail


inline namespace types
{

    // Determines whether a type is an instantiation of a particular class template.
template <typename T, template <typename...> class U> using is_same_template = makeshift::detail::is_same_template_<T, U>;
template <typename T, template <typename...> class U> constexpr bool is_same_template_v = is_same_template<T, U>::value;


    // Removes an rvalue reference from a type.
template <typename T> struct remove_rvalue_reference { using type = T; };
template <typename T> struct remove_rvalue_reference<T&&> { using type = T; };
template <typename T> using remove_rvalue_reference_t = typename remove_rvalue_reference<T>::type;


    // Determines whether the given type is a functor (i.e. a class with non-ambiguous operator ()).
template <typename F> using is_functor = can_apply<makeshift::detail::is_functor_r, F>;
template <typename F> constexpr bool is_functor_v = is_functor<T>::value;


    // Determines whether the given type is a function pointer.
template <typename F> struct is_function_pointer : std::false_type { };
template <typename F> struct is_function_pointer<F*> : std::is_function<F> { };
template <typename F> constexpr bool is_function_pointer_v = is_function_pointer<T>::value;


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


    // Retrieves the I-th argument type of a function signature.
template <std::size_t I, typename SigT> struct sig_arg_type;
template <std::size_t I, typename R, typename Arg0T, typename... ArgsT> struct sig_arg_type<I, R(Arg0T, ArgsT...)> : sig_arg_type<I - 1, R(ArgsT...)> { };
template <typename R, typename Arg0T, typename... ArgsT> struct sig_arg_type<0, R(Arg0T, ArgsT...)> { using type = Arg0T; };
template <std::size_t I, typename SigT> using sig_arg_type_t = typename sig_arg_type<I, SigT>::type;


    // Calls a callable object. (This is a less general but constexpr version of std::invoke().)
template <typename F, typename... ArgsT>
    constexpr decltype(auto) call(F&& func, ArgsT&&... args)
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
            if constexpr (std::is_base_of<T, std::decay_t<Arg0T>>::value)
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

#endif // MAKESHIFT_TYPE_TRAITS_HPP_
