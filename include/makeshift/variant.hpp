
#ifndef INCLUDED_MAKESHIFT_VARIANT_HPP_
#define INCLUDED_MAKESHIFT_VARIANT_HPP_


#include <cstddef>      // for ptrdiff_t
#include <utility>      // for forward<>()
#include <variant>
#include <optional>
#include <type_traits>  // for remove_cv<>, remove_reference<>, invoke_result<>

#include <gsl-lite/gsl-lite.hpp>  // for type_identity<>, gsl_Expects(), gsl_CPP17_OR_GREATER

#if !gsl_CPP17_OR_GREATER
# error makeshift requires C++17 mode or higher
#endif // !gsl_CPP17_OR_GREATER

#include <makeshift/constval.hpp>  // for ref_constval<>
#include <makeshift/metadata.hpp>

#include <makeshift/detail/variant.hpp>


namespace makeshift {

namespace gsl = ::gsl_lite;


    //
    // Given a runtime value and a constexpr array of values, `expand()` returns a variant of known constexpr values.
    // `gsl_Expects()` is used to ensure that the runtime value is among the values in the array.
    //ᅟ
    //ᅟ    int bits = ...;
    //ᅟ    auto bitsV = expand_failfast(bits, MAKESHIFT_CONSTVAL(std::array{ 16, 32, 64 }));
    //ᅟ
    //ᅟ    visit(
    //ᅟ        [](auto bitsC) {
    //ᅟ            constexpr int bits = bitsC();
    //ᅟ            ...
    //ᅟ        },
    //ᅟ        bitsV);
    //
template <typename T, typename ValuesC>
[[nodiscard]] constexpr typename detail::constval_variant_map<std::variant, ValuesC>::type
expand_failfast(T const& value, ValuesC valuesC)
{
    std::ptrdiff_t index = detail::search_value_index(value, valuesC);
    gsl_Expects(index >= 0);
    return detail::constval_variant_map<std::variant, ValuesC>::values[index];
}

