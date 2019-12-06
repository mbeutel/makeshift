
#ifndef INCLUDED_MAKESHIFT_VARIANT_HPP_
#define INCLUDED_MAKESHIFT_VARIANT_HPP_


#include <gsl/gsl-lite.hpp> // for gsl_CPP17_OR_GREATER, gsl_NODISCARD

#if !gsl_CPP17_OR_GREATER
# error Header <makeshift/variant.hpp> requires C++17 mode or higher.
#endif // !gsl_CPP17_OR_GREATER

#include <variant>
#include <optional>
#include <type_traits> // for remove_cv<>, remove_reference<>

#include <makeshift/constval.hpp> // for ref_constval<>
#include <makeshift/reflect.hpp>  // for have_values_of<>, values_of<>

#include <makeshift/detail/variant.hpp>


namespace makeshift
{


    //ᅟ
    // Given a runtime value and a constexpr array of values, `expand()` returns a variant of known constexpr values.
    // `Expects()` is used to ensure that the runtime value is among the values in the array.
    //ᅟ
    //ᅟ    int bits = ...;
    //ᅟ    auto bitsV = expand(bits, MAKESHIFT_CONSTVAL(std::array{ 16, 32, 64 }));
    //ᅟ
    //ᅟ    visit(
    //ᅟ        [](auto bitsC) {
    //ᅟ            constexpr int bits = bitsC();
    //ᅟ            ...
    //ᅟ        },
    //ᅟ        bitsV);
    //
template <typename T, typename ValuesC>
gsl_NODISCARD constexpr typename detail::constval_variant_map<std::variant, ValuesC>::type
expand(T const& value, ValuesC valuesC)
{
    std::ptrdiff_t index = detail::search_value_index(value, valuesC);
    Expects(index >= 0);
    return detail::constval_variant_map<std::variant, ValuesC>::values[index];
}

    //ᅟ
    // Given a runtime value of a type for which all possible values are known, `expand()` returns a variant of known constexpr values.
    // `Expects()` is used to ensure that the runtime value is among the values in the array.
    //ᅟ
    //ᅟ    bool logging = ...;
    //ᅟ    auto loggingV = expand(logging);
    //ᅟ
    //ᅟ    visit(
    //ᅟ        [](auto loggingC) {
    //ᅟ            ...
    //ᅟ            if constexpr (loggingC) {
    //ᅟ                std::clog << "Log message: " << ... << '\n';
    //ᅟ            }
    //ᅟ            ...
    //ᅟ        },
    //ᅟ        loggingV);
    //
template <typename T>
gsl_NODISCARD constexpr auto
expand(T const& value)
{
    static_assert(have_values_of_v<T>, "expand() cannot find admissible values");
    return makeshift::expand(value, makeshift::make_constval_t<detail::values_of_<T>>{ });
}

    //ᅟ
    // Given a runtime value and a constexpr array of values, `try_expand()` returns an optional variant of known constexpr values.
    // The result is `std::nullopt` if the runtime value is not among the values in the array.
    //ᅟ
    //ᅟ    int bits = ...;
    //ᅟ    auto bitsVO = try_expand(bits, MAKESHIFT_CONSTVAL(std::array{ 16, 32, 64 }));
    //ᅟ    if (!bitsVO.has_value()) return;
    //ᅟ
    //ᅟ    visit(
    //ᅟ        [](auto bitsC) {
    //ᅟ            constexpr int bits = bitsC();
    //ᅟ            ...
    //ᅟ        },
    //ᅟ        bitsVO.value());
    //
template <typename T, typename ValuesC>
gsl_NODISCARD constexpr std::optional<typename detail::constval_variant_map<std::variant, ValuesC>::type>
try_expand(T const& value, ValuesC valuesC)
{
    std::ptrdiff_t index = detail::search_value_index(value, valuesC);
    if (index < 0) return std::nullopt;
    return { detail::constval_variant_map<std::variant, ValuesC>::values[index] };
}

