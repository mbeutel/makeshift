
#ifndef INCLUDED_MAKESHIFT_VARIANT2_HPP_
#define INCLUDED_MAKESHIFT_VARIANT2_HPP_


#include <utility>     // for forward<>()
#include <functional>  // for equal_to<>
#include <stdexcept>   // for runtime_error

#include <makeshift/functional2.hpp> // for hash2<>
#include <makeshift/version.hpp>     // for MAKESHIFT_CXX17, MAKESHIFT_NODISCARD

#include <makeshift/detail/variant2.hpp>
#include <makeshift/detail/workaround.hpp> // for cor()


namespace makeshift
{

inline namespace types
{


    //ᅟ
    // Determines whether a type has a variant-like interface (i.e. whether `std::variant_size<T>::value` is well-formed).
    //
template <typename T> struct is_variant_like : can_instantiate<makeshift::detail::is_variant_like_r, T> { };

    //ᅟ
    // Determines whether a type has a variant-like interface (i.e. whether `std::variant_size<T>::value` is well-formed).
    //
template <typename T> constexpr bool is_variant_like_v = is_variant_like<T>::value;


    //ᅟ
    // Exception class thrown by `expand_or_throw()` if the runtime value to be expanded is not among the values listed.
    //
class unsupported_runtime_value : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};


constexpr inline makeshift::detail::member_values_name member_values = { };


#ifdef MAKESHIFT_CXX17
    //ᅟ
    // Given a runtime value, a constexpr value list, a hasher, and an equality comparer, returns an optional variant of known
    // constexpr values. The result is `nullopt` if the runtime value is not among the values listed.
    //ᅟ
    //ᅟ    int bits = ...;
    //ᅟ    auto bitsVO = try_expand(bits, []{ return values<int> = { 16, 32, 64 }; },
    //ᅟ        hash<>{ }, std::equal_to<>{ });
    //ᅟ
    //ᅟ    visit(
    //ᅟ        [](auto bitsC)
    //ᅟ        {
    //ᅟ            constexpr int bits = bitsC();
    //ᅟ            ...
    //ᅟ        },
    //ᅟ        bitsVO.value());
    //
template <typename T, typename C, typename HashT, typename EqualToT>
    MAKESHIFT_NODISCARD constexpr auto try_expand2(const T& value, const C& valuesC, HashT&& hash, EqualToT&& equal)
{
    return makeshift::detail::expand2_impl0<makeshift::detail::result_handler_optional>(value, valuesC, std::forward<HashT>(hash), std::forward<EqualToT>(equal));
}


    //ᅟ
    // Given a runtime value and a constexpr value list, returns an optional variant of known constexpr values. The result is
    // `nullopt` if the runtime value is not among the values listed.
    //ᅟ
    //ᅟ    int bits = ...;
    //ᅟ    auto bitsVO = try_expand(bits, []{ return values<int> = { 16, 32, 64 }; });
    //ᅟ
    //ᅟ    visit(
    //ᅟ        [](auto bitsC)
    //ᅟ            constexpr int bits = bitsC();
    //ᅟ            ...
    //ᅟ        },
    //ᅟ        bitsVO.value());
    //
template <typename T, typename C>
    MAKESHIFT_NODISCARD constexpr auto try_expand2(const T& value, const C& valuesC)
{
    return makeshift::try_expand2(value, valuesC, hash2<>{ }, std::equal_to<>{ });
}
#endif // MAKESHIFT_CXX17


    //ᅟ
    // Given a runtime value, a constexpr value list, a hasher, and an equality comparer, returns a variant of known constexpr values.
    // An exception of type `unsupported_runtime_value` is thrown if the runtime value is not among the values listed.
    //ᅟ
    //ᅟ    int bits = ...;
    //ᅟ    auto bitsV = expand_or_throw(bits, []{ return values<int> = { 16, 32, 64 }; },
    //ᅟ        hash<>{ }, std::equal_to<>{ });
    //ᅟ
    //ᅟ    visit(
    //ᅟ        [](auto bitsC)
    //ᅟ        {
    //ᅟ            constexpr int bits = bitsC();
    //ᅟ            ...
    //ᅟ        },
    //ᅟ        bitsV);
    //
template <typename T, typename C, typename HashT, typename EqualToT>
    MAKESHIFT_NODISCARD constexpr auto expand2_or_throw(const T& value, const C& valuesC, HashT&& hash, EqualToT&& equal)
{
    return makeshift::detail::expand2_impl0<makeshift::detail::result_handler_throw>(value, valuesC, std::forward<HashT>(hash), std::forward<EqualToT>(equal));
}

