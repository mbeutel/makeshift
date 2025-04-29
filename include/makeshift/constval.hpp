﻿
#ifndef INCLUDED_MAKESHIFT_CONSTVAL_HPP_
#define INCLUDED_MAKESHIFT_CONSTVAL_HPP_


#include <array>
#include <utility>      // for tuple_size<>, tuple_element<>, integer_sequence<>
#include <type_traits>  // for is_empty<>, conjunction<>

#include <gsl-lite/gsl-lite.hpp>  // for type_identity<>, index, dim, stride, diff, gsl_CPP20_OR_GREATER

#if !gsl_CPP20_OR_GREATER
# error makeshift requires C++20 mode or higher
#endif // !gsl_CPP20_OR_GREATER

#include <makeshift/type_traits.hpp>  // for can_instantiate<>

#include <makeshift/detail/constval.hpp>


    //
    // Returns a normalized constval with the value of the given constant expression.
    //ᅟ
    //ᅟ    auto answerC = MAKESHIFT_CONSTVAL(42); // returns `std::integral_constant<int, 42>{ }`
    //ᅟ    auto piC = MAKESHIFT_CONSTVAL(3.14159); // returns constval representing 3.14159
    //
#define MAKESHIFT_CONSTVAL(...) MAKESHIFT_CONSTVAL_(__VA_ARGS__)

    // `MAKESHIFT_CONSTVAL()` also works for non-integral types:
    //
    //     auto piC = MAKESHIFT_CONSTVAL(3.14159);
    //     // returns constval representing 3.14159
    //
    //     struct PerformanceParams { int loopUnrollSize; };
    //     auto paramsC = MAKESHIFT_CONSTVAL(PerformanceParams{ .loopUnrollSize = 2 });
    //     // returns constval representing value `PerformanceParams{ 2 }`
    //
    // Additionally, given the code
    //
    //     constexpr X x;
    //     auto xC = MAKESHIFT_CONSTVAL(x);
    //
    // constval normalization leads to to the following guarantees:
    //
    // - `xC` is a subtype of `std::integral_constant<X, x>` if `X` is a valid non-type template parameter type (e.g. an integer
    //   or enumeration type).
    // - `xC` is a subtype of `array_constant<V, Vs...>` if `x` is `std::array<U, sizeof...(us)>{ us... }`, where `V` is `U` if
    //   `U` is a valid non-type template parameter type, and `U const&` otherwise.
    // - `xC` is a subtype of `tuple_constant<Cs...>` if `x` is `std::tuple{ us... }`, where `Cs...` is
    //   `decltype(MAKESHIFT_CONSTVAL(us))...`.
    // - `xC` is tuple-like if `x` is tuple-like; `std::get<I>(xC)` returns `MAKESHIFT_CONSTVAL(std::get<I>(x))`.


namespace makeshift {

namespace gsl = ::gsl_lite;


    // For a reference to the general idea behind constexpr values, cf.
    // https://mpark.github.io/programming/2017/05/26/constexpr-function-parameters/ .


    //
    // Represents an integer constval.
    //
template <int Value>
using int_constant = std::integral_constant<int, Value>;

    //
    // Represents an integer constval.
    //
template <int Value>
constexpr int_constant<Value>
int_c{ };


    //
    // Represents an index constval.
    //
template <gsl::index Value>
using index_constant = std::integral_constant<gsl::index, Value>;

    //
    // Represents an index constval.
    //
template <gsl::index Value>
constexpr index_constant<Value>
index_c{ };


    //
    // Represents a difference constval.
    //
template <gsl::diff Value>
using diff_constant = std::integral_constant<gsl::diff, Value>;

    //
    // Represents a difference constval.
    //
template <gsl::diff Value>
constexpr diff_constant<Value>
diff_c{ };


    //
    // Represents a dimension constval.
    //
template <gsl::dim Value>
using dim_constant = std::integral_constant<gsl::dim, Value>;

    //
    // Represents an dimension constval.
    //
template <gsl::dim Value>
constexpr dim_constant<Value>
dim_c{ };


