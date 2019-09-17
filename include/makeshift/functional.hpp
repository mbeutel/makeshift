
#ifndef INCLUDED_MAKESHIFT_FUNCTIONAL_HPP_
#define INCLUDED_MAKESHIFT_FUNCTIONAL_HPP_


#include <utility>     // for move(), forward<>()
#include <cstddef>     // for size_t
#include <functional>  // for hash<>
#include <type_traits> // for decay<>, declval<>()

#include <makeshift/macros.hpp>  // for MAKESHIFT_CXX, MAKESHIFT_NODISCARD, MAKESHIFT_DETAIL_EMPTY_BASES

#include <makeshift/detail/functional.hpp>


namespace makeshift
{


#if MAKESHIFT_CXX >= 17
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
    struct MAKESHIFT_DETAIL_EMPTY_BASES overload : Fs...
{
    constexpr overload(Fs... fs) : Fs(std::move(fs))... { }
    using Fs::operator ()...; // requires C++17
};
template <typename... Ts>
    overload(Ts&&...) -> overload<std::decay_t<Ts>...>;
#endif // MAKESHIFT_CXX >= 17


    //ᅟ
    // Higher-order function for defining recursive lambda functions.
    // Note that the lambda function must explicitly declare a return type.
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

public:
    constexpr explicit y_combinator(F&& _func)
        : func_(std::forward<F>(_func))
    {
    }

    template <typename... ArgsT>
        constexpr decltype(auto) operator()(ArgsT&&... args)
    {
        return func_(makeshift::detail::y_combinator_func_ref<F&>{ func_ }, std::forward<ArgsT>(args)...);
    }
    template <typename... ArgsT>
        constexpr decltype(auto) operator()(ArgsT&&... args) const
    {
        return func_(makeshift::detail::y_combinator_func_ref<F const&>{ func_ }, std::forward<ArgsT>(args)...);
    }
};
#if MAKESHIFT_CXX >= 17
template <typename F>
    y_combinator(F&& func) -> y_combinator<std::decay_t<F>>;
#endif // MAKESHIFT_CXX >= 17


    //ᅟ
    // Similar to `std::hash<>` but permits omitting the type argument and supports conditional specialization with `enable_if_t<>` or `void_t<>`.
    //
template <typename KeyT = void, typename = void>
    struct hash : std::hash<KeyT> // TODO: do we still need/want this?
{
    using std::hash<KeyT>::hash;
};
template <>
    struct hash<void>
{
        //ᅟ
        // Computes a hash value of the given argument.
        //
    template <typename T>
        MAKESHIFT_NODISCARD constexpr decltype(hash<T>{ }(std::declval<T const&>())) // SFINAE
        operator ()(T const& arg) const noexcept
    {
        return hash<T>{ }(arg);
    }
};


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_FUNCTIONAL_HPP_
