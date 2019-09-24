
#ifndef INCLUDED_MAKESHIFT_DETAIL_VARIANT_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_VARIANT_HPP_


#include <array>
#include <cstddef>   // for size_t, ptrdiff_t
#include <algorithm> // for 
#include <type_traits> // for integral_constant<>, underlying_type<>, declval<>()

#include <makeshift/utility.hpp>  // for type_seq_<>
#include <makeshift/constval.hpp> // for array_constant<>


namespace makeshift
{

namespace detail
{


template <template <typename...> class VariantT, typename ValuesC>
    struct constval_variant_map;
template <template <typename...> class VariantT, typename T, T... Vs>
    struct constval_variant_map<VariantT, array_constant<T, Vs...>>
{
    using type = VariantT<typename constant_<T, Vs>::type...>; // workaround for VC++ bug, cf. https://developercommunity.visualstudio.com/content/problem/719235/erroneous-c2971-caused-by-using-variadic-by-ref-no.html
    static constexpr type values[] = {
        type(typename constant_<T, Vs>::type{ })...
    };
};
template <template <typename...> class VariantT, typename T, T... Vs>
    constexpr typename constval_variant_map<VariantT, array_constant<T, Vs...>>::type constval_variant_map<VariantT, array_constant<T, Vs...>>::values[];

template <typename T> using can_order_r = decltype(std::declval<T>() < std::declval<T>());
template <typename T> struct can_order : can_instantiate<can_order_r, T> { };

template <typename T, std::size_t N>
    constexpr bool are_values_sorted_0(std::true_type /* canOrder */, std::array<T, N> const& values)
{
        // `std::is_sorted()` is not `constexpr`, so we have to reimplement it here
    for (std::size_t i = 1; i < N; ++i)
    {
        if (values[i] < values[i - 1]) return false;
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
    std::ptrdiff_t search_value_index_2(std::true_type /*sorted*/, T const& value, std::array<T, N> const& values)
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
    std::ptrdiff_t search_value_index_2(std::false_type /*sorted*/, T const& value, std::array<T, N> const& values)
{
        // linear search
    for (std::ptrdiff_t i = 0; i < std::ptrdiff_t(N); ++i)
    {
        if (values[i] == value) return i;
    }
    return -1;
}

template <typename RepT, typename T, typename ValuesC>
    std::ptrdiff_t search_value_index_1(std::true_type /*bool||enum||integral && contiguous*/, T const& value, ValuesC valuesC)
{
    constexpr auto const& values = valuesC.value;
    constexpr auto r0 = static_cast<RepT>(values[0]);
    auto r = static_cast<RepT>(value);
    if (r < r0 || r - r0 >= values.size()) return -1;
    return r - r0;
}
template <typename RepT, typename T, typename ValuesC>
    std::ptrdiff_t search_value_index_1(std::false_type /*bool||enum||integral && contiguous*/, T const& value, ValuesC valuesC)
{
    constexpr bool sorted = are_values_sorted(valuesC.value); // TODO: perhaps we can just sort values for the index lookup
    return search_value_index_2(std::integral_constant<bool, sorted>{ }, value, valuesC.value);
}

template <typename RepT, typename T, std::size_t N>
    constexpr bool are_values_contiguous(std::array<T, N> const& values)
{
    if (N == 0) return false;
    auto r = static_cast<RepT>(values[0]);
    for (std::size_t i = 1; i < N; ++i)
    {
        auto rNext = static_cast<RepT>(values[i]);
        if (rNext != r + 1) return false;
        r = rNext;
    }
    return true;
}

template <typename RepT, typename T, typename ValuesC>
    std::ptrdiff_t search_value_index_0(std::true_type /*bool||enum||integral*/, T const& value, ValuesC valuesC)
{
    constexpr bool contiguous = are_values_contiguous<RepT>(valuesC.value); // implies that at least one element exists
    return search_value_index_1<RepT>(std::integral_constant<bool, contiguous>{ }, value, valuesC);
}
template <typename T, typename ValuesC>
    std::ptrdiff_t search_value_index_0(std::false_type /*bool||enum||integral*/, T const& value, ValuesC valuesC)
{
    return search_value_index_1<T>(std::false_type{ }, value, valuesC);
}

template <bool IsEnum, typename T> struct has_integral_rep_0_;
template <typename T> struct has_integral_rep_0_<true, T> : std::true_type { using rep = std::underlying_type_t<T>; };
template <typename T> struct has_integral_rep_0_<false, T> : std::false_type { using rep = T; };
template <typename T> struct has_integral_rep_ : has_integral_rep_0_<std::is_enum<T>::value, T> { };
template <> struct has_integral_rep_<bool> : std::true_type { using rep = int; };

template <typename T, typename ValuesC>
    std::ptrdiff_t search_value_index(T const& value, ValuesC valuesC)
{
    return search_value_index_0<typename has_integral_rep_<T>::rep>(has_integral_rep_<T>{ }, value, valuesC);
}


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_VARIANT_HPP_
