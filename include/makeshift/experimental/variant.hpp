
#ifndef INCLUDED_MAKESHIFT_EXPERIMENTAL_VARIANT_HPP_
#define INCLUDED_MAKESHIFT_EXPERIMENTAL_VARIANT_HPP_


#include <gsl-lite/gsl-lite.hpp> // for gsl_CPP17_OR_GREATER, gsl_NODISCARD

#if !gsl_CPP17_OR_GREATER
# error Header <makeshift/experimental/variant.hpp> requires C++17 mode or higher.
#endif // !gsl_CPP17_OR_GREATER

#include <utility>     // for forward<>()
#include <variant>
#include <optional>
#include <type_traits> // for remove_cv<>, remove_reference<>

#include <makeshift/experimental/detail/variant.hpp>


namespace makeshift {

namespace gsl = ::gsl_lite;


    //
    // Given an argument of type `std::variant<Ts...>`, this is `std::variant<std::monostate, Ts...>`.
    //
template <typename V> using with_monostate = typename detail::with_monostate_<std::variant, std::monostate, V>::type;

    //
    // Given an argument of type `std::variant<std::monostate, Ts...>`, this is `std::variant<Ts...>`.
    //
template <typename V> using without_monostate = typename detail::without_monostate_<std::variant, std::monostate, V>::type;

    //
    // Casts an argument of type `std::variant<Ts...>` to the given variant type.
    //
template <typename DstV, typename SrcV>
gsl_NODISCARD constexpr DstV
variant_cast(SrcV&& variant)
{
#if !defined(__INTELLISENSE__)
# if gsl_CPP20_OR_GREATER
    return std::visit<DstV>(
        [](auto&& arg) -> DstV
        {
            return std::forward<decltype(arg)>(arg);
        },
        std::forward<SrcV>(variant));
# else // gsl_CPP20_OR_GREATER
    return std::visit(
        [](auto&& arg) -> DstV
        {
            return std::forward<decltype(arg)>(arg);
        },
        std::forward<SrcV>(variant));
# endif // gsl_CPP20_OR_GREATER}
#endif // !defined(__INTELLISENSE__)
}

    //
    // Converts an argument of type `std::variant<std::monostate, Ts...>` to `std::optional<std::variant<Ts...>>`.
    //
template <typename V>
gsl_NODISCARD constexpr decltype(auto)
variant_to_optional(V&& variantWithMonostate)
{
    using R = without_monostate<std::remove_cv_t<std::remove_reference_t<V>>>;
    if (std::holds_alternative<std::monostate>(variantWithMonostate))
    {
        return std::optional<R>(std::nullopt);
    }
#if !defined(__INTELLISENSE__)
# if gsl_CPP20_OR_GREATER
    return std::optional<R>(std::visit<R>(
        detail::monostate_filtering_visitor<std::monostate, R>{ },
        std::forward<V>(variantWithMonostate)));
# else // gsl_CPP20_OR_GREATER
    return std::optional<R>(std::visit(
        detail::monostate_filtering_visitor<std::monostate, R>{ },
        std::forward<V>(variantWithMonostate)));
# endif // gsl_CPP20_OR_GREATER}
#endif // !defined(__INTELLISENSE__)
}

    //
    // Converts an argument of type `std::optional<std::variant<Ts...>>` to `std::variant<std::monostate, Ts...>`.
    //
template <typename VO>
gsl_NODISCARD constexpr decltype(auto)
optional_to_variant(VO&& optionalVariant)
{
    using R = with_monostate<typename std::remove_cv_t<std::remove_reference_t<VO>>::value_type>;
    if (!optionalVariant.has_value())
    {
        return R{ std::monostate{ } };
    }
    return variant_cast<R>(*std::forward<VO>(optionalVariant));
}


    //
    // Concatenates the alternatives in the given variants.
    //
template <typename... Vs> using variant_cat_t = typename detail::variant_cat_<std::variant, Vs...>::type;


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_EXPERIMENTAL_VARIANT_HPP_