    //ᅟ
    // Given a runtime value of a type for which all possible values are known, `try_expand()` returns an optional variant of known constexpr values.
    // The result is `std::nullopt` if the runtime value is not among the values in the array.
    //ᅟ
    //ᅟ    enum Color { red, green, blue };
    //ᅟ    constexpr auto reflect_values(type<Color>) { return std::array{ red, green, blue }; }
    //ᅟ
    //ᅟ    auto color = ...;
    //ᅟ    auto colorVO = try_expand(color);
    //ᅟ    if (!colorVO.has_value()) return;
    //ᅟ
    //ᅟ    visit(
    //ᅟ        [](auto colorC) {
    //ᅟ            constexpr Color color = colorC();
    //ᅟ            ...
    //ᅟ        },
    //ᅟ        colorVO.value());
    //
template <typename T>
gsl_NODISCARD constexpr auto
try_expand(T const& value)
{
    static_assert(have_values_of_v<T>, "try_expand() cannot find admissible values");
    return makeshift::try_expand(value, makeshift::make_constval_t<detail::values_of_<T>>{ });
}

    //ᅟ
    // Given a runtime value and a constexpr value list, `expand_or_throw()` returns a variant of known constexpr values.
    // An exception of type `unsupported_runtime_value` is thrown if the runtime value is not among the values in the array.
    //ᅟ
    //ᅟ    int bits = ...;
    //ᅟ    auto bitsV = expand_or_throw(bits, MAKESHIFT_CONSTVAL(std::array{ 16, 32, 64 }));
    //ᅟ
    //ᅟ    visit(
    //ᅟ        [](auto bitsC) {
    //ᅟ            constexpr int bits = bitsC();
    //ᅟ            ...
    //ᅟ        },
    //ᅟ        bitsV);
    //
template <typename T, typename ValuesC>
gsl_NODISCARD constexpr typename detail::constval_variant_map<std::variant, ValuesC>::type
expand_or_throw(T const& value, ValuesC valuesC)
{
    std::ptrdiff_t index = detail::search_value_index(value, valuesC);
    if (index < 0) throw unsupported_runtime_value{ };
    return detail::constval_variant_map<std::variant, ValuesC>::values[index];
}

    //ᅟ
    // Given a runtime value of a type for which all possible values are known, `expand_or_throw()` returns a variant of known constexpr values.
    // An exception of type `unsupported_runtime_value` is thrown if the runtime value is not among the values in the array.
    //ᅟ
    //ᅟ    enum Color { red, green, blue };
    //ᅟ    constexpr auto reflect_values(type<Color>) { return std::array{ red, green, blue }; }
    //ᅟ
    //ᅟ    auto color = ...;
    //ᅟ    auto colorV = expand_or_throw(color);
    //ᅟ
    //ᅟ    visit(
    //ᅟ        [](auto colorC) {
    //ᅟ            constexpr Color color = colorC();
    //ᅟ            ...
    //ᅟ        },
    //ᅟ        colorV);
    //
template <typename T>
gsl_NODISCARD constexpr auto
expand_or_throw(T const& value)
{
    static_assert(have_values_of_v<T>, "expand_or_throw() cannot find admissible values");
    return makeshift::expand_or_throw(value, makeshift::make_constval_t<detail::values_of_<T>>{ });
}


    //ᅟ
    // Equivalent to `std::visit()`.
    // Suppresses any template instantiations for intellisense parsers to improve responsivity.
    //
template <typename F, typename... Vs>
gsl_NODISCARD constexpr auto
visit(F&& func, Vs&&... args)
#if !(defined(_MSC_VER) && defined(__INTELLISENSE__))
    -> decltype(std::visit(std::forward<F>(func), std::forward<Vs>(args)...))
#endif // !(defined(_MSC_VER) && defined(__INTELLISENSE__))
{
#if !(defined(_MSC_VER) && defined(__INTELLISENSE__))
    return std::visit(std::forward<F>(func), std::forward<Vs>(args)...);
#endif // !(defined(_MSC_VER) && defined(__INTELLISENSE__))
}

