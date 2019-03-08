
#ifndef INCLUDED_MAKESHIFT_DETAIL_FUNCTIONAL2_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_FUNCTIONAL2_HPP_


#include <utility>     // for move()
#include <functional>  // for reference_wrapper<>
#include <type_traits> // for enable_if<>, conjunction<>, is_default_constructible<>

#include <makeshift/version.hpp>  // for MAKESHIFT_EMPTY_BASES


namespace makeshift
{

namespace detail
{


template <typename... Ts>
    struct MAKESHIFT_EMPTY_BASES adapter_base : Ts...
{
    template <typename = std::enable_if_t<std::conjunction_v<std::is_default_constructible<Ts>...>>>
        constexpr adapter_base(void)
    {
    }
    constexpr adapter_base(Ts... args) : Ts(std::move(args))... { }
    constexpr adapter_base(const adapter_base&) = default;
    constexpr adapter_base(adapter_base&&) = default;
    constexpr adapter_base& operator =(const adapter_base&) = default;
    constexpr adapter_base& operator =(adapter_base&&) = default;
};


struct hashable_base { };


template <typename T> struct is_reference_wrapper : std::false_type { };
template <typename U> struct is_reference_wrapper<std::reference_wrapper<U>> : std::true_type { };
template <typename T> constexpr bool is_reference_wrapper_v = is_reference_wrapper<T>::value;

//#ifndef MAKESHIFT_CXX17
template <typename F, typename C, typename Arg0T, typename... ArgsT>
    constexpr decltype(auto) invoke_impl_FNC(std::true_type /*isRefWrapper*/, F C::* f, Arg0T&& arg0, ArgsT&&... args) // reference_wrapper<> doesn't currently have constexpr support
{
    return (arg0.get().*f)(std::forward<ArgsT>(args)...);
}
template <typename F, typename C, typename Arg0T, typename... ArgsT>
    constexpr decltype(auto) invoke_impl_FNC(std::false_type /*isRefWrapper*/, F C::* f, Arg0T&& arg0, ArgsT&&... args)
{
    return ((*std::forward<Arg0T>(arg0)).*f)(std::forward<ArgsT>(args)...);
}
template <typename F, typename C, typename Arg0T, typename... ArgsT>
    constexpr decltype(auto) invoke_impl_F(std::true_type /*isClassObj*/, F C::* f, Arg0T&& arg0, ArgsT&&... args)
{
    return (std::forward<Arg0T>(arg0).*f)(std::forward<ArgsT>(args)...);
}
template <typename F, typename C, typename Arg0T, typename... ArgsT>
    constexpr decltype(auto) invoke_impl_F(std::false_type /*isClassObj*/, F C::* f, Arg0T&& arg0, ArgsT&&... args)
{
    return makeshift::detail::invoke_impl_FNC(is_reference_wrapper<std::decay_t<Arg0T>>{ }, f, std::forward<Arg0T>(arg0), std::forward<ArgsT>(args)...);
}
template <typename F, typename C, typename Arg0T, typename... ArgsT>
    constexpr decltype(auto) invoke_impl(std::true_type /*isMemberFunctionPtr*/, F C::* f, Arg0T&& arg0, ArgsT&&... args)
{
    return makeshift::detail::invoke_impl_F(std::is_base_of<C, std::decay_t<Arg0T>>{ }, f, std::forward<Arg0T>(arg0), std::forward<ArgsT>(args)...);
}
template <typename F, typename C, typename ArgT>
    constexpr decltype(auto) invoke_impl_ONC(std::true_type /*isRefWrapper*/, F C::* op, ArgT&& arg) // reference_wrapper<> doesn't currently have constexpr support
{
    return arg.get().*op;
}
template <typename F, typename C, typename ArgT>
    constexpr decltype(auto) invoke_impl_ONC(std::false_type /*isRefWrapper*/, F C::* op, ArgT&& arg)
{
    return (*std::forward<ArgT>(arg)).*op;
}
template <typename F, typename C, typename ArgT>
    constexpr decltype(auto) invoke_impl_O(std::true_type /*isClassObj*/, F C::* op, ArgT&& arg)
{
    return std::forward<ArgT>(arg).*op;
}
template <typename F, typename C, typename ArgT>
    constexpr decltype(auto) invoke_impl_O(std::false_type /*isClassObj*/, F C::* op, ArgT&& arg)
{
    return makeshift::detail::invoke_impl_ONC(is_reference_wrapper<std::decay_t<ArgT>>{ }, op, std::forward<ArgT>(arg));
}
template <typename F, typename C, typename Arg0T, typename... ArgsT>
    constexpr decltype(auto) invoke_impl(std::false_type /*isMemberFunctionPtr*/, F C::* f, Arg0T&& arg0, ArgsT&&... args)
{
    static_assert(std::is_member_object_pointer<F C::*>::value, "argument must be member object pointer");
    static_assert(sizeof...(ArgsT) == 0, "member object pointer cannot be invoked with more than one argument");
    return makeshift::detail::invoke_impl_O(std::is_base_of<C, std::decay_t<Arg0T>>{ }, f, std::forward<Arg0T>(arg0));
}
//#endif // MAKESHIFT_CXX17


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_FUNCTIONAL2_HPP_