    //
    // Given a runtime value of a type for which all possible values are known, `expand()` returns a variant of known constexpr
    // values. `gsl_Expects()` is used to ensure that the runtime value is among the values in the array.
    //ᅟ
    //ᅟ    bool logging = ...;
    //ᅟ    auto loggingV = expand_failfast(logging);
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
[[nodiscard]] constexpr auto
expand_failfast(T const& value)
{
    if constexpr (metadata::is_available_v<decltype(metadata::values<T>())>)
    {
        return makeshift::expand_failfast(value, MAKESHIFT_CONSTVAL(metadata::values<T>()));
    }
    else
    {
        static_assert(!sizeof(gsl::type_identity<T>), "expand_failfast() cannot find admissible values");
    }
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
template <typename T, typename ValuesC>
[[nodiscard]] constexpr std::optional<typename detail::constval_variant_map<std::variant, ValuesC>::type>
try_expand(T const& value, ValuesC valuesC)
{
    std::ptrdiff_t index = detail::search_value_index(value, valuesC);
    if (index < 0) return std::nullopt;
    return { detail::constval_variant_map<std::variant, ValuesC>::values[index] };
}

    //
    // Given a runtime value of a type for which all possible values are known, `try_expand()` returns an optional variant of
    // known constexpr values. The result is `std::nullopt` if the runtime value is not among the values in the array.
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
[[nodiscard]] constexpr auto
try_expand(T const& value)
{
    if constexpr (metadata::is_available_v<decltype(metadata::values<T>())>)
    {
        return makeshift::try_expand(value, MAKESHIFT_CONSTVAL(metadata::values<T>()));
    }
    else
    {
        static_assert(!sizeof(gsl::type_identity<T>), "try_expand() cannot find admissible values");
    }
}

    //
    // Given a runtime value and a constexpr value list, `expand()` returns a variant of known constexpr values.
    // An exception of type `unsupported_runtime_value` is thrown if the runtime value is not among the values in the array.
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
[[nodiscard]] constexpr typename detail::constval_variant_map<std::variant, ValuesC>::type
expand(T const& value, ValuesC valuesC)
{
    std::ptrdiff_t index = detail::search_value_index(value, valuesC);
    if (index < 0) throw unsupported_runtime_value{ };
    return detail::constval_variant_map<std::variant, ValuesC>::values[index];
}

    //
    // Given a runtime value of a type for which all possible values are known, `expand()` returns a variant of known
    // constexpr values. An exception of type `unsupported_runtime_value` is thrown if the runtime value is not among the values
    // in the array.
    //ᅟ
    //ᅟ    enum Color { red, green, blue };
    //ᅟ    constexpr auto reflect_values(type<Color>) { return std::array{ red, green, blue }; }
    //ᅟ
    //ᅟ    auto color = ...;
    //ᅟ    auto colorV = expand(color);
    //ᅟ
    //ᅟ    visit(
    //ᅟ        [](auto colorC) {
    //ᅟ            constexpr Color color = colorC();
    //ᅟ            ...
    //ᅟ        },
    //ᅟ        colorV);
    //
template <typename T>
[[nodiscard]] constexpr auto
expand(T const& value)
{
    if constexpr (metadata::is_available_v<decltype(metadata::values<T>())>)
    {
        return makeshift::expand(value, MAKESHIFT_CONSTVAL(metadata::values<T>()));
    }
    else
    {
        static_assert(!sizeof(gsl::type_identity<T>), "expand() cannot find admissible values");
    }
}


    //
    // Equivalent to `std::visit()`.
    //ᅟ
    // Suppresses any template instantiations for intellisense parsers to improve responsivity.
    //
template <typename F, typename... Vs>
[[nodiscard]] constexpr auto
visit(F&& func, Vs&&... args)
#if !defined(__INTELLISENSE__)
    -> decltype(std::visit(std::forward<F>(func), std::forward<Vs>(args)...))
#endif // !defined(__INTELLISENSE__)
{
#if defined(__INTELLISENSE__)
    return detail::convertible_to_anything{ };
#else
    return std::visit(std::forward<F>(func), std::forward<Vs>(args)...);
#endif // defined(__INTELLISENSE__)
}

    //
    // Equivalent to `std::visit<>()`.
    //ᅟ
    // Suppresses any template instantiations for intellisense parsers to improve responsivity.
    //
template <typename R, typename F, typename... Vs>
[[nodiscard]] constexpr R
visit(F&& func, Vs&&... args)
{
#if defined(__INTELLISENSE__)
    return detail::convertible_to_anything{ };
#else
//# if gsl_CPP20_OR_GREATER  // TODO: need more precise feature check
//    return std::visit<R>(std::forward<F>(func), std::forward<Vs>(args)...);
//# else // gsl_CPP20_OR_GREATER
    return std::visit(
        [&func]
        (auto&&... args) -> R
        {
            return std::forward<F>(func)(std::forward<decltype(args)>(args)...);
        },
        std::forward<Vs>(args)...);
//# endif // gsl_CPP20_OR_GREATER
#endif // !defined(__INTELLISENSE__)
}


    //
    // Similar to `std::visit()`, but permits the functor to map different argument types to different result types and returns a
    // variant of the possible results.
    //ᅟ
    // `variant_transform()` merges identical result types, i.e. every distinct result type appears only once in the resulting
    // variant type.
    // Suppresses any template instantiations for intellisense parsers to improve responsivity.
    //
template <typename F, typename... Vs>
[[nodiscard]] constexpr decltype(auto)
variant_transform(F&& func, Vs&&... args)
{
    gsl_DISABLE_MSVC_WARNINGS(4702)  // unreachable code

    // Currently we merge identical results, i.e. if two functor invocations both return the same type, the type appears only once
    // in the result variant. Although `std::variant<>` is explicitly designed to permit multiple alternatives of identical type,
    // it seems reasonable to merge identically typed alternatives here because identically typed alternatives cannot be
    // distinguished by the visitor functor anyway, and because the choice of identically typed alternatives depends on the
    // strides of the specialization table built by `visit()` (which is an implementation detail) and hence cannot be reliably
    // predicted by the caller.

#if defined(__INTELLISENSE__)
    return detail::convertible_to_anything{ };
#else
    using R = detail::variant_transform_result<std::variant, F, Vs...>;
    return std::visit
# if gsl_CPP20_OR_GREATER
    <R>
# endif // gsl_CPP20_OR_GREATER
    (
        [&func]
        (auto&&... args) -> R
        {
            using RR = std::invoke_result_t<F, decltype(args)...>;
            if constexpr (std::is_same_v<RR, void>)
            {
                std::forward<F>(func)(std::forward<decltype(args)>(args)...);
                gsl_FailFast();
            }
            else
            {
                return std::forward<F>(func)(std::forward<decltype(args)>(args)...);
            }
        },
        std::forward<Vs>(args)...);
#endif // defined(__INTELLISENSE__)

    gsl_RESTORE_MSVC_WARNINGS()
}

    //
    // Similar to `std::visit()`, but permits the functor to map different argument types to different variants and returns an
    // unwrapped variant of the possible results.
    //ᅟ
    // `variant_transform_many()` merges identical result types, i.e. every distinct result type appears only once in the
    // resulting variant type.
    // Suppresses any template instantiations for intellisense parsers to improve responsivity.
    //
template <typename F, typename... Vs>
[[nodiscard]] constexpr decltype(auto)
variant_transform_many(F&& func, Vs&&... args)
{
#if defined(__INTELLISENSE__)
    return detail::convertible_to_anything{ };
#else
    using R = detail::variant_transform_many_result<std::variant, F, Vs...>;
# if gsl_CPP20_OR_GREATER
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
# else // gsl_CPP20_OR_GREATER
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
# endif // gsl_CPP20_OR_GREATER
#endif // defined(__INTELLISENSE__)
}


} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_VARIANT_HPP_
