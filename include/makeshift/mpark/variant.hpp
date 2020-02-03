
#ifndef INCLUDED_MAKESHIFT_MPARK_VARIANT_HPP_
#define INCLUDED_MAKESHIFT_MPARK_VARIANT_HPP_


#include <type_traits> // for remove_cv<>, remove_reference<>
#include <cstddef>     // for ptrdiff_t
#include <utility>     // for integer_sequence<>

#include <gsl-lite/gsl-lite.hpp> // for gsl_Expects(), gsl_NODISCARD

#include <mpark/variant.hpp>

#include <makeshift/constval.hpp> // for ref_constval<>
#include <makeshift/reflect.hpp>  // for have_values_of<>, values_of<>

#include <makeshift/detail/variant.hpp>


namespace makeshift {


namespace gsl = ::gsl_lite;


namespace mpark {


    //
    // Given a runtime value and a constexpr array of values, `expand()` returns a variant of known constexpr values.
    // `gsl_Expects()` is used to ensure that the runtime value is among the values in the array.
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
gsl_NODISCARD constexpr typename detail::constval_variant_map<::mpark::variant, ValuesC>::type
expand(T const& value, ValuesC valuesC)
{
    std::ptrdiff_t index = detail::search_value_index(value, valuesC);
    gsl_Expects(index >= 0);
    return detail::constval_variant_map<::mpark::variant, ValuesC>::values[index];
}

    //
    // Given a runtime value of a type for which all possible values are known, `expand()` returns a variant of known constexpr values.
    // `gsl_Expects()` is used to ensure that the runtime value is among the values in the array.
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
    return makeshift::mpark::expand(value, makeshift::constval_t<detail::values_of_<T>>{ });
}

    //
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
//template <typename T, typename ValuesC>
//gsl_NODISCARD constexpr std::optional<typename detail::constval_variant_map<::mpark::variant, ValuesC>::type>
//try_expand(T const& value, ValuesC valuesC)
//{
//    std::ptrdiff_t index = detail::search_value_index(value, valuesC);
//    if (index < 0) return std::nullopt;
//    return { detail::constval_variant_map<::mpark::variant, ValuesC>::values[index] };
//}

    //
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
//template <typename T>
//gsl_NODISCARD constexpr auto
//try_expand(T const& value)
//{
//    static_assert(have_values_of_v<T>, "try_expand() cannot find admissible values");
//    return makeshift::mpark::try_expand(value, makeshift::constval_t<detail::values_of_<T>>{ });
//}

    //
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
gsl_NODISCARD constexpr typename detail::constval_variant_map<::mpark::variant, ValuesC>::type
expand_or_throw(T const& value, ValuesC valuesC)
{
    std::ptrdiff_t index = detail::search_value_index(value, valuesC);
    if (index < 0) throw unsupported_runtime_value{ };
    return detail::constval_variant_map<::mpark::variant, ValuesC>::values[index];
}

    //
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
    return makeshift::mpark::expand_or_throw(value, makeshift::constval_t<detail::values_of_<T>>{ });
}


    //
    // Equivalent to `mpark::visit()`.
    //ᅟ
    // Suppresses any template instantiations for intellisense parsers to improve responsivity.
    //
template <typename F, typename... Vs>
gsl_NODISCARD constexpr auto
visit(F&& func, Vs&&... args)
#if !defined(__INTELLISENSE__)
    -> decltype(::mpark::visit(std::forward<F>(func), std::forward<Vs>(args)...))
#endif // !defined(__INTELLISENSE__)
{
#if !defined(__INTELLISENSE__)
    return ::mpark::visit(std::forward<F>(func), std::forward<Vs>(args)...);
#endif // !defined(__INTELLISENSE__)
}

    //
    // Equivalent to `mpark::visit<>()`.
    //ᅟ
    // Suppresses any template instantiations for intellisense parsers to improve responsivity.
    //
template <typename R, typename F, typename... Vs>
gsl_NODISCARD constexpr R
visit(F&& func, Vs&&... args)
{
#if !defined(__INTELLISENSE__)
    return ::mpark::visit(
        [func = std::forward<F>(func)]
        (auto&&... args) -> R
        {
            return func(std::forward<decltype(args)>(args)...);
        },
        std::forward<Vs>(args)...);
#endif // !defined(__INTELLISENSE__)
}


    //
    // Similar to `mpark::visit()`, but permits the functor to map different argument types to different result types and returns a variant of the possible results.
    //ᅟ
    // `variant_transform()` merges identical result types, i.e. every distinct result type appears only once in the resulting variant type.
    // Suppresses any template instantiations for intellisense parsers to improve responsivity.
    //
template <typename F, typename... Vs>
gsl_NODISCARD constexpr decltype(auto)
variant_transform(F&& func, Vs&&... args)
{
    // Currently we merge identical results, i.e. if two functor invocations both return the same type, the type appears only once in the result variant.
    // Although `mpark::variant<>` is explicitly designed to permit multiple alternatives of identical type, it seems reasonable to merge identically typed alternatives here because identically typed alternatives
    // cannot be distinguished by the visitor functor anyway, and because the choice of identically typed alternatives depends on the strides of the specialization table built by `visit()` (which is an implementation
    // detail) and hence cannot be reliably predicted by the caller.

#if !defined(__INTELLISENSE__)
    using R = detail::variant_transform_result<::mpark::variant, F, Vs...>;
    return ::mpark::visit(
        [func = std::forward<F>(func)]
        (auto&&... args) -> R
        {
            return func(std::forward<decltype(args)>(args)...);
        },
        std::forward<Vs>(args)...);
#endif // MAKESHIFT_INTELLISENSE
}

    //
    // Similar to `mpark::visit()`, but permits the functor to map different argument types to different variants and returns an unwrapped variant of the possible results.
    //ᅟ
    // `variant_transform_many()` merges identical result types, i.e. every distinct result type appears only once in the resulting variant type.
    // Suppresses any template instantiations for intellisense parsers to improve responsivity.
    //
template <typename F, typename... Vs>
gsl_NODISCARD constexpr decltype(auto)
variant_transform_many(F&& func, Vs&&... args)
{
#if !defined(__INTELLISENSE__)
    using R = detail::variant_transform_many_result<::mpark::variant, F, Vs...>;
    return ::mpark::visit(
        [func = std::forward<F>(func)]
        (auto&&... args) -> R
        {
            return ::mpark::visit(
                [](auto&& result) -> R
                {
                    return std::forward<decltype(result)>(result);
                },
                func(std::forward<decltype(args)>(args)...));
        },
        std::forward<Vs>(args)...);
#endif // MAKESHIFT_INTELLISENSE
}


} // namespace mpark

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_MPARK_VARIANT_HPP_
