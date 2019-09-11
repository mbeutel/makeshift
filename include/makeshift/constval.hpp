
#ifndef INCLUDED_MAKESHIFT_CONSTVAL_HPP_
#define INCLUDED_MAKESHIFT_CONSTVAL_HPP_


#include <utility>     // for tuple_size<>, tuple_element<>, integer_sequence<>
#include <type_traits> // for is_empty<>, conjunction<>

#include <gsl/gsl-lite.hpp> // for Expects()

#include <makeshift/type_traits.hpp> // for can_instantiate<>
#include <makeshift/macros.hpp>      // for MAKESHIFT_NODISCARD

#include <makeshift/detail/constval.hpp>


namespace makeshift
{


    // For a reference to the general idea behind constexpr values, cf.
    // https://mpark.github.io/programming/2017/05/26/constexpr-function-parameters/ .


    //ᅟ
    // A constval type representing the given nullary constexpr function object type. Applies normalization if applicable.
    //
template <typename C> using make_constval_t = makeshift::detail::make_constval<C>;


    //ᅟ
    // Returns a constval with the value of the given proto-constval.
    //ᅟ
    //ᅟ    struct PerformanceParams { int loopUnrollSize; };
    //ᅟ
    //ᅟ    auto params = make_constval([]
    //ᅟ    {
    //ᅟ        return PerformanceParams{ 2 };
    //ᅟ    });
    //ᅟ    // returns constval representing value `PerformanceParams{ 2 }`
    //
template <typename C>
    constexpr make_constval_t<C> make_constval(C const&)
{
    static_assert(std::is_empty<C>::value, "argument must be stateless");
    static_assert(can_instantiate_v<makeshift::detail::is_constexpr_functor_r, C>, "argument must be constexpr function object");

    return { };
}


    //ᅟ
    // Returns a const reference to the constexpr object with the value of the given proto-constval.
    //ᅟ
    //ᅟ    struct PerformanceParams { int loopUnrollSize; };
    //ᅟ
    //ᅟ    constexpr auto& params = make_constref([]
    //ᅟ    {
    //ᅟ        return PerformanceParams{ 2 };
    //ᅟ    });
    //ᅟ    // returns `PerformanceParams const&` referring to constexpr object with value `PerformanceParams{ 2 }`
    //
template <typename C>
    constexpr typename make_constval_t<C>::value_type const& make_constref(C const&)
{
    static_assert(std::is_empty<C>::value, "argument must be stateless");
    static_assert(can_instantiate_v<makeshift::detail::is_constexpr_functor_r, C>, "argument must be constexpr function object");

    return make_constval_t<C>::value;
}


    //ᅟ
    // Constval type that represents the object given or referenced.
    // `T` may be an object type or a const reference type. This is useful to obtain constvals representing the elements of an `array_constant<>`.
    //
template <typename T, T V> using constant = typename makeshift::detail::constant_<T, V>::type;

    //ᅟ
    // Constval that represents the object given or referenced.
    // `T` may be an object type or a const reference type. This is useful to obtain constvals representing the elements of an `array_constant<>`.
    //
template <typename T, T V> constexpr constant<T, V> c{ };


#if MAKESHIFT_CXX >= 17
    //ᅟ
    // Constval type that represents the given object.
    //
template <auto V> using nttp_constant = constant<decltype(V), V>;

    //ᅟ
    // Constval that represents the given object.
    //
template <auto V> constexpr nttp_constant<V> nttp_c{ };


    //ᅟ
    // Constval that represents the referenced object.
    // Note that the language requires template reference arguments to have static linkage (e.g. global static objects, or static member objects of classes).
    //
template <auto const& Ref> using ref_constant = constant<decltype(Ref), Ref>;

    //ᅟ
    // Constval represents the referenced object.
    // Note that the language requires template reference arguments to have static linkage (e.g. global static objects, or static member objects of classes).
    //
template <auto const& Ref> constexpr ref_constant<Ref> ref_c{ };
#endif // MAKESHIFT_CXX >= 17


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
    return makeshift::detail::constval_transform_impl<F>(conjunction<is_constval<Cs>...>{ }, args...);
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
    return makeshift::detail::constval_extend_impl<CF>(conjunction<is_constval<Cs>...>{ }, args...);
}


    //ᅟ
    // Generates an assertion for the given condition value which is checked at compile time for constvals, or at runtime for non-constvals.
    //ᅟ
    //ᅟ    auto condC = constval_transform([](index idx) { return idx >= 0; }, idxC);
    //ᅟ    constval_assert(condC);
    //
template <typename BoolC>
    constexpr std::enable_if_t<std::is_convertible<BoolC, bool>::value, void>
    constval_assert(const BoolC& arg) // TODO: remove
{
    return makeshift::detail::constval_assert_impl(is_constval<BoolC>{ }, arg);
}

    //ᅟ
    // Generates an assertion for the given condition value which is checked at compile time for constvals, or at runtime for non-constvals.
    //ᅟ
    //ᅟ    constval_assert([](index idx) { return idx >= 0; }, idxC);
    //
template <typename F, typename... Cs>
    constexpr std::enable_if_t<!std::is_convertible<F, bool>::value, void>
    constval_assert(const F& func, const Cs&... args) // TODO: remove
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
    using element_type = std::remove_const_t<std::remove_reference_t<T>>;
    using value_type = std::array<element_type, sizeof...(Vs)>;

    static constexpr value_type value = { Vs... };

    constexpr array_constant(void) noexcept = default;
    constexpr array_constant(typename makeshift::detail::constant_<T, Vs>::type...) noexcept // workaround for VC++ bug, cf. https://developercommunity.visualstudio.com/content/problem/719235/erroneous-c2971-caused-by-using-variadic-by-ref-no.html
    {
    }

    MAKESHIFT_NODISCARD constexpr value_type operator ()(void) const noexcept
    {
        return value;
    }
    MAKESHIFT_NODISCARD constexpr operator value_type(void) const noexcept
    {
        return value;
    }
};
template <typename T>
    struct array_constant<T> : makeshift::detail::constval_tag
{
    using element_type = std::remove_const_t<std::remove_reference_t<T>>;
    using value_type = std::array<element_type, 0>;

