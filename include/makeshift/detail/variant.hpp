
#ifndef INCLUDED_MAKESHIFT_DETAIL_VARIANT_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_VARIANT_HPP_


#include <array>
#include <cstddef> // for ptrdiff_t
#include <variant>
#include <algorithm>

#include <makeshift/constval.hpp> // for array_constant<>


namespace makeshift
{

namespace detail
{


template <typename ValuesC>
    struct constval_variant_map;
template <typename T, T... Vs>
    struct constval_variant_map<array_constant<T, Vs...>>
{
    using type = std::variant<typename constant_<T, Vs>::type...>; // workaround for VC++ bug, cf. https://developercommunity.visualstudio.com/content/problem/719235/erroneous-c2971-caused-by-using-variadic-by-ref-no.html
    static constexpr type values[] = {
        type(typename constant_<T, Vs>::type{ })...
    };
};
template <typename T, T... Vs>
    constexpr typename constval_variant_map<array_constant<T, Vs...>>::type constval_variant_map<array_constant<T, Vs...>>::values[];

template <typename T> using can_order_r = decltype(std::declval<T>() < std::declval<T>());
template <typename T> struct can_order : can_instantiate<can_order_r, T> { };

template <typename T, std::size_t N>
    constexpr bool are_values_sorted_0(std::true_type /* canOrder */, std::array<T, N> const& values)
{
        // `std::is_sorted()` is not `constexpr`, so we have to reimplement it here
    for (std::size_t i = 1; i < N; ++i)
    {
        if (values[i - 1] > values[i]) return false;
    }
    return true;
}
template <typename T, std::size_t N>
    constexpr bool are_values_sorted_0(std::false_type /* canOrder */, std::array<T, N> const&)
{
    return false;
}
template <typename T, std::size_t N>
    constexpr bool are_values_sorted(std::array<T, N> const& values)
{
    return are_values_sorted_0(can_order<T>{ }, values);
}

template <typename T, std::size_t N>
    std::ptrdiff_t search_value_index_0(std::true_type /*sorted*/, T const& value, std::array<T, N> const& values)
{
        // binary search
    std::ptrdiff_t first = 0,
                   last = N;
    while (last - first > 1)
    {
        std::ptrdiff_t half = first + (last - first) / 2;
        if (values[half] < value)
        {
            first = half;
        }
        else if (value < values[half])
        {
            last = half;
        }
        else
        {
            last = half + 1;
        }
    }
    if (last - first == 1)
    {
        return values[first] == value ? first : -1;
    }
    return -1;
}
template <typename T, std::size_t N>
    std::ptrdiff_t search_value_index_0(std::false_type /*sorted*/, T const& value, std::array<T, N> const& values)
{
        // linear search
    for (std::ptrdiff_t i = 0; i < std::ptrdiff_t(N); ++i)
    {
        if (values[i] == value) return i;
    }
    return -1;
}

template <typename T, typename ValuesC>
    std::ptrdiff_t search_value_index(T const& value, ValuesC)
{
    constexpr auto const& values = ValuesC::value;
    constexpr bool isSorted = are_values_sorted(values); // TODO: perhaps we can just sort values for the index lookup
    return search_value_index(std::integral_constant<bool, isSorted>{ }, value, values);
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_VARIANT_HPP_
