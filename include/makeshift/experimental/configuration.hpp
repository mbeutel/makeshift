
#ifndef MAKESHIFT_CONFIGURATION_HPP_
#define MAKESHIFT_CONFIGURATION_HPP_


#include <functional>
#include <type_traits>

#include <makeshift/type_traits.hpp>


namespace makeshift
{

namespace detail
{

#pragma region Leaf classes

template <typename T>
    class config_value
{
private:
    T value_;
public:
    constexpr config_value(T _value) : value_(std::move(_value)) { }
};

template <typename SigT>
    class config_map;
template <typename R, typename... ArgsT>
    class config_map<R(ArgsT...)>
{
private:
    std::function<R(ArgsT...)> func_;
public:
    config_map(std::function<R(ArgsT...)> _func) : func_(std::move(_func)) { }
};


struct open_map_base { };

template <typename SigT, template <typename NewArgsT> class RebinderT, typename FuncT>
    class open_map : public open_map_base
{
private:
    FuncT func_;
public:
    constexpr open_map(FuncT _func) : func_(std::move(_func)) { }
};

#pragma endregion Leaf classes

#pragma region Configuration mappings

template <typename... CfgMapT>
    struct Match
{
};

#pragma endregion Configuration mappings

template <typename OpenMapT, typename NewArgsT>
    struct rebind_;
template <typename SigT, template <typename NewArgsT> class RebinderT, typename FuncT, typename NewArgsT>
    struct rebind_<open_map<SigT, RebinderT, FuncT>, NewArgsT> : RebinderT<NewArgsT>
{
};
template <typename OpenMapT, typename NewArgsT>
    using rebind = typename rebind_<OpenMapT, NewArgsT>::type;


    // RebinderT must support `using NewR = RebinderT<R(NewArgsT)>::type`
/*template <typename OpenMapT>
    class config_typed_map
{
private:
    OpenMapT map_;

public:
    constexpr config_typed_map(FuncT _func) : func_(std::move(_func)) { }
};*/


template <typename T> using can_call_r = decltype(&T::operator ());
template <typename T> using can_call = can_instantiate<can_call_r, T>;
template <typename T> constexpr bool can_call_v = can_call<T>::value;

template <typename F> struct call_sig_0_;
template <typename R, typename... ArgsT> struct call_sig_0_<R(ArgsT...)> { using type = R(ArgsT...); };
template <typename R, typename C, typename... ArgsT> struct call_sig_0_<R (C::*)(ArgsT...)> : call_sig_0_<R(ArgsT...)> { };
template <typename R, typename C, typename... ArgsT> struct call_sig_0_<R (C::*)(ArgsT...) const> : call_sig_0_<R(ArgsT...)> { };
template <typename F> using call_sig_t = typename call_sig_0_<decltype(&F::operator ())>::type;

template <typename T> struct config_decay_map_;
template <typename R, typename... ArgsT> struct config_decay_map_<R(ArgsT...)> { using type = R(ArgsT...); using value_type = std::function<R(ArgsT...)>; using leaf = config_map<R(ArgsT...)>; };

template <typename T> struct config_decay_value_ { using type = T; using value_type = T; using leaf = config_value<type>; };

template <bool CanCall, typename T> struct config_decay_0_;
template <typename T> struct config_decay_0_<false, T> : config_decay_value_<T> { };
template <typename T> struct config_decay_0_<true, T> : config_decay_map_<call_sig_t<T>> { };

template <typename T> struct config_decay_ : config_decay_0_<can_call_v<std::decay_t<T>>, std::decay_t<T>> { };
template <typename R, typename... ArgsT> class config_decay_<R (*)(ArgsT...)> : config_decay_map_<R(ArgsT)> { };

template <typename T> using config_type = typename config_decay_<T>::type;
template <typename T> using config_value_type = typename config_decay_<T>::value_type;
template <typename T> using config_leaf = typename config_decay_<T>::leaf;

} // namespace detail

inline namespace types
{

template <typename... Ts>
    class configuration : makeshift::detail::config_leaf<Ts>...
{
    constexpr configuration(config_value_type<Ts>... args)
        : makeshift::detail::config_leaf<Ts>(std::move(args))...
    {
    }
};
template <typename... Ts>
    configuration(Ts&&...) -> configuration<makeshift::detail::config_type<Ts>...>;

} // inline namespace types

} // namespace makeshift

#endif // MAKESHIFT_CONFIGURATION_HPP_


/*
What to handle?
lhs:
- optional<>: nullopt if not provided
- named<>: match by name, not by type
=> named<optional<>>, but not optional<named<>>!
rhs:
- variant<>: matches if *all* alternatives match
- nested configuration<>: flatten
- 

kwargs!!

cfg[tag_v<T>]
cfg[name<"foo"_kw>]
retrieve(cfg)
*/
