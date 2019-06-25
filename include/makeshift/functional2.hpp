
#ifndef INCLUDED_MAKESHIFT_FUNCTIONAL2_HPP_
#define INCLUDED_MAKESHIFT_FUNCTIONAL2_HPP_


#include <utility>     // for move()
#include <functional>  // for hash<>
#include <type_traits> // for decay<>

#include <makeshift/version.hpp>  // for MAKESHIFT_NODISCARD, MAKESHIFT_EMPTY_BASES


namespace makeshift
{


    //ᅟ
    // Constructs a functor wrapper that selects the matching overload among a number of given functors.
    //ᅟ
    //ᅟ    auto type_name_func = overload(
    //ᅟ        [](int)   { return "int"; },
    //ᅟ        [](float) { return "float"; },
    //ᅟ        [](auto)  { return "unknown"; }
    //ᅟ    );
    //
template <typename... Fs>
    struct MAKESHIFT_EMPTY_BASES overload2 : Fs...
{
    constexpr overload2(Fs... fs) : Fs(std::move(fs))... { }
    using Fs::operator ()...;
};
template <typename... Ts>
    overload2(Ts&&...) -> overload2<std::decay_t<Ts>...>;


    //ᅟ
    // Higher-order function for defining recursive lambda functions.
    //ᅟ
    //ᅟ    auto fac = y_combinator{ 
    //ᅟ        [](auto fac, int i) -> int
    //ᅟ        {
    //ᅟ            return i <= 1
    //ᅟ                ? 1
    //ᅟ                : i * fac(i - 1);
    //ᅟ        }
    //ᅟ    };
    //ᅟ    int i = fac(4); // returns 24
    //
template <typename F>
    class y_combinator
{
private:
    F func_;

    class func_ref
    {
    private:
        F& func_;

    public:
        constexpr explicit func_ref(F& _func) noexcept
            : func_(_func)
        {
        }
        template <typename... ArgsT>
            constexpr decltype(auto) operator()(ArgsT&&... args) const
        {
            return func_(*this, std::forward<ArgsT>(args)...);
        }
    };

    class func_const_ref
    {
    private:
        const F& func_;

    public:
        constexpr explicit func_const_ref(F& _func) noexcept
            : func_(_func)
        {
        }
        template <typename... ArgsT>
            constexpr decltype(auto) operator()(ArgsT&&... args) const
        {
            return func_(*this, std::forward<ArgsT>(args)...);
        }
    };

public:
    constexpr explicit y_combinator(F&& _func)
        : func_(std::forward<F>(_func))
    {
    }

    template <typename... ArgsT>
        constexpr decltype(auto) operator()(ArgsT&&... args)
    {
        return func_(func_ref{ func_ }, std::forward<ArgsT>(args)...);
    }
    template <typename... ArgsT>
        constexpr decltype(auto) operator()(ArgsT&&... args) const
    {
        return func_(func_const_ref{ func_ }, std::forward<ArgsT>(args)...);
    }
};
template <typename F>
    y_combinator(F&& func) -> y_combinator<std::decay_t<F>>;


    //ᅟ
    // Similar to `std::hash<>` but permits omitting the type argument and conditional specialization with `enable_if<>`.
    //
template <typename KeyT = void, typename = void>
    struct hash2 : std::hash<KeyT>
{
    using std::hash<KeyT>::hash;
};
template <>
    struct hash2<void>
{
        //ᅟ
        // Computes a hash value of the given argument.
        //
    template <typename T,
              typename = decltype(hash2<T>{ }(std::declval<const T&>()))>
        MAKESHIFT_NODISCARD constexpr std::size_t operator ()(const T& arg) const noexcept
    {
        return hash2<T>{ }(arg);
    }
};


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_FUNCTIONAL2_HPP_
