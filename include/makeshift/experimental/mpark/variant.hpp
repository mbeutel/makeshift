
#ifndef INCLUDED_MAKESHIFT_EXPERIMENTAL_MPARK_VARIANT_HPP_
#define INCLUDED_MAKESHIFT_EXPERIMENTAL_MPARK_VARIANT_HPP_


#include <utility>     // for forward<>()
#include <type_traits> // for remove_cv<>, remove_reference<>

#include <gsl-lite/gsl-lite.hpp> // for gsl_Expects(), gsl_NODISCARD

#include <mpark/variant.hpp>

#include <makeshift/experimental/detail/variant.hpp>


namespace makeshift {


namespace gsl = ::gsl_lite;


namespace mpark {


    //
    // Given an argument of type `mpark::variant<Ts...>`, this is `mpark::variant<::mpark::monostate, Ts...>`.
    //
template <typename V> using with_monostate = typename detail::with_monostate_<::mpark::variant, ::mpark::monostate, V>::type;

    //
    // Given an argument of type `mpark::variant<::mpark::monostate, Ts...>`, this is `mpark::variant<Ts...>`.
    //
template <typename V> using without_monostate = typename detail::without_monostate_<::mpark::variant, ::mpark::monostate, V>::type;

    //
    // Casts an argument of type `mpark::variant<Ts...>` to the given variant type.
    //
template <typename DstV, typename SrcV>
gsl_NODISCARD constexpr DstV
variant_cast(SrcV&& variant)
{
#if !(defined(_MSC_VER) && defined(__INTELLISENSE__))
    return ::mpark::visit(
        [](auto&& arg) -> DstV
        {
            return std::forward<decltype(arg)>(arg);
        },
        std::forward<SrcV>(variant));
#endif // MAKESHIFT_INTELLISENSE
}


    //
    // Converts an argument of type `mpark::variant<::mpark::monostate, Ts...>` to `std::optional<::mpark::variant<Ts...>>`.
    //
//template <typename V>
//gsl_NODISCARD constexpr decltype(auto)
//variant_to_optional(V&& variantWithMonostate)
//{
//    using R = without_monostate<std::remove_cv_t<std::remove_reference_t<V>>>;
//    if (std::holds_alternative<::mpark::monostate>(variantWithMonostate))
//    {
//        return std::optional<R>(std::nullopt);
//    }
//#if !(defined(_MSC_VER) && defined(__INTELLISENSE__))
//    return std::optional<R>(::mpark::visit(
//        detail::monostate_filtering_visitor<::mpark::monostate, R>{ },
//        std::forward<V>(variantWithMonostate)));
//#endif // MAKESHIFT_INTELLISENSE
//}

    //
    // Converts an argument of type `std::optional<::mpark::variant<Ts...>>` to `mpark::variant<::mpark::monostate, Ts...>`.
    //
//template <typename VO>
//gsl_NODISCARD constexpr decltype(auto)
//optional_to_variant(VO&& optionalVariant)
//{
//    using R = with_monostate<typename std::remove_cv_t<std::remove_reference_t<VO>>::value_type>;
//    if (!optionalVariant.has_value())
//    {
//        return R{ ::mpark::monostate{ } };
//    }
//    return variant_cast<R>(*std::forward<VO>(optionalVariant));
//}


    //
    // Concatenates the alternatives in the given variants.
    //
template <typename... Vs> using variant_cat_t = typename detail::variant_cat_<::mpark::variant, Vs...>::type;


} // namespace mpark

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_EXPERIMENTAL_MPARK_VARIANT_HPP_
