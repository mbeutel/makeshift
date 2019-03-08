
#ifndef INCLUDED_MAKESHIFT_VARIANT2_HPP_
#define INCLUDED_MAKESHIFT_VARIANT2_HPP_


#include <utility>     // for forward<>()
#include <functional>  // for equal_to<>
#include <stdexcept>   // for runtime_error

#include <gsl/gsl_assert> // for Expects()

#include <makeshift/functional2.hpp> // for hash2<>
#include <makeshift/version.hpp>     // for MAKESHIFT_NODISCARD

#include <makeshift/detail/variant2.hpp>


namespace makeshift
{

inline namespace types
{


    //ᅟ
    // Determines whether a type has a variant-like interface (i.e. whether `std::variant_size<T>::value` is well-formed).
    //
template <typename T> struct is_variant_like : can_apply<makeshift::detail::is_variant_like_r, T> { };

    //ᅟ
    // Determines whether a type has a variant-like interface (i.e. whether `std::variant_size<T>::value` is well-formed).
    //
template <typename T> constexpr bool is_variant_like_v = is_variant_like<T>::value;


    //ᅟ
    // Exception class thrown by `expand()` if the runtime value to be expanded is not among the values listed.
    //
class unsupported_runtime_value : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};


constexpr inline makeshift::detail::member_values_initializer_t member_values = { };


    //ᅟ
    // Given a runtime value, a retriever of known values, a hasher, and an equality comparer, returns an optional variant of retrievers
    // of the known values. The result is `nullopt` if the runtime value is not among the values listed.
    //ᅟ
    //ᅟ    int bits = ...;
    //ᅟ    auto bitsV = try_expand(bits, []{ return values<int> = { 16, 32, 64 }; },
    //ᅟ        hash2<>{ }, std::equal_to<>{ });
    //ᅟ
    //ᅟ    std::visit(
    //ᅟ        [](auto bitsR)
    //ᅟ        {
    //ᅟ            constexpr int bitsC = retrieve(bitsR);
    //ᅟ            ...
    //ᅟ        },
    //ᅟ        bitsV.value());
    //
template <typename T, typename R, typename HashT, typename EqualToT>
    MAKESHIFT_NODISCARD constexpr auto try_expand2(const T& value, R valuesR, HashT&& hash, EqualToT&& equal)
{
    return makeshift::detail::expand2_impl0(value, valuesR, std::forward<HashT>(hash), std::forward<EqualToT>(equal));
}

    //ᅟ
    // Given a runtime value and a retriever of known values, returns an optional variant of retrievers of the known values. The result is
    // `nullopt` if the runtime value is not among the values listed.
    //ᅟ
    //ᅟ    int bits = ...;
    //ᅟ    auto bitsV = try_expand(bits, []{ return values<int> = { 16, 32, 64 }; });
    //ᅟ
    //ᅟ    std::visit(
    //ᅟ        [](auto bitsR)
    //ᅟ            constexpr int bitsC = retrieve(bitsR);
    //ᅟ            ...
    //ᅟ        },
    //ᅟ        bitsV.value());
    //
template <typename T, typename R>
    MAKESHIFT_NODISCARD constexpr auto try_expand2(const T& value, R valuesR)
{
    return makeshift::try_expand2(value, valuesR, hash2<>{ }, std::equal_to<>{ });
}


    //ᅟ
    // Given a runtime value, a retriever of known values, a hasher, and an equality comparer, returns an optional variant of retrievers
    // of the known values. An exception of type `unsupported_runtime_value` is thrown if the runtime value is not among the values listed.
    //ᅟ
    //ᅟ    int bits = ...;
    //ᅟ    auto bitsV = expand_or_throw(bits, []{ return values<int> = { 16, 32, 64 }; },
    //ᅟ        hash2<>{ }, std::equal_to<>{ });
    //ᅟ
    //ᅟ    std::visit(
    //ᅟ        [](auto bitsR)
    //ᅟ        {
    //ᅟ            constexpr int bitsC = retrieve(bitsR);
    //ᅟ            ...
    //ᅟ        },
    //ᅟ        bitsV);
    //
template <typename T, typename R, typename HashT, typename EqualToT>
    MAKESHIFT_NODISCARD constexpr auto expand2_or_throw(const T& value, R valuesR, HashT&& hash, EqualToT&& equal)
{
    auto maybeResult = makeshift::try_expand2(value, valuesR, std::forward<HashT>(hash), std::forward<EqualToT>(equal));
    if (!maybeResult.has_value())
        throw unsupported_runtime_value("unsupported runtime value");
    return *maybeResult;
}