    //
    // Represents an array stride constval.
    //
template <gsl::stride Value>
using stride_constant = std::integral_constant<gsl::stride, Value>;

    //
    // Represents an array stride constval.
    //
template <gsl::stride Value>
constexpr stride_constant<Value>
stride_c{ };


    //
    // Determines the normalized constval type for a given proto-constval type.
    //
template <typename C>
using constval_t = detail::constval_t<C>;


    //
    // Constval type that represents the object given or referenced.
    //ᅟ
    // `T` may be an object type or a const reference type. This is useful to obtain constvals representing the elements of an
    // `array_constant<>`.
    //
template <typename T, T V>
using constant = typename detail::constant_<T, V>::type;

    //
    // Constval that represents the object given or referenced.
    //ᅟ
    // `T` may be an object type or a const reference type. This is useful to obtain constvals representing the elements of an
    // `array_constant<>`.
    //
template <typename T, T V>
constexpr constant<T, V>
c{ };


    //
    // Constval type that represents the given object.
    //ᅟ
    // The object type must be a valid C++14 non-type template parameter type.
    //
template <auto V,
          typename = std::enable_if_t<detail::is_valid_nttp_<decltype(V)>::value>>
using val_constant = std::integral_constant<decltype(V), V>;

    //
    // Constval that represents the given object.
    //ᅟ
    // The object type must be a valid C++14 non-type template parameter type.
    //
template <auto V>
constexpr val_constant<V>
val_c{ };


    //
    // Constval that represents the referenced object.
    //ᅟ
    // Note that the language requires template reference arguments to have static linkage (e.g. global static objects, or static
    // member objects of classes).
    //
template <auto const& Ref>
using ref_constant = constant<decltype(Ref), Ref>;

    //
    // Constval represents the referenced object.
    //ᅟ
    // Note that the language requires template reference arguments to have static linkage (e.g. global static objects, or static
    // member objects of classes).
    //
template <auto const& Ref>
constexpr ref_constant<Ref>
ref_c{ };


    //
    // Use `type_identity_c<T>` as a value representation of `T` for tag dispatching.
    //
template <typename T>
constexpr gsl::type_identity<T>
type_identity_c{ };


    //
    // Returns the value of a constval, or passes through the argument if it is not a constval.
    //
template <typename C>
constexpr auto
constval_extract(const C& value)
{
    return detail::constval_extract_impl(is_constval<C>{ }, value);
}


    //
    // Returns the result of the function applied to the values of the given constvals as a constval, or the result value itself
    // if one of the arguments is not type-transportable.
    //ᅟ
    //ᅟ    auto baseIndexR = MAKESHIFT_CONSTVAL(42); // returns `std::integral_constant<int, 42>{ }`
    //ᅟ    auto offsetR = MAKESHIFT_CONSTVAL(3); // returns `std::integral_constant<int, 3>{ }`
    //ᅟ    auto indexR = constval_transform(std::plus<>, baseIndexR, offsetR); // returns `std::integral_constant<int, 45>{ }`
    //
template <typename F, typename... Cs>
[[nodiscard]] constexpr auto
constval_transform(const F&, const Cs&... args)
{
    static_assert(is_type_transportable_v<F>, "transformer must be type-transportable");
    return detail::constval_transform_impl<F>(std::conjunction<is_type_transportable<Cs>...>{ }, args...);
}


    //
    // Returns the result of the function applied to the given constvals as a constval, or the result value itself if one of the
    // arguments is not type-transportable.
    //ᅟ
    //ᅟ    auto variantR = MAKESHIFT_CONSTVAL(std::variant<int, float>{ 42 });
    //ᅟ    auto elementR = constval_extend(
    //ᅟ        [](auto _variantR)
    //ᅟ        {
    //ᅟ            constexpr auto variant = _variantR();
    //ᅟ            return std::get<variant.index()>(variant);
    //ᅟ        },
    //ᅟ        variantR);
    //ᅟ    // equivalent to `MAKESHIFT_CONSTVAL(42)`
    //
template <typename CF, typename... Cs>
[[nodiscard]] constexpr auto
constval_extend(const CF&, const Cs&... args)
{
    static_assert(is_type_transportable_v<CF>, "extender must be type-transportable");
    return detail::constval_extend_impl<CF>(std::conjunction<is_type_transportable<Cs>...>{ }, args...);
}


