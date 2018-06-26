
#ifndef MAKESHIFT_DETAIL_FUNCTIONAL_OVERLOAD_HPP_
#define MAKESHIFT_DETAIL_FUNCTIONAL_OVERLOAD_HPP_


#include <type_traits> // for declval<>(), decay<>, reference_wrapper<>, is_same<>
#include <utility>     // for forward<>()
#include <cstddef>     // for size_t


namespace makeshift
{

namespace detail
{


struct default_overload_tag { };

template <typename... Fs>
    struct overload_base : Fs...
{
    constexpr overload_base(Fs&&... fs) : Fs(std::move(fs))... { }
    using Fs::operator ()...;
    template <typename T>
        constexpr decltype(auto) operator()(std::reference_wrapper<T> arg)
    {
        return (*this)(arg.get());
    }
    template <typename T>
        constexpr decltype(auto) operator()(std::reference_wrapper<T> arg) const
    {
        return (*this)(arg.get());
    }
};

template <typename F>
    struct default_overload_wrapper : F
{
    constexpr default_overload_wrapper(F&& func)
        : F(std::move(func))
    {
    }
    template <typename... Ts>
#ifdef MAKESHIFT_FANCY_DEFAULT
        constexpr decltype(auto) operator ()(default_overload_tag, Ts&&... args) const
#else // MAKESHIFT_FANCY_DEFAULT
        constexpr decltype(auto) operator ()(Ts&&... args) const
#endif // MAKESHIFT_FANCY_DEFAULT
        //noexcept(noexcept(F::operator ()(std::forward<Ts>(args)...)))
    {
        return F::operator ()(std::forward<Ts>(args)...);
    }
};
struct ignore_overload_wrapper
{
    template <typename... Ts>
#ifdef MAKESHIFT_FANCY_DEFAULT
        constexpr void operator ()(default_overload_tag, Ts&&...) const noexcept
#else // MAKESHIFT_FANCY_DEFAULT
        constexpr void operator ()(Ts&&...) const noexcept
#endif // MAKESHIFT_FANCY_DEFAULT
    {
    }
};

template <typename F, template <typename...> class T>
    struct match_template_func : F
{
    constexpr match_template_func(F&& func)
        : F(std::move(func))
    {
    }
    template <typename... Ts>
        constexpr decltype(auto) operator ()(const T<Ts...>& arg)
    {
        return F::operator ()(arg);
    }
    template <typename... Ts>
        constexpr decltype(auto) operator ()(const T<Ts...>& arg) const
    {
        return F::operator ()(arg);
    }
    template <typename... Ts>
        constexpr decltype(auto) operator ()(T<Ts...>& arg)
    {
        return F::operator ()(arg);
    }
    template <typename... Ts>
        constexpr decltype(auto) operator ()(T<Ts...>& arg) const
    {
        return F::operator ()(arg);
    }
    template <typename... Ts>
        constexpr decltype(auto) operator ()(T<Ts...>&& arg)
    {
        return F::operator ()(std::move(arg));
    }
    template <typename... Ts>
        constexpr decltype(auto) operator ()(T<Ts...>&& arg) const
    {
        return F::operator ()(std::move(arg));
    }
};

} // namespace detail


inline namespace types
{


    // Defines a functor that accepts all arguments and does nothing.
struct ignore_t
{
    template <typename... Ts>
        void operator ()(Ts&&...) const noexcept
    {
    }
};

    // Functor that accepts all arguments and does nothing.
constexpr inline ignore_t ignore = { };


    // Defines a functor that will be called if no other overloaded functors match.
template <typename F>
    constexpr makeshift::detail::default_overload_wrapper<std::decay_t<F>> otherwise(F&& func)
    noexcept(noexcept(F(std::forward<F>(func))))
{
    return { std::forward<F>(func) };
}

    // Defines a functor that will be called if no other overloaded functors match, and which accepts all arguments and does nothing.
constexpr inline makeshift::detail::ignore_overload_wrapper otherwise(ignore_t) noexcept
{
    return { };
}


    // Returns a functor wrapper that selects the matching overload among a number of given functors.
    //ᅟ
    //ᅟ    auto type_name_func = overload(
    //ᅟ        [](int) { return "int"; },
    //ᅟ        [](float) { return "float"; },
    //ᅟ        otherwise([](auto) { return "unknown"; })
    //ᅟ    );
    //
template <typename... Fs>
    struct overload : makeshift::detail::overload_base<Fs...>
{
    using base = makeshift::detail::overload_base<Fs...>;
    using base::base;

#ifdef MAKESHIFT_FANCY_DEFAULT
private:
    struct test : base
    {
        using base::operator ();
        makeshift::detail::default_overload_tag operator ()(...) const;
    };

public:
    template <typename... Ts>
        constexpr decltype(auto) operator()(Ts&&... args)
    {
        using ResultType = decltype(std::declval<test>()(std::forward<Ts>(args)...));
        constexpr bool isDefaultOverload = std::is_same<ResultType, makeshift::detail::default_overload_tag>::value;
        if constexpr (isDefaultOverload)
            return base::operator ()(makeshift::detail::default_overload_tag{ }, std::forward<Ts>(args)...);
        else
            return base::operator ()(std::forward<Ts>(args)...);
    }
    template <typename... Ts>
        constexpr decltype(auto) operator()(Ts&&... args) const
    {
        using ResultType = decltype(std::declval<const test>()(std::forward<Ts>(args)...));
        constexpr bool isDefaultOverload = std::is_same<ResultType, makeshift::detail::default_overload_tag>::value;
        if constexpr (isDefaultOverload)
            return base::operator ()(makeshift::detail::default_overload_tag{ }, std::forward<Ts>(args)...);
        else
            return base::operator ()(std::forward<Ts>(args)...);
    }
#else // MAKESHIFT_FANCY_DEFAULT
    using base::operator ();
#endif // MAKESHIFT_FANCY_DEFAULT
};
template <typename... Ts>
    overload(Ts&&...) -> overload<std::decay_t<Ts>...>;


    // Returns a functor wrapper that only matches arguments of the given template type.
    //ᅟ
    //ᅟ    auto vec_size_func = match_template<std::vector>([](const auto& v) { return v.size(); });
    //
template <template <typename...> class T, typename F>
    constexpr makeshift::detail::match_template_func<std::decay_t<F>, T> match_template(F&& func)
{
    return { std::forward<F>(func) };
}


} // inline namespace types

} // namespace makeshift


#endif // MAKESHIFT_DETAIL_FUNCTIONAL_OVERLOAD_HPP_