    //ᅟ
    // Given a runtime value and a retriever of known values, returns an optional variant of retrievers of the known values. An exception
    // of type `unsupported_runtime_value` is thrown if the runtime value is not among the values listed.
    //ᅟ
    //ᅟ    int bits = ...;
    //ᅟ    auto bitsV = expand_or_throw(bits, []{ return values<int> = { 16, 32, 64 }; });
    //ᅟ
    //ᅟ    std::visit(
    //ᅟ        [](auto bitsR)
    //ᅟ        {
    //ᅟ            constexpr int bitsC = retrieve(bitsR);
    //ᅟ            ...
    //ᅟ        },
    //ᅟ        bitsV);
    //
template <typename T, typename R>
    MAKESHIFT_NODISCARD constexpr auto expand2_or_throw(const T& value, R valuesR)
{
    return makeshift::expand2_or_throw(value, valuesR, hash2<>{ }, std::equal_to<>{ });
}


    //ᅟ
    // Given a runtime value with metadata, returns an optional variant of retrievers of the known values.
    //ᅟ
    //ᅟ    enum class Precision { single, double_ };
    //ᅟ    constexpr static auto reflect(mk::type<Precision>) { return values<Precision> = { Precision::single, Precision::double_ }; }
    //ᅟ
    //ᅟ    Precision precision = ...;
    //ᅟ    auto precisionV = expand(precision);
    //ᅟ
    //ᅟ    std::visit(
    //ᅟ        [](auto precisionR)
    //ᅟ        {
    //ᅟ            constexpr int precisionC = retrieve(precisionR);
    //ᅟ            ...
    //ᅟ        },
    //ᅟ        precisionV);
    //
template <typename T>
    MAKESHIFT_NODISCARD constexpr auto expand2(const T& value)
{
    auto maybeResult = makeshift::try_expand2(value,
        makeshift::detail::values_of_retriever<T>{ });
    Expects(maybeResult.has_value());
    return *maybeResult;
}


    //ᅟ
    // Like `std::visit()`, but permits the functor to return different types for different argument types, and returns a variant of the possible results.
    //
template <typename F, typename... VariantsT,
          typename = std::enable_if_t<std::conjunction<is_variant_like<std::decay_t<VariantsT>>...>::value>>
    MAKESHIFT_NODISCARD constexpr typename makeshift::detail::visit_many_result_<F, VariantsT...>::type
    visit_many(F&& func, VariantsT&&... variants)
{
    using VisitManyResult = typename makeshift::detail::visit_many_result_<F, VariantsT...>::type;
    return std::visit(
        [func = std::forward<F>(func)](auto&&... args)
        {
            return VisitManyResult{ func(std::forward<decltype(args)>(args)...) };
        },
        std::forward<VariantsT>(variants)...);
}


    //ᅟ
    // Extracts the value of a retriever of a variant.
    //ᅟ
    //ᅟ    using FloatType = std::variant<type<float>, type<double>>;
    //ᅟ    auto floatTypeR = []{ return FloatType{ type_v<double> } };
    //ᅟ    constexpr auto floatType = retrieve_variant(floatTypeR); // = type_v<double>
    //
template <typename R>
    MAKESHIFT_NODISCARD constexpr auto retrieve_variant(const R& variantR)
{
    static_assert(is_variant_like_v<retrieved_t<R>>, "argument must retrieve a variant");
    constexpr auto theVariant = makeshift::retrieve<R>();
    return std::get<theVariant.index()>(theVariant);
}


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_VARIANT2_HPP_
