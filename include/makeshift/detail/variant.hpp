
#ifndef INCLUDED_MAKESHIFT_DETAIL_VARIANT_HPP_
#define INCLUDED_MAKESHIFT_DETAIL_VARIANT_HPP_


#include <array>
#include <cstddef>     // for size_t, ptrdiff_t
#include <utility>     // for integer_sequence<>
#include <exception>
#include <type_traits> // for integral_constant<>, underlying_type<>, declval<>(), remove_const<>, remove_reference<>

#include <gsl-lite/gsl-lite.hpp> // for gsl_CPP17_OR_GREATER

#include <makeshift/utility.hpp>  // for type_seq_<>
#include <makeshift/constval.hpp> // for array_constant<>


namespace makeshift {

namespace gsl = ::gsl_lite;


    //
    // Exception thrown by `expand_or_throw()` if the runtime value to be expanded is not among the values listed.
    //
class unsupported_runtime_value : public std::exception
{
public:
    char const* what(void) const noexcept override { return "unsupported runtime value"; }
};


namespace detail {


#if defined(_MSC_VER) && defined(__INTELLISENSE__)
struct convertible_to_anything
{
    template <typename T> constexpr operator T(void) const;
};
#endif // defined(_MSC_VER) && defined(__INTELLISENSE__)

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
            first = half + 1;
        }
        else if (value < values[half])
        {
            last = half;
        }
        else // value == values[half]
        {
            if (half + 1 == last) return half;
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
    if (r < r0 || std::size_t(r - r0) >= values.size()) return -1;
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
    static_assert(N > 0, "overload resolution failed");

    auto r = static_cast<RepT>(values[0]);
    for (std::size_t i = 1; i < N; ++i)
    {
        auto rNext = static_cast<RepT>(values[i]);
        if (rNext != r + 1) return false;
        r = rNext;
    }
    return true;
}
template <typename RepT, typename T>
constexpr bool are_values_contiguous(std::array<T, 0> const& /*values*/)
{
    return false;
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
template <typename T> struct has_integral_rep_0_<false, T> : std::is_integral<T> { using rep = T; };
template <typename T> struct has_integral_rep_ : has_integral_rep_0_<std::is_enum<T>::value, T> { };
template <> struct has_integral_rep_<bool> : std::true_type { using rep = int; };

template <typename T, typename ValuesC>
std::ptrdiff_t search_value_index(T const& value, ValuesC valuesC)
{
    return search_value_index_0<typename has_integral_rep_<T>::rep>(has_integral_rep_<T>{ }, value, valuesC);
}

template <typename... Ts> struct type_seq_;
template <std::size_t I, typename SeqT> struct nth_variant_type_;
template <std::size_t I, template <typename...> class SeqT, typename... Ts> struct nth_variant_type_<I, SeqT<Ts...>> { using type = typename nth_type_<I, Ts...>::type; };
template <std::size_t I, template <typename...> class SeqT, typename... Ts> struct nth_variant_type_<I, SeqT<Ts...>&> { using type = typename nth_type_<I, Ts...>::type&; };
template <std::size_t I, template <typename...> class SeqT, typename... Ts> struct nth_variant_type_<I, SeqT<Ts...> const&> { using type = typename nth_type_<I, Ts...>::type const&; };
template <std::size_t I, template <typename...> class SeqT, typename... Ts> struct nth_variant_type_<I, SeqT<Ts...>&&> { using type = typename nth_type_<I, Ts...>::type&&; };
template <std::size_t I, template <typename...> class SeqT, typename... Ts> struct nth_variant_type_<I, SeqT<Ts...> const&&> { using type = typename nth_type_<I, Ts...>::type const&&; };
template <std::size_t I, typename SeqT> using nth_variant_type = typename nth_variant_type_<I, SeqT>::type;

template <typename SeqT> struct seq_size_;
template <template <typename...> class SeqT, typename... Ts> struct seq_size_<SeqT<Ts...>> : std::integral_constant<std::ptrdiff_t, sizeof...(Ts)> { };

    // Using a signed type here means we get a compile error on overflow.
template <std::ptrdiff_t CurStride, typename StridesT, typename ShapeT> struct compute_strides_0_;
template <std::ptrdiff_t CurStride, std::ptrdiff_t... Strides> struct compute_strides_0_<CurStride, std::integer_sequence<std::ptrdiff_t, Strides...>, std::integer_sequence<std::ptrdiff_t>> { using type = std::integer_sequence<std::ptrdiff_t, Strides...>; };
template <std::ptrdiff_t CurStride, std::ptrdiff_t... Strides, std::ptrdiff_t NextDim, std::ptrdiff_t... Dims>
struct compute_strides_0_<CurStride, std::integer_sequence<std::ptrdiff_t, Strides...>, std::integer_sequence<std::ptrdiff_t, NextDim, Dims...>>
    : compute_strides_0_<CurStride * NextDim, std::integer_sequence<std::ptrdiff_t, Strides..., CurStride>, std::integer_sequence<std::ptrdiff_t, Dims...>> { };
template <typename ShapeT> using compute_strides_ = compute_strides_0_<1, std::integer_sequence<std::ptrdiff_t>, ShapeT>;
template <typename ShapeT> using compute_strides_t = typename compute_strides_<ShapeT>::type;

template <typename ShapeT> struct compute_size_;
#if gsl_CPP17_OR_GREATER
template <std::ptrdiff_t... Is> struct compute_size_<std::integer_sequence<std::ptrdiff_t, Is...>> : std::integral_constant<std::ptrdiff_t, (Is * ... * 1)> { };
#else // gsl_CPP17_OR_GREATER
template <> struct compute_size_<std::integer_sequence<std::ptrdiff_t>> : std::integral_constant<std::ptrdiff_t, 1> { };
template <std::ptrdiff_t I0, std::ptrdiff_t... Is> struct compute_size_<std::integer_sequence<std::ptrdiff_t, I0, Is...>> : std::integral_constant<std::ptrdiff_t, I0 * compute_size_<std::integer_sequence<std::ptrdiff_t, Is...>>::value> { };
#endif // gsl_CPP17_OR_GREATER

template <typename ShapeT, typename StridesT, typename F, typename... ArgSeqsT>
struct variant_transform_result_1_;
template <std::ptrdiff_t... Dims, std::ptrdiff_t... Strides, typename F, typename... ArgSeqsT>
struct variant_transform_result_1_<std::integer_sequence<std::ptrdiff_t, Dims...>, std::integer_sequence<std::ptrdiff_t, Strides...>, F, ArgSeqsT...>
{
    template <std::size_t I>
    struct for_idx
    {
        using type = decltype(std::declval<F>()(std::declval<nth_variant_type<(I / std::size_t(Strides)) % std::size_t(Dims), ArgSeqsT>>()...));
    };
};

template <typename LinearIndices, typename ShapeT, typename StridesT, typename F, typename... ArgSeqsT>
struct variant_transform_results_0_;
template <std::size_t... Is, typename ShapeT, typename StridesT, typename F, typename... ArgSeqsT>
struct variant_transform_results_0_<std::index_sequence<Is...>, ShapeT, StridesT, F, ArgSeqsT...>
{
    using V1 = variant_transform_result_1_<ShapeT, StridesT, F, ArgSeqsT...>;
    using type = type_seq_<typename V1::template for_idx<Is>::type...>;
};

template <template <typename...> class VariantT, typename F, typename... Vs>
struct variant_transform_result_0_
{
    using shape_ = std::integer_sequence<std::ptrdiff_t, seq_size_<std::remove_const_t<std::remove_reference_t<Vs>>>::value...>;
    using strides_ = compute_strides_t<shape_>;
    static constexpr std::size_t numOptions_ = std::size_t(compute_size_<shape_>::value);
    using type = typename variant_transform_results_0_<std::make_index_sequence<numOptions_>, shape_, strides_, F, Vs...>::type;
};

template <template <typename...> class VariantT, typename F, typename... Vs>
struct variant_transform_result_
{
    using result_seq_ = typename variant_transform_result_0_<VariantT, F, Vs...>::type;
    using unique_result_seq_ = typename unique_sequence_<result_seq_>::type;
    using type = typename instantiate_<VariantT, unique_result_seq_>::type;
};
template <template <typename...> class VariantT, typename F, typename... Vs>
using variant_transform_result = typename variant_transform_result_<VariantT, F, Vs...>::type;

template <template <typename...> class VariantT, typename Rs, typename Vs>
struct flatten_variant_;
template <template <typename...> class VariantT, typename Rs>
struct flatten_variant_<VariantT, Rs, type_seq_<>>
{
    using type = Rs;
};
template <template <typename...> class VariantT, typename... Rs, typename... V0Ts, typename... Vs>
struct flatten_variant_<VariantT, type_seq_<Rs...>, type_seq_<VariantT<V0Ts...>, Vs...>>
    : flatten_variant_<VariantT, type_seq_<Rs..., V0Ts...>, type_seq_<Vs...>>
{
};

template <template <typename...> class VariantT, typename F, typename... Vs>
struct variant_transform_many_result_
{
    using result_seq_ = typename variant_transform_result_0_<VariantT, F, Vs...>::type;
    using flat_result_seq_ = typename flatten_variant_<VariantT, type_seq_<>, result_seq_>::type;
    using unique_result_seq_ = typename unique_sequence_<flat_result_seq_>::type;
    using type = typename instantiate_<VariantT, unique_result_seq_>::type;
};
template <template <typename...> class VariantT, typename F, typename... Vs>
using variant_transform_many_result = typename variant_transform_many_result_<VariantT, F, Vs...>::type;


} // namespace detail

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_DETAIL_VARIANT_HPP_
