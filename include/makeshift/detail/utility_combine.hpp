
#ifndef MAKESHIFT_DETAIL_UTILITY_COMBINE_HPP_
#define MAKESHIFT_DETAIL_UTILITY_COMBINE_HPP_


#include <utility>     // for forward<>()
#include <type_traits> // for decay<>


namespace makeshift
{

inline namespace types
{


    // A type that publicly inherits from all argument types and initializes its base classes with `std::forward<>()`.
template <typename... Ts>
    struct combination_t : Ts...
{
    template <typename... LTs> constexpr combination_t(LTs&&... args) : Ts(std::forward<LTs>(args))... { }
};
template <typename... Ts>
    constexpr combination_t<std::decay_t<Ts>...> combine(Ts&&... args)
{
    return { std::forward<Ts>(args)... };
}


} // inline namespace types

} // namespace makeshift


#endif // MAKESHIFT_DETAIL_UTILITY_COMBINE_HPP_
