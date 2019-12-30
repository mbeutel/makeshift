
#ifndef INCLUDED_MAKESHIFT_EXPERIMENTAL_DETAIL_VARIANT_HPP_
#define INCLUDED_MAKESHIFT_EXPERIMENTAL_DETAIL_VARIANT_HPP_


#include <utility>   // for forward<>()
#include <exception> // for terminate()


namespace makeshift
{

namespace detail
{


template <template <typename...> class VariantT, typename MonostateT, typename V> struct with_monostate_;
template <template <typename...> class VariantT, typename MonostateT, typename... Vs> struct with_monostate_<VariantT, MonostateT, VariantT<Vs...>> { using type = VariantT<MonostateT, Vs...>; };
template <template <typename...> class VariantT, typename MonostateT, typename V> struct without_monostate_;
template <template <typename...> class VariantT, typename MonostateT, typename... Vs> struct without_monostate_<VariantT, MonostateT, VariantT<MonostateT, Vs...>> { using type = VariantT<Vs...>; };

template <typename MonostateT, typename R>
struct monostate_filtering_visitor
{
    [[noreturn]] R operator ()(MonostateT) const
    {
        std::terminate();
    }
    template <typename T>
    constexpr R operator ()(T&& arg) const
    {
        return std::forward<T>(arg);
    }
};

template <template <typename...> class VariantT, typename... Vs>
struct variant_cat_;
template <template <typename...> class VariantT, typename... Ts>
struct variant_cat_<VariantT, VariantT<Ts...>>
{
    using type = VariantT<Ts...>;
};
template <template <typename...> class VariantT, typename... V0Ts, typename... V1Ts, typename... Vs>
struct variant_cat_<VariantT, VariantT<V0Ts...>, VariantT<V1Ts...>, Vs...>
    : variant_cat_<VariantT, VariantT<V0Ts..., V1Ts...>>
{
};


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_VARIANT_HPP_
