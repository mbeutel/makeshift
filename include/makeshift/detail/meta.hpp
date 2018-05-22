
#ifndef MAKESHIFT_DETAIL_META_HPP_
#define MAKESHIFT_DETAIL_META_HPP_


#include <type_traits>


namespace makeshift
{

namespace detail
{

template <typename...> using void_t = void; // ICC doesn't have std::void_t<> yet
template <template <typename...> class, typename, typename...> struct can_apply_1_ : std::false_type { };
template <template <typename...> class Z, typename... Ts> struct can_apply_1_<Z, void_t<Z<Ts...>>, Ts...> : std::true_type { };
template <template <typename...> class Z, typename... Ts> using can_apply_t = can_apply_1_<Z, void, Ts...>;
template <template <typename...> class Z, typename... Ts> constexpr bool can_apply = can_apply_t<Z, Ts...>::value;

} // namespace detail

} // namespace makeshift


#endif // MAKESHIFT_DETAIL_META_HPP_
