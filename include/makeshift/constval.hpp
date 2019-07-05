
#ifndef INCLUDED_MAKESHIFT_CONSTVAL_HPP_
#define INCLUDED_MAKESHIFT_CONSTVAL_HPP_


#include <utility>     // for tuple_size<>, tuple_element<>, integer_sequence<>
#include <type_traits> // for is_empty<>, conjunction<>

#include <gsl/gsl_assert> // for Expects()

#include <makeshift/type_traits2.hpp> // for can_instantiate<>
#include <makeshift/version.hpp>      // for MAKESHIFT_NODISCARD

#include <makeshift/detail/constval.hpp>


namespace makeshift
{


    // For a reference to the general idea behind constexpr values, cf.
    // https://mpark.github.io/programming/2017/05/26/constexpr-function-parameters/ .


    //ᅟ
    // Determines whether the given type is a constval.
    //
template <typename T> struct is_constval : makeshift::detail::is_constval_<T> { };

    //ᅟ
    // Determines whether the given type is a constval.
    //
template <typename T> constexpr bool is_constval_v = is_constval<T>::value;


    //ᅟ
    // Determines whether the given type `T` is a constval returning the type `R`.
    //
template <typename T, typename R> struct is_constval_of_type : makeshift::detail::is_constval_of_type_<T, R> { };

    //ᅟ
    // Determines whether the given type `T` is a constval returning the type `R`.
    //
template <typename T, typename R> constexpr bool is_constval_of_type_v = is_constval_of_type<T, R>::value;


    //ᅟ
    // A constval type representing the given nullary constexpr function object type. Applies normalization if applicable.
    //ᅟ
template <typename C> using make_constval_t = makeshift::detail::make_constval_t<C>;


    //ᅟ
    // A constval representing the given nullary constexpr function object type. Applies normalization if applicable.
    //ᅟ
template <typename C> constexpr make_constval_t<C> make_constval_v = { };


    //ᅟ
    // Returns a constval representing the given nullary constexpr function object type. Applies normalization if applicable.
    //ᅟ
template <typename C>
    constexpr make_constval_t<C> make_constval(const C&)
{
    static_assert(std::is_empty<C>::value, "argument must be stateless");
    static_assert(can_instantiate_v<makeshift::detail::is_constexpr_functor_r, C>, "argument must be constexpr function object");

    return { };
}


    //ᅟ
    // A constval type that represents the given constexpr object `V`.
    // If `V` is a NTTP wrapper value, the constval type represents the unwrapped value.
    //ᅟ
template <typename T, T V> using constval14_t = typename makeshift::detail::unwrap_constval_<T, V>::type;

    //ᅟ
    // A constval that represents the given constexpr object `V`.
    // If `V` is a NTTP wrapper value, the constval represents the unwrapped value.
    //ᅟ
template <typename T, T V> constval14_t<T, V> constval14{ };


    //ᅟ
    // A constval type that represents the given constexpr object referenced by `Ref`.
    // The language requires template reference arguments to have static linkage (e.g. global static objects, or static member objects of classes).
    //ᅟ
template <typename T, const T& Ref> using ref_constval14_t = makeshift::detail::ref_constval<T, Ref>;

    //ᅟ
    // A constval that represents the given constexpr object referenced by `Ref`.
    // The language requires template reference arguments to have static linkage (e.g. global static objects, or static member objects of classes).
    //ᅟ
template <typename T, const T& Ref> constexpr ref_constval14_t<T, Ref> ref_constval14 = { };

#ifdef MAKESHIFT_CXX17
    //ᅟ
    // A constval type that represents the given constexpr object `V`.
    // If `V` is a NTTP wrapper value, the constval type represents the unwrapped value.
    //ᅟ
template <auto V> using constval_t = typename makeshift::detail::unwrap_constval_<decltype(V), V>::type;

    //ᅟ
    // A constval that represents the given constexpr object `V`.
    // If `V` is a NTTP wrapper value, the constval represents the unwrapped value.
    //ᅟ
template <auto V> constval_t<V> constval{ };