    //ᅟ
    // Equivalent to `std::visit<>()`.
    // Suppresses any template instantiations for intellisense parsers to improve responsivity.
    //
template <typename R, typename F, typename... Vs>
gsl_NODISCARD constexpr R
visit(F&& func, Vs&&... args)
{
#if !(defined(_MSC_VER) && defined(__INTELLISENSE__))
 #if gsl_CPP20_OR_GREATER
    return std::visit<R>(std::forward<F>(func), std::forward<Vs>(args)...);
 #else // gsl_CPP20_OR_GREATER
    return std::visit(
        [func = std::forward<F>(func)]
        (auto&&... args) -> R
        {
            return func(std::forward<decltype(args)>(args)...);
        },
        std::forward<Vs>(args)...);
 #endif // gsl_CPP20_OR_GREATER
#endif // !(defined(_MSC_VER) && defined(__INTELLISENSE__))
}

    //ᅟ
    // Given an argument of type `std::variant<Ts...>`, this is `std::variant<std::monostate, Ts...>`.
    //
//template <typename V> using with_monostate = typename detail::with_monostate_<std::variant, std::monostate, V>::type;

    //ᅟ
    // Given an argument of type `std::variant<std::monostate, Ts...>`, this is `std::variant<Ts...>`.
    //
//template <typename V> using without_monostate = typename detail::without_monostate_<std::variant, std::monostate, V>::type;

    //ᅟ
    // Casts an argument of type `std::variant<Ts...>` to the given variant type.
    //
//template <typename DstV, typename SrcV>
//gsl_NODISCARD constexpr DstV
//variant_cast(SrcV&& variant)
//{
//#if !(defined(_MSC_VER) && defined(__INTELLISENSE__))
// #if gsl_CPP20_OR_GREATER
//    return std::visit<DstV>(
//        [](auto&& arg) -> DstV
//        {
//            return std::forward<decltype(arg)>(arg);
//        },
//        std::forward<SrcV>(variant));
// #else // gsl_CPP20_OR_GREATER
//    return std::visit(
//        [](auto&& arg) -> DstV
//        {
//            return std::forward<decltype(arg)>(arg);
//        },
//        std::forward<SrcV>(variant));
// #endif // gsl_CPP20_OR_GREATER}
//#endif // !(defined(_MSC_VER) && defined(__INTELLISENSE__))
//}

    //ᅟ
    // Converts an argument of type `std::variant<std::monostate, Ts...>` to `std::optional<std::variant<Ts...>>`.
    //
//template <typename V>
//gsl_NODISCARD constexpr decltype(auto)
//variant_to_optional(V&& variantWithMonostate)
//{
//    using R = without_monostate<std::remove_cv_t<std::remove_reference_t<V>>>;
//    if (std::holds_alternative<std::monostate>(variantWithMonostate))
//    {
//        return std::optional<R>(std::nullopt);
//    }
//#if !(defined(_MSC_VER) && defined(__INTELLISENSE__))
// #if gsl_CPP20_OR_GREATER
//    return std::optional<R>(std::visit<R>(
//        detail::monostate_filtering_visitor<std::monostate, R>{ },
//        std::forward<V>(variantWithMonostate)));
// #else // gsl_CPP20_OR_GREATER
//    return std::optional<R>(std::visit(
//        detail::monostate_filtering_visitor<std::monostate, R>{ },
//        std::forward<V>(variantWithMonostate)));
// #endif // gsl_CPP20_OR_GREATER}
//#endif // !(defined(_MSC_VER) && defined(__INTELLISENSE__))
//}