    //
    // Converts the given constval, taken to be a range of elements, to a `std::array<>`. Useful to convert dynamic compile-time
    // computations into `constinit` data.
    // TODO: we do not support proper constvals here yet.
    //ᅟ
    //ᅟ    auto dataC = [] { return std::vector{ 1, 2, 3 }; };
    //ᅟ    constexpr std::array<int, 3> data = constval_range_to_array(dataC);  // returns `std::array{ 1, 2, 3 }`
    //
template <typename C, typename ProjT = gsl::identity>
[[nodiscard]] constexpr auto
constval_range_to_array(C, ProjT = { })
{
    static_assert(is_type_transportable_v<ProjT>, "projector must be type-transportable");
    constexpr std::size_t N = std::size(ProjT{ }(C{ }()));
    return detail::constval_range_to_array_impl<N>(ProjT{ }(C{ }()));
}


    //
    // Represents a constval of type `std::array<>` with the given element type and values.
    //
template <typename T, T... Vs>
struct array_constant : detail::constval_tag
{
    using element_type = std::remove_const_t<std::remove_reference_t<T>>;
    using value_type = std::array<element_type, sizeof...(Vs)>;

    static constexpr value_type value = { Vs... };

    constexpr array_constant(void) noexcept = default;
    constexpr array_constant(typename detail::constant_<T, Vs>::type...) noexcept // workaround for VC++ bug, cf. https://developercommunity.visualstudio.com/content/problem/719235/erroneous-c2971-caused-by-using-variadic-by-ref-no.html
    {
    }

    [[nodiscard]] constexpr value_type
    operator ()(void) const noexcept
    {
        return value;
    }
    [[nodiscard]] constexpr
    operator value_type(void) const noexcept
    {
        return value;
    }
};
template <typename T, T... Vs>
constexpr typename array_constant<T, Vs...>::value_type array_constant<T, Vs...>::value;
template <typename T>
struct array_constant<T> : detail::constval_tag
{
    using element_type = std::remove_const_t<std::remove_reference_t<T>>;
    using value_type = std::array<element_type, 0>;

    static constexpr value_type value = { };

    constexpr array_constant(void) noexcept = default;

    [[nodiscard]] constexpr
    value_type operator ()(void) const noexcept
    {
        return value;
    }
    [[nodiscard]] constexpr
    operator value_type(void) const noexcept
    {
        return value;
    }
};
template <typename T>
constexpr typename array_constant<T>::value_type array_constant<T>::value;
template <typename... Cs>
array_constant(Cs...) -> array_constant<typename detail::array_constant_element_type_<typename detail::equal_types_<typename Cs::value_type...>::common_type>::type, Cs::value...>;

    // Implement tuple-like protocol for `array_constant<>`.
template <std::size_t I, typename T, gsl::type_identity_t<T>... Vs>
[[nodiscard]] constexpr
constval_t<detail::array_accessor_functor<I, array_constant<T, Vs...>>>
get(array_constant<T, Vs...>) noexcept
{
    static_assert(I < sizeof...(Vs), "index out of range");
    return { };
}

    //
    // Represents a constval of type `std::array<>` with the given element type and values.
    //
template <typename T, T... Vs> constexpr array_constant<T, Vs...> array_c{ };

    //
    // Constructs a constval of type `std::array<>` from a sequence of homogeneously typed constvals.
    //
template <typename... Cs>
[[nodiscard]] constexpr array_constant<typename detail::array_constant_element_type_<typename detail::equal_types_<typename Cs::value_type...>::common_type>::type, Cs::value...>
make_array_constant(Cs...) noexcept
{
    return { };
}


    //
    // Represents a constval of type `std::tuple<>` with the given values.
    //
template <typename... Cs>
struct tuple_constant : detail::constval_tag
{
    static_assert(std::conjunction_v<is_constval<Cs>...>, "arguments must be constval types");

    using value_type = std::tuple<typename Cs::value_type...>;