    //ᅟ
    // A constval type that represents the given constexpr object referenced by `Ref`.
    // The language requires template reference arguments to have static linkage (e.g. global static objects, or static member objects of classes).
    //ᅟ
template <const auto& Ref> using ref_constval_t = make_constval_t<makeshift::detail::ref_constval<std::remove_const_t<std::remove_reference_t<decltype(Ref)>>, Ref>>;

    //ᅟ
    // A constval that represents the given constexpr object referenced by `Ref`.
    // The language requires template reference arguments to have static linkage (e.g. global static objects, or static member objects of classes).
    //ᅟ
template <const auto& Ref> constexpr ref_constval_t<Ref> ref_constval = { };
#endif // MAKESHIFT_CXX17


    //ᅟ
    // Returns the value of a constval, or passes through the argument if it is not a constval.
    //
template <typename C>
    constexpr auto constval_extract(const C& value)
{
    return makeshift::detail::constval_extract_impl(is_constval<C>{ }, value);
}


    //ᅟ
    // Returns the result of the function applied to the values of the given constvals as a constval, or the result value itself if one of the arguments is not a constval.
    //ᅟ
    //ᅟ    auto baseIndexR = make_constval([]{ return 42; }); // returns `std::integral_constant<int, 42>{ }`
    //ᅟ    auto offsetR = make_constval([]{ return 3; }); // returns `std::integral_constant<int, 3>{ }`
    //ᅟ    auto indexR = constval_transform(std::plus<>, baseIndexR, offsetR); // returns `std::integral_constant<int, 45>{ }`
    //
template <typename F, typename... Cs>
    MAKESHIFT_NODISCARD constexpr auto
    constval_transform(const F&, const Cs&... args)
{
    static_assert(std::is_empty<F>::value, "transformer must be stateless");
    return makeshift::detail::constval_transform_impl<F>(std::conjunction<is_constval<Cs>...>{ }, args...);
}


    //ᅟ
    // Returns the result of the function applied to the given constvals as a constval, or the result value itself if one of the arguments is not a constval.
    //ᅟ
    //ᅟ    auto variantR = make_constval([]{ return std::variant<int, float>{ 42 }; });
    //ᅟ    auto elementR = constval_extend(
    //ᅟ        [](auto _variantR)
    //ᅟ        {
    //ᅟ            constexpr auto variant = _variantR();
    //ᅟ            return std::get<variant.index()>(variant);
    //ᅟ        },
    //ᅟ        variantR);
    //ᅟ    // equivalent to `make_constval([]{ return 42; })`
    //
template <typename CF, typename... Cs>
    MAKESHIFT_NODISCARD constexpr auto
    constval_extend(const CF&, const Cs&... args)
{
    static_assert(std::is_empty<CF>::value, "extender must be stateless");
    return makeshift::detail::constval_extend_impl<CF>(std::conjunction<is_constval<Cs>...>{ }, args...);
}


    //ᅟ
    // Generates an assertion for the given condition value which is checked at compile time for constvals, or at runtime for non-constvals.
    //ᅟ
    //ᅟ    auto condC = constval_transform([](index idx) { return idx >= 0; }, idxC);
    //ᅟ    constval_assert(condC);
    //
template <typename BoolC,
          std::enable_if_t<std::is_convertible<BoolC, bool>::value, int> = 0>
    constexpr void
    constval_assert(const BoolC& arg)
{
    return makeshift::detail::constval_assert_impl(is_constval<BoolC>{ }, arg);
}

    //ᅟ
    // Generates an assertion for the given condition value which is checked at compile time for constvals, or at runtime for non-constvals.
    //ᅟ
    //ᅟ    constval_assert([](index idx) { return idx >= 0; }, idxC);
    //
template <typename F, typename... Cs,
          std::enable_if_t<!std::is_convertible<F, bool>::value, int> = 0>
    constexpr void
    constval_assert(const F& func, const Cs&... args)
{
    static_assert(std::is_empty<F>::value, "transformer must be stateless");
    makeshift::constval_assert(makeshift::constval_transform(func, args...));
}


    //ᅟ
    // Represents a constval of type `std::array<>` with the given element type and values.
    //
template <typename T, T... Vs>
    struct array_constant : makeshift::detail::constval_tag
{
    using element_type = decltype(makeshift::detail::nttp_unwrap(std::declval<T>()));
    using value_type = std::array<element_type, sizeof...(Vs)>;

