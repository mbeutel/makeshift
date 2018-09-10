
#ifndef INCLUDED_MAKESHIFT_DETAIL_UTILITY_COMBINE_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_UTILITY_COMBINE_HPP_


#include <utility>     // for forward<>()
#include <type_traits> // for decay<>


namespace makeshift
{

inline namespace types
{


    //ᅟ
    // A type that publicly inherits from all argument types and initializes its base classes with `std::forward<>()`.
    //
template <typename... Ts>
    struct combination : Ts...
{
    template <typename... LTs,
              typename = std::enable_if_t<sizeof...(LTs) == sizeof...(Ts)>>
        constexpr combination(LTs&&... args)
            : Ts(std::forward<LTs>(args))...
    {
    }
};

    //ᅟ
    // Returns an instance of a type that publicly inherits from all argument types and initializes its base classes with `std::forward<Ts>(args)`.
    //
template <typename Ts>
    constexpr std::decay_t<Ts> combine(Ts&& arg)
{
    return std::forward<Ts>(arg);
}

    //ᅟ
    // Returns an instance of a type that publicly inherits from all argument types and initializes its base classes with `std::forward<Ts>(args)`.
    //
template <typename... Ts>
    constexpr combination<std::decay_t<Ts>...> combine(Ts&&... args)
{
    return { std::forward<Ts>(args)... };
}


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_UTILITY_COMBINE_HPP_