    static constexpr value_type value = { };

    constexpr array_constant(void) noexcept = default;

    MAKESHIFT_NODISCARD constexpr value_type operator ()(void) const noexcept
    {
        return value;
    }
    MAKESHIFT_NODISCARD constexpr operator value_type(void) const noexcept
    {
        return value;
    }
};
#if MAKESHIFT_CXX >= 17
template <typename... Cs>
    array_constant(Cs...) -> array_constant<typename makeshift::detail::array_constant_element_type_<typename makeshift::detail::equal_types_<typename Cs::value_type...>::common_type>::type, Cs::value...>;
#endif // MAKESHIFT_CXX >= 17

    // Implement tuple-like protocol for `array_constant<>`.
template <std::size_t I, typename T, T... Vs>
    MAKESHIFT_NODISCARD constexpr
    make_constval_t<makeshift::detail::array_accessor_functor<I, array_constant<T, Vs...>>>
    get(array_constant<T, Vs...>) noexcept
{
    static_assert(I < sizeof...(Vs), "index out of range");
    return { };
}

    //ᅟ
    // Represents a constval of type `std::array<>` with the given element type and values.
    //
template <typename T, T... Vs> constexpr array_constant<T, Vs...> array_c{ };

    //ᅟ
    // Constructs a constval of type `std::array<>` from a sequence of homogeneously typed constvals.
    //
template <typename... Cs>
    MAKESHIFT_NODISCARD constexpr array_constant<typename makeshift::detail::array_constant_element_type_<typename makeshift::detail::equal_types_<typename Cs::value_type...>::common_type>::type, Cs::value...>
    make_array_constant(Cs...) noexcept
{
    return { };
}


    //ᅟ
    // Represents a constval of type `std::tuple<>` with the given values.
    //
template <typename... Cs>
    struct tuple_constant : makeshift::detail::constval_tag
{
    static_assert(conjunction_v<is_constval<Cs>...>, "arguments must be constval types");

    using value_type = std::tuple<typename Cs::value_type...>;

    static constexpr value_type value = { Cs{ }... }; // TODO: probably VC++ workaround needed

    constexpr tuple_constant(void) noexcept = default;
    constexpr tuple_constant(Cs...) noexcept
    {
    }

    MAKESHIFT_NODISCARD constexpr value_type operator ()(void) const noexcept
    {
        return value;
    }
    MAKESHIFT_NODISCARD constexpr operator value_type(void) const noexcept
    {
        return value;
    }
};
template <>
    struct tuple_constant<> : makeshift::detail::constval_tag
{
    using value_type = std::tuple<>;

    static constexpr value_type value = { }; // TODO: probably VC++ workaround needed

    constexpr tuple_constant(void) noexcept = default;

    MAKESHIFT_NODISCARD constexpr value_type operator ()(void) const noexcept
    {
        return value;
    }
    MAKESHIFT_NODISCARD constexpr operator value_type(void) const noexcept
    {
        return value;
    }
};
#if MAKESHIFT_CXX >= 17
template <typename... Cs>
    tuple_constant(Cs...) -> tuple_constant<Cs...>;
#endif // MAKESHIFT_CXX >= 17

    //ᅟ
    // Implement tuple-like protocol for `tuple_constant<>`.
    //
template <std::size_t I, typename... Cs>
    MAKESHIFT_NODISCARD constexpr
    make_constval_t<makeshift::detail::tuple_accessor_functor<I, tuple_constant<Cs...>>>
    get(tuple_constant<Cs...>) noexcept
{
    static_assert(I < sizeof...(Cs), "index out of range");
    return { };
}

    //ᅟ
    // Represents a constval of type `std::tuple<>` with the given values.
    //
template <typename... Cs> constexpr tuple_constant<Cs...> tuple_c{ };

    //ᅟ
    // Constructs a constval of type `std::tuple<>` from a sequence of constvals.
    //
template <typename... Cs>
    MAKESHIFT_NODISCARD constexpr tuple_constant<Cs...>
    make_tuple_constant(Cs...) noexcept
{
    static_assert(conjunction_v<is_constval<Cs>...>, "arguments must be constval types");
    return { };
}


} // namespace makeshift


namespace std
{


    // Implement tuple-like protocol for `array_constant<>`.
template <typename T, T... Vs> struct tuple_size<makeshift::array_constant<T, Vs...>> : public std::integral_constant<std::size_t, sizeof...(Vs)> { };
template <std::size_t I, typename T, T... Vs> struct tuple_element<I, makeshift::array_constant<T, Vs...>> { using type = makeshift::make_constval_t<makeshift::detail::array_accessor_functor<I, makeshift::array_constant<T, Vs...>>>; };

    // Implement tuple-like protocol for `tuple_constant<>`.
template <typename... Cs> struct tuple_size<makeshift::tuple_constant<Cs...>> : public std::integral_constant<std::size_t, sizeof...(Cs)> { };
template <std::size_t I, typename... Cs> struct tuple_element<I, makeshift::tuple_constant<Cs...>> { using type = makeshift::make_constval_t<makeshift::detail::tuple_accessor_functor<I, makeshift::tuple_constant<Cs...>>>; };


} // namespace std


#endif // INCLUDED_MAKESHIFT_CONSTVAL_HPP_
