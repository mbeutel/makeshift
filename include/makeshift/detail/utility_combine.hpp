
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
    struct combine : Ts...
{
    template <typename... LTs> constexpr combine(LTs&&... args) : Ts(std::forward<LTs>(args))... { }
};
template <typename... Ts>
    combine(Ts&&...) -> combine<std::decay_t<Ts>...>;


} // inline namespace types

} // namespace makeshift


#endif // MAKESHIFT_DETAIL_UTILITY_COMBINE_HPP_