    //ᅟ
    // Given a runtime value and a constexpr value list, returns a variant of known constexpr values. An exception
    // of type `unsupported_runtime_value` is thrown if the runtime value is not among the values listed.
    //ᅟ
    //ᅟ    int bits = ...;
    //ᅟ    auto bitsV = expand_or_throw(bits, []{ return values<int> = { 16, 32, 64 }; });
    //ᅟ
    //ᅟ    visit(
    //ᅟ        [](auto bitsC)
    //ᅟ        {
    //ᅟ            constexpr int bits = bitsC();
    //ᅟ            ...
    //ᅟ        },
    //ᅟ        bitsV);
    //
template <typename T, typename C>
    MAKESHIFT_NODISCARD constexpr auto expand2_or_throw(const T& value, const C& valuesC)
{
    return makeshift::expand2_or_throw(value, valuesC, hash2<>{ }, std::equal_to<>{ });
}


    //ᅟ
    // Given a runtime value and a constexpr value list, returns a variant of known constexpr values.
    //ᅟ
    //ᅟ    struct Params { bool foo; bool bar; };
    //ᅟ
    //ᅟ    Params params = ...;
    //ᅟ    auto paramsV = expand(params, []{ return member_values(&Params::foo) * member_values(&Params::bar); });
    //ᅟ
    //ᅟ    visit(
    //ᅟ        [](auto paramsC)
    //ᅟ            constexpr Params params = paramsC();
    //ᅟ            ...
    //ᅟ        },
    //ᅟ        paramsV);
    //
template <typename T, typename C>
    MAKESHIFT_NODISCARD constexpr auto expand2(const T& value, const C& valuesC)
{
    static_assert(makeshift::detail::is_exhaustive_v<C>, "use try_expand() or expand_or_throw() if the list of values is non-exhaustive");

    return makeshift::detail::expand2_impl0<makeshift::detail::result_handler_terminate>(value, valuesC, hash2<>{ }, std::equal_to<>{ });
}


    //ᅟ
    // Given a runtime value, returns a variant of known constexpr values.
    //ᅟ
    //ᅟ    enum class Precision { single, double_ };
    //ᅟ    constexpr static auto reflect(mk::type<Precision>) { return values<Precision> = { Precision::single, Precision::double_ }; }
    //ᅟ
    //ᅟ    Precision precision = ...;
    //ᅟ    auto precisionV = expand(precision);
    //ᅟ
    //ᅟ    visit(
    //ᅟ        [](auto precisionC)
    //ᅟ        {
    //ᅟ            constexpr int precision = precisionC();
    //ᅟ            ...
    //ᅟ        },
    //ᅟ        precisionV);
    //
template <typename T>
    MAKESHIFT_NODISCARD constexpr auto expand2(const T& value)
{
    return makeshift::expand2(value, constval<makeshift::detail::metadata_values_retriever<T>>);
}


    //ᅟ
    // Like `std::visit()`, but supports variant-like types and implicitly expands expandable non-variant arguments.
    //ᅟ
    //ᅟ    bool flag = ...;
    //ᅟ    visit(
    //ᅟ        [](auto flagC)
    //ᅟ        {
    //ᅟ            constexpr bool flag = flagC();
    //ᅟ            ...
    //ᅟ        },
    //ᅟ        flag);
    //
template <typename F, typename... Vs>
    constexpr decltype(auto) visit(F&& func, Vs&&... args)
{
#ifndef MAKESHIFT_INTELLISENSE
    return makeshift::detail::visit_impl_0(std::forward<F>(func), makeshift::detail::maybe_expand(std::forward<Vs>(args))...);
#endif // MAKESHIFT_INTELLISENSE
}



#ifdef MAKESHIFT_CXX17
    // TODO: this should be called variant_transform(), and it should not depend on C++17
    //ᅟ
    // Like `std::visit()`, but permits the functor to return different types for different argument types, and returns a variant of the possible results.
    //
template <typename F, typename... VariantsT,
          typename = std::enable_if_t<std::conjunction<is_variant_like<std::decay_t<VariantsT>>...>::value>>
    MAKESHIFT_NODISCARD constexpr typename makeshift::detail::visit_many_result_<F, VariantsT...>::type
    visit_many(F&& func, VariantsT&&... variants)
{
    using VisitManyResult = typename makeshift::detail::visit_many_result_<F, VariantsT...>::type;
    return makeshift::visit(
        [func = std::forward<F>(func)](auto&&... args)
        {
            return VisitManyResult{ func(std::forward<decltype(args)>(args)...) };
        },
        std::forward<VariantsT>(variants)...);
}
#endif // MAKESHIFT_CXX17


} // inline namespace types

} // namespace makeshift


#endif // INCLUDED_MAKESHIFT_VARIANT2_HPP_