    static constexpr value_type value = { Cs{ }... };

    constexpr tuple_constant(void) noexcept = default;
    template <int N = sizeof...(Cs), std::enable_if_t<N != 0, int> = 0>
    constexpr tuple_constant(Cs...) noexcept
    {
    }

    [[nodiscard]] constexpr value_type
    operator ()(void) const noexcept
    {
        return value;
    }
    [[nodiscard]] constexpr
    operator value_type(void) const noexcept
    {
        return value;
    }
};
template <typename... Cs>
constexpr typename tuple_constant<Cs...>::value_type tuple_constant<Cs...>::value;
template <typename... Cs>
tuple_constant(Cs...) -> tuple_constant<Cs...>;

    // Implement tuple-like protocol for `tuple_constant<>`.
template <std::size_t I, typename... Cs>
[[nodiscard]] constexpr
constval_t<detail::tuple_accessor_functor<I, tuple_constant<Cs...>>>
get(tuple_constant<Cs...>) noexcept
{
    static_assert(I < sizeof...(Cs), "index out of range");
    return { };
}

    //
    // Represents a constval of type `std::tuple<>` with the given values.
    //
template <typename... Cs>
constexpr tuple_constant<Cs...>
tuple_c{ };

    //
    // Constructs a constval of type `std::tuple<>` from a sequence of constvals.
    //
template <typename... Cs>
[[nodiscard]] constexpr tuple_constant<Cs...>
make_tuple_constant(Cs...) noexcept
{
    static_assert(std::conjunction_v<is_constval<Cs>...>, "arguments must be constval types");
    return { };
}


    //
    // Returns the size of an array, range, tuple-like, or container as constval if known at compile time, or as value otherwise.
    //
template <typename ContainerT>
[[nodiscard]] constexpr auto
csize(ContainerT const& c)
{
    return detail::csize_impl(detail::can_instantiate_<detail::is_tuple_like_r, void, ContainerT>{ }, c);
}

    //
    // Returns the size of an array, range, tuple-like, or container as constval if known at compile time, or as value otherwise.
    //
template <typename T, std::size_t N>
[[nodiscard]] constexpr std::integral_constant<std::size_t, N>
csize(T const (&)[N]) noexcept
{
    return { };
}


    //
    // Returns the signed size of an array, range, tuple-like, or container as constval if known at compile time, or as value otherwise.
    //
template <typename ContainerT>
[[nodiscard]] constexpr auto
cssize(ContainerT const& c)
{
    return detail::cssize_impl(detail::can_instantiate_<detail::is_tuple_like_r, void, ContainerT>{ }, c);
}

    //
    // Returns the signed size of an array, range, tuple-like, or container as a constval if known at compile time, or as value otherwise.
    //
template <typename T, std::size_t N>
[[nodiscard]] constexpr std::integral_constant<std::ptrdiff_t, N>
cssize(T const (&)[N]) noexcept
{
    return { };
}


} // namespace makeshift


    // Implement tuple-like protocol for `array_constant<>`.
template <typename T, gsl_lite::type_identity_t<T>... Vs> class std::tuple_size<makeshift::array_constant<T, Vs...>> : public std::integral_constant<std::size_t, sizeof...(Vs)> { };
template <std::size_t I, typename T, gsl_lite::type_identity_t<T>... Vs> class std::tuple_element<I, makeshift::array_constant<T, Vs...>> { public: using type = makeshift::constval_t<makeshift::detail::array_accessor_functor<I, makeshift::array_constant<T, Vs...>>>; };

    // Implement tuple-like protocol for `tuple_constant<>`.
template <typename... Cs> class std::tuple_size<makeshift::tuple_constant<Cs...>> : public std::integral_constant<std::size_t, sizeof...(Cs)> { };
template <std::size_t I, typename... Cs> class std::tuple_element<I, makeshift::tuple_constant<Cs...>> { public: using type = makeshift::constval_t<makeshift::detail::tuple_accessor_functor<I, makeshift::tuple_constant<Cs...>>>; };


#endif // INCLUDED_MAKESHIFT_CONSTVAL_HPP_