    //ᅟ
    // Converts an argument of type `std::optional<std::variant<Ts...>>` to `std::variant<std::monostate, Ts...>`.
    //
//template <typename VO>
//gsl_NODISCARD constexpr decltype(auto)
//optional_to_variant(VO&& optionalVariant)
//{
//    using R = with_monostate<typename std::remove_cv_t<std::remove_reference_t<VO>>::value_type>;
//    if (!optionalVariant.has_value())
//    {
//        return R{ std::monostate{ } };
//    }
//    return variant_cast<R>(*std::forward<VO>(optionalVariant));
//}

    //ᅟ
    // Similar to `std::visit()`, but permits the functor to map different argument types to different result types and returns a variant of the possible results.
    // `variant_transform()` merges identical result types, i.e. every distinct result type appears only once in the resulting variant type.
    // Suppresses any template instantiations for intellisense parsers to improve responsivity.
    //
template <typename F, typename... Vs>
gsl_NODISCARD constexpr decltype(auto)
variant_transform(F&& func, Vs&&... args)
{
    // Currently we merge identical results, i.e. if two functor invocations both return the same type, the type appears only once in the result variant.
    // Although `std::variant<>` is explicitly designed to permit multiple alternatives of identical type, it seems reasonable to merge identically typed alternatives here because identically typed alternatives
    // cannot be distinguished by the visitor functor anyway, and because the choice of identically typed alternatives depends on the strides of the specialization table built by `visit()` (which is an implementation
    // detail) and hence cannot be reliably predicted by the caller.

#if !(defined(_MSC_VER) && defined(__INTELLISENSE__))
    using R = detail::variant_transform_result<std::variant, F, Vs...>;
 #if gsl_CPP20_OR_GREATER
    return std::visit<R>(std::forward<F>(func), std::forward<Vs>(args)...);
 #else // gsl_CPP20_OR_GREATER
    return std::visit(
        [func = std::forward<F>(func)]
        (auto&&... args) -> R
        {
            return func(std::forward<decltype(args)>(args)...);
        },
        std::forward<Vs>(args)...);
 #endif // gsl_CPP20_OR_GREATER
#endif // !(defined(_MSC_VER) && defined(__INTELLISENSE__))
}

    //ᅟ
    // Similar to `std::visit()`, but permits the functor to map different argument types to different variants and returns an unwrapped variant of the possible results.
    // `variant_transform_many()` merges identical result types, i.e. every distinct result type appears only once in the resulting variant type.
    // Suppresses any template instantiations for intellisense parsers to improve responsivity.
    //
template <typename F, typename... Vs>
gsl_NODISCARD constexpr decltype(auto)
variant_transform_many(F&& func, Vs&&... args)
{
#if !(defined(_MSC_VER) && defined(__INTELLISENSE__))
    using R = detail::variant_transform_many_result<std::variant, F, Vs...>;
 #if gsl_CPP20_OR_GREATER
    return std::visit<R>(
        [func = std::forward<F>(func)]
        (auto&&... args) -> R
        {
            return std::visit<R>(
                [](auto&& result) -> R
                {
                    return std::forward<decltype(result)>(result);
                },
                func(std::forward<decltype(args)>(args)...));
        },
        std::forward<Vs>(args)...);
 #else // gsl_CPP20_OR_GREATER
    return std::visit(
        [func = std::forward<F>(func)]
        (auto&&... args) -> R
        {
            return std::visit(
                [](auto&& result) -> R
                {
                    return std::forward<decltype(result)>(result);
                },
                func(std::forward<decltype(args)>(args)...));
        },
        std::forward<Vs>(args)...);
 #endif // gsl_CPP20_OR_GREATER
#endif // !(defined(_MSC_VER) && defined(__INTELLISENSE__))
}


    //ᅟ
    // Concatenates the alternatives in the given variants.
    //
//template <typename... Vs> using variant_cat_t = typename detail::variant_cat_<std::variant, Vs...>::type;


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_VARIANT_HPP_