    static constexpr value_type value = { makeshift::detail::nttp_unwrap(Vs)... };

    MAKESHIFT_NODISCARD constexpr value_type operator ()(void) const noexcept
    {
        return value;
    }
    MAKESHIFT_NODISCARD constexpr operator value_type(void) const noexcept
    {
        return value;
    }

        //ᅟ
        // Implement tuple-like protocol for `array_constant<>`.
        //
    template <std::size_t I>
        MAKESHIFT_NODISCARD friend constexpr
        make_constval_t<makeshift::detail::array_accessor_functor<array_constant, I>>
        get(array_constant) noexcept
    {
        static_assert(I < sizeof...(Vs), "index out of range");
        return { };
    }
};

    //ᅟ
    // Represents a constval of type `std::array<>` with the given element type and values.
    //
template <typename T, T... Vs> constexpr array_constant<T, Vs...> array_c{ };


    //ᅟ
    // Converts a `std::integer_sequence<>` to a constval of type `std::array<>`.
    // TODO: remove?
    //
template <typename T, T... Vs>
    MAKESHIFT_NODISCARD constexpr array_constant<T,Vs...> make_array_constant(std::integer_sequence<T, Vs...>) noexcept
{
    return { };
}

    //ᅟ
    // Converts a sequence of homogeneously typed constvals to a constval of type `std::array<>`.
    // TODO: remove?
    //
template <typename T, T... Vs>
    MAKESHIFT_NODISCARD constexpr array_constant<T,Vs...> make_array_constant(std::integral_constant<T, Vs>...) noexcept
{
        // This permits conversion from any sequence of constvals because of constval normalization.
    return { };
}


#ifdef MAKESHIFT_CXX17
    //ᅟ
    // Represents a constval of type `std::tuple<>` with the given values.
    //
template <auto... Vs>
    struct tuple_constant : makeshift::detail::constval_tag
{
    using value_type = std::tuple<decltype(makeshift::detail::nttp_unwrap(Vs))...>;

    static constexpr value_type value = { makeshift::detail::nttp_unwrap(Vs)... };

    MAKESHIFT_NODISCARD constexpr value_type operator ()(void) const noexcept
    {
        return value;
    }
    MAKESHIFT_NODISCARD constexpr operator value_type(void) const noexcept
    {
        return value;
    }

        //ᅟ
        // Implement tuple-like protocol for `tuple_constant<>`.
        //
    template <std::size_t I>
        MAKESHIFT_NODISCARD friend constexpr
        make_constval_t<makeshift::detail::tuple_accessor_functor<tuple_constant, I>>
        get(tuple_constant) noexcept
    {
        static_assert(I < sizeof...(Vs), "index out of range");
        return { };
    }
};

    //ᅟ
    // Represents a constval of type `std::tuple<>` with the given values.
    //
template <auto... Vs> constexpr tuple_constant<Vs...> tuple_c{ };
#endif // MAKESHIFT_CXX17


} // namespace makeshift


namespace std
{


    // Implement tuple-like protocol for `array_constant<>`.
template <typename T, T... Vs> struct tuple_size<makeshift::array_constant<T, Vs...>> : public std::integral_constant<std::size_t, sizeof...(Vs)> { };
template <std::size_t I, typename T, T... Vs> struct tuple_element<I, makeshift::array_constant<T, Vs...>> { using type = makeshift::make_constval_t<makeshift::detail::array_accessor_functor<makeshift::array_constant<T, Vs...>, I>>; };

#ifdef MAKESHIFT_CXX17
    // Implement tuple-like protocol for `tuple_constant<>`.
template <auto... Vs> struct tuple_size<makeshift::tuple_constant<Vs...>> : public std::integral_constant<std::size_t, sizeof...(Vs)> { };
template <std::size_t I, auto... Vs> struct tuple_element<I, makeshift::tuple_constant<Vs...>> { using type = makeshift::make_constval_t<makeshift::detail::tuple_accessor_functor<makeshift::tuple_constant<Vs...>, I>>; };
#endif // MAKESHIFT_CXX17


} // namespace std


#endif // INCLUDED_MAKESHIFT_CONSTVAL_HPP_
